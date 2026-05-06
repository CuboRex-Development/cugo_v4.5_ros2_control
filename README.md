<!-- TODO:図の修正 -->
![image](https://github.com/user-attachments/assets/be603edd-43dd-42b7-8215-2a89df03e3c2)

# cugo_v4.5_ros2_control

CuGo V4.5 用 ROS 2 コントロールノードです。

`/cmd_vel` を Subscribe してロボットに速度指令を送信し、`/odom` と `/tf` を Publish します。
マイコン側のスケッチ [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) と組み合わせて使用します。

ロボットの状態監視・操作のため、 カスタムメッセージ [cugo_v4.5_ros2_msgs](https://github.com/CuboRex-Development/cugo_v4.5_ros2_msgs) を使用します。

ROS 2 Jazzy Jalisco 以降で動作します。

### 対応製品

<!-- TODO: V4.5のリンクを貼る -->
* [CuGo V4.5](null)

> [!WARNING]
> クローラロボット開発プラットフォーム CuGo V4 / クローラロボット開発プラットフォーム V3i をご利用の方は [cugo_ros2_control2](https://github.com/CuboRex-Development/cugo_ros2_control2) を参照してください。



# Table of Contents
- [Features](#features)
- [Connection](#connection)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Parameters](#parameters)
- [Topics](#topics)
- [TF](#tf)
- [Note](#note)
- [License](#license)

# Features

`/cmd_vel` で受け取った速度指令を [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) に送信し、マイコンから受け取った現在速度をもとにオドメトリを計算して `/odom` として Publish します。

速度制御に加え、ヘッドライト・タワーライト・バンパー・ブレーキなどの IO デバイス制御と、コントローラステータス・エンコーダ・モータドライバ温度などのセンサ情報モニタリングに対応しています。IO 制御はトピック経由で行い、センサデータは各トピックへ Publish されます。

<img width="4077" height="2541" alt="cugo_v4 5_ros2_control" src="https://github.com/user-attachments/assets/f1fafdfb-5448-4293-af8a-70d4c094e65e" />


本ノードを実行しているROS2 PC とCuGo V4.5 との通信方式は USB-Serial 接続 をデフォルトとしていますが、その他の通信方式も選択することができます。

# Connection

### USB-Serial 接続 (デフォルト)

PC と Raspberry Pi Pico 2 W を USB ケーブルで直接接続する方式です。  
追加機器なしに最も簡単に使用できます。

<img width="4194" height="769" alt="USB" src="https://github.com/user-attachments/assets/1d0d16b3-4b9c-4172-8e61-97643d443131" />


### BOXコネクタ-Serial接続

CuGo V4.5 本体の BOX コネクタを介して PC とシリアル通信する方式です。  
USB-UART 変換アダプタなどを使用して PC と接続するほか、UARTポートを持つ機器とUSBなしに接続が可能です。接続ケーブルは、ご自身でご用意ください。
ROS 側の設定は USB-Serial 接続と同じ `serial` モードのまま使用できます。必要に応じて、シリアルポートのパス(`serial_port`)のみ変更してください。

<img width="4193" height="769" alt="UART_BOX" src="https://github.com/user-attachments/assets/9764360b-66bd-4714-a145-e1018a2f05e5" />


### Bluetoothモード

Classic Bluetooth の SPP(Serial Port Profile)を介して PC とロボット間を無線接続する方式です。
WiFi ルータが不要で、ペアリング済みのデバイスと直接接続できます。
接続には事前にペアリングが必要です。詳細は Usage セクションの「Bluetoothモード」を参照してください。  
なお、Bluetoothモードでは、接続断時の自動再接続が適用されません。

<img width="4194" height="769" alt="BT_SPP" src="https://github.com/user-attachments/assets/2e9734c6-8ad2-427f-8452-cbab96952714" />


### WiFi APモード

Raspberry Pi Pico 2 W 自身がアクセスポイントとして動作し、 TCP 接続する方式です。  
WiFi ルータを用意せずに無線でロボットを操作できます。
接続には事前にロボット側の WiFi 設定が必要です。([cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照)

<img width="4194" height="976" alt="WIFI_AP" src="https://github.com/user-attachments/assets/4bbf264b-b8f3-4d30-89ce-3272c1721563" />


### WiFi Stationモード

外部 WiFi ルータ経由で PC とロボット間を TCP 接続する方式です。  
通信を行うために、WiFiルータが必要です。WiFiルータは、ご自身でご用意ください。
接続には事前にロボット側の WiFi 設定が必要です。([cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照)

<img width="4193" height="1558" alt="WiFI_STA" src="https://github.com/user-attachments/assets/c3c286bf-0dbe-4675-84a8-cbf344140c9a" />


# Requirements

### OS / ROS ディストリビューション
  - Ubuntu 24.04 LTS / ROS 2 Jazzy Jalisco

### 依存パッケージ

  - [cugo_v4.5_ros2_msgs](https://github.com/CuboRex-Development/cugo_v4.5_ros2_msgs) — 本パッケージが使用するカスタムメッセージ定義パッケージ(**必須**)
  - robot_state_publisher
  - joint_state_publisher_gui(URDF確認・デバッグ用)
  - socat(WiFiモード〔APモード・Stationモード〕を使用する場合のみ)
  - rfcomm / bluez-utils(Bluetoothモードを使用する場合のみ)

# Installation

以下のコマンド中の `your_ros2_ws` は、ご自身のワークスペース名に置き換えてください。

1. ROS 2 環境を用意します。

   本ノードを実行するPCに ROS 2 環境が導入されていない場合は、 [ROS 2 Documentation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html) を参照し、ROS 2 のインストールを実施してください。


2. cugo_v4.5_ros2_control ノードをクローンします。

   ```bash
   cd ~/your_ros2_ws/src
   git clone https://github.com/CuboRex-Development/cugo_v4.5_ros2_control.git
   ```

3. カスタムメッセージパッケージ [cugo_v4.5_ros2_msgs](https://github.com/CuboRex-Development/cugo_v4.5_ros2_msgs) をクローンします。

   本パッケージは `cugo_v4.5_ros2_control` が使用するカスタムメッセージ型を提供します。同じワークスペース内に配置する必要があります。

   ```bash
   cd ~/your_ros2_ws/src
   git clone https://github.com/CuboRex-Development/cugo_v4.5_ros2_msgs.git
   ```

4. rosdepで依存パッケージをインストールします。

   ```bash
   rosdep install -i --from-paths ~/your_ros2_ws/src/cugo_v4.5_ros2_control ~/your_ros2_ws/src/cugo_v4.5_ros2_msgs
   ```

5. ノードをビルドします。

   ```bash
   cd ~/your_ros2_ws
   colcon build --symlink-install
   source ~/your_ros2_ws/install/local_setup.bash
   ```



# Usage

> [!IMPORTANT]
> いずれの通信モードでも、事前にマイコン(Raspberry Pi Pico 2 W)に [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) のスケッチを書き込んでおく必要があります。事前に、cugo_v4.5_ros2_motorcontrollerのInstallationの手順を実施してください。

### USB-Serial 接続 (デフォルト)

1. CuGo V4.5 の Raspberry Pi Pico 2 W と PC を USB ケーブルで接続します。

2. シリアルポートへのアクセス権を付与します(環境に合わせてポート名やアクセス権限を変更してください)。

   ```bash
   sudo chmod 777 /dev/ttyACM0
   ```

3. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```


### BOXコネクタ-Serial接続

1. CuGo V4.5 本体の BOX コネクタと PC を USB-シリアル変換アダプタなどで接続します。
   
   <img width="2248" height="723" alt="ボックスコネクタ" src="https://github.com/user-attachments/assets/8c8e809c-5500-44a4-ad43-a3b5945c6a83" />

   **J28 ピンアサイン**
   
   | ピン番号 | Raspberry Pi Pico 2 W GP |            機能            |
   | :------: | :----------------------: | :------------------------: |
   |    7     |      GP8 (UART1 TX)      | TxD (ROS側機器のRxDを接続) |
   |    8     |      GP9 (UART1 RX)      | RxD (ROS側機器のTxDを接続) |
   |    20    |           GND            |            GND             |
   
  > [!TIP]
  > 適合コネクタ例(参考)  
  > ハウジング：日本航空電子工業社製 PS-D4C20  
  > コンタクト：日本航空電子工業社製 030-51307-001 など

2. デバイスが認識されているか確認します。(環境に合わせてポート名やアクセス権限を変更してください)。

   ```bash
   ls /dev/ttyUSB*
   ```

3. シリアルポートへのアクセス権を付与します(環境に合わせてポート名やアクセス権限を変更してください)。

   ```bash
   sudo chmod 777 /dev/ttyUSB0
   ```

4. `config/params.yaml` の `serial_port` を接続したポートに変更します(`comm_type: serial` のままで問題ありません)。

   ```yaml
   cugo_v4_5_ros2_control:
     ros__parameters:
       comm_type: serial
       serial_port: /dev/ttyUSB0   # ← BOXコネクタ接続時のポート名
   ```

5. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```

### Bluetoothモード

ロボット側の Bluetooth SPP 設定は [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照してください。

> [!IMPORTANT]
> 以下の手順 1・2 は初回のみ実施してください。

1. PC の Bluetooth でロボットとペアリングします。

   GUI(Bluetooth 設定)または `bluetoothctl` コマンドでペアリングしてください。

   `bluetoothctl` を使用する場合は以下を実行してください。

   ```bash
   bluetoothctl
   ```

   `bluetoothctl` 起動後、以下のコマンドを順に実行してください。

   ```text
   power on
   scan on
   # ロボットのデバイス名または MAC アドレスを確認したら scan off
   scan off
   pair XX:XX:XX:XX:XX:XX
   trust XX:XX:XX:XX:XX:XX
   quit
   ```

   ペアリング済みデバイスの MAC アドレスは以下で確認できます。

   ```bash
   bluetoothctl paired-devices
   ```
> [!NOTE]
> ペアリング完了後は、デバイスとの接続を実施してもすぐに切断されますが、問題ありません。
<!--  -->
> [!NOTE]
> ペアリング後にロボット側のプログラムを書き換えた際は、一度ペアリングを解除して再度ペアリングしてください。

2. `rfcomm` コマンドがパスワードなしで実行できるよう sudoers を設定します。

   ```bash
   sudo gnome-text-editor /etc/sudoers.d/rfcomm-rule
   ```

   以下を記述して保存してください(`your_username` はご自身のユーザー名に置き換えてください)。

   ```text
   your_username ALL=(ALL) NOPASSWD: /usr/bin/rfcomm bind /dev/rfcomm0 *
   ```

3. `config/params.yaml` を編集して `comm_type`、`bt_address`、`bt_channel` を設定します。

   ```yaml
   cugo_v4_5_ros2_control:
     ros__parameters:
       comm_type: bluetooth
       bt_address: "XX:XX:XX:XX:XX:XX"   # ← ロボットの MAC アドレス
       bt_channel: 1
   ```

4. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```

### WiFi APモード

ロボット側の WiFi AP 設定は [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照してください。

1. PC の WiFi を、ロボット側で設定したアクセスポイント(デフォルト SSID: `CuGo_AP`)に接続します。

2. `config/params.yaml` を編集して `comm_type`、`tcp_host`、`tcp_port` を設定します。

   ```yaml
   cugo_v4_5_ros2_control:
     ros__parameters:
       comm_type: wifi
       tcp_host: 192.168.42.1   # ← APモード時の固定 IP アドレス、デフォルト値
       tcp_port: 8080
   ```

3. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```

### WiFi Stationモード

ロボット側の WiFi Station 設定は [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照してください。

1. `config/params.yaml` を編集して `comm_type`、`tcp_host`、`tcp_port` を設定します。

   ```yaml
   cugo_v4_5_ros2_control:
     ros__parameters:
       comm_type: wifi
       tcp_host: 192.168.1.100   # ← ルータが割り当てたロボットの IP アドレス
       tcp_port: 8080
   ```

2. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```

### URDF の確認

`use_urdf: true`(デフォルト)でノードを起動すると、`robot_state_publisher` が起動し `/robot_description` トピックが配信されます。RViz2 でロボットモデルを確認できます。

```bash
ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
```

RViz2 を起動し、以下の設定を行ってください。

1. Fixed Frame を `base_footprint` に設定
2. Add → `RobotModel` を追加し、`Description Topic` を `/robot_description` に設定

URDF のロードが不要な場合は `use_urdf` を無効化できます。詳細は [Parameters > URDF設定](#urdf設定) を参照してください。

# Parameters

ノードのパラメータは `config/params.yaml` で管理します。
通常の使用では、このファイルの「通信設定」と「制御設定」のみ確認・変更してください。

パラメータを変更した場合は `colcon build` を再実行してください。

`params.yaml` で設定できる主なパラメータは以下の通りです。


### URDF設定

|  パラメータ  | デフォルト値 | 説明                                                                                                                    |
| :----------: | :----------: | ----------------------------------------------------------------------------------------------------------------------- |
| `use_urdf`   |    `true`    | `true`: URDF をロードして `robot_state_publisher` を起動する。`false`: スキップする(処理負荷を下げたい場合に使用) |


### 通信設定

|    パラメータ     |  デフォルト値   | 説明                                                                                                              |
| :---------------: | :-------------: | ----------------------------------------------------------------------------------------------------------------- |
|    `comm_type`    |    `serial`     | 通信方式: `serial`(USB)/ `wifi`(APモード・Stationモード共通)/ `bluetooth`(Bluetooth SPP)                          |
|   `serial_port`   | `/dev/ttyACM0`  | シリアルポートのパス(USB接続時)                                                                                   |
| `serial_baudrate` |    `115200`     | ボーレート                                                                                                        |
|    `tcp_host`     | `192.168.1.100` | WiFiモード時の接続先IPアドレス(APモードは `192.168.42.1`がデフォルト、`params.yaml` では `192.168.42.1` に設定済) |
|    `tcp_port`     |     `8080`      | WiFiモード時の接続先ポート番号                                                                                    |
|   `bt_address`    |      `""`       | Bluetoothモード時の接続先MACアドレス                                                                              |
|   `bt_channel`    |       `1`       | Bluetoothモード時の SPP チャンネル番号(通常は `1`)                                                                |

### 制御設定

|        パラメータ        | デフォルト値 | 説明                                                                                                                                                                                                                                                                                             |
| :----------------------: | :----------: | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
|   `control_frequency`    |    `10.0`    | 制御周期 [Hz] (通常 10Hz、最大 20Hz 程度(無線接続時)、最大 50Hz 程度(有線接続時))                                                                                                                                                                                                                |
|    `cmd_vel_timeout`     |    `0.5`     | 他ノードからの`/cmd_vel` 受信タイムアウト [秒] (超過で速度ゼロ)                                                                                                                                                                                                                                  |
|     `serial_timeout`     |    `0.5`     | マイコンからの受信タイムアウト [秒] (超過で再接続)。WiFi接続(APモード・Stationモード)では通信レイテンシが有線より高いため、`1.0`〜`2.0` を推奨                                                                                                                                                   |
| `max_consecutive_errors` |     `5`      | 連続デコードエラーの許容回数。この回数を超えると受信バッファをフラッシュしてフレーミングを再同期します。通信品質が低い環境では大きくし、即座に再同期したい場合は小さくしてください                                                                                                               |
| `response_lost_timeout`  |    `0.0`     | 応答ロスト判定時間 [秒](`0.0` で無効)。ロボットへのリクエスト送信後、この時間内に応答がなければ再度リクエストを送信します。**値により制御周期が大きく低下する可能性があります。** 通信遅延への対策としてのみ使用してください。詳細は [Note > 通信遅延対策について](#通信遅延対策について) を参照 |

> [!CAUTION]
> `response_lost_timeout` は通信遅延の対策としてのみ使用してください。0.0以外の値を設定すると制御周期が低下する可能性があります。設定前に必ず [Note > 通信遅延対策について](#通信遅延対策について) を熟読し、適切な値を設定してください。
<!--  -->

> [!TIP]
> `control_frequency` を大きい値にする場合、`serial_baudrate`を上げる必要があります。デフォルトの `serial_baudrate` = `115200` では、30Hz程度が上限となります。



### オドメトリ積分設定

速度の積分によるオドメトリ計算に使用する手法を選択できます。

|               パラメータ                |       デフォルト値       | 説明                                                                                                                  |
| :-------------------------------------: | :----------------------: | --------------------------------------------------------------------------------------------------------------------- |
|            `odometry_method`            |        `midpoint`        | 積分手法: `midpoint`(修正オイラー法)/ `analytic`(解析解)                                                              |
| `odometry_analytic_angular_z_threshold` | `0.003142`(π/1000 rad/s) | `analytic` 選択時に「旋回なし」とみなすしきい値 [rad/s]。この値未満の場合は `midpoint` にフォールバック(ゼロ除算回避) |


### ROS フレーム・トピック設定

|       パラメータ       | デフォルト値 | 説明                                                                          |
| :--------------------: | :----------: | ----------------------------------------------------------------------------- |
|    `odom_frame_id`     |    `odom`    | オドメトリフレーム名                                                          |
|  `base_link_frame_id`  | `base_link`  | ベースリンクフレーム名(params.yaml では `base_footprint`に設定されています。) |
| `subscribe_topic_name` |  `/cmd_vel`  | 速度指令トピック名                                                            |
|  `publish_topic_name`  |   `/odom`    | オドメトリトピック名                                                          |

### デバイスID

|  パラメータ  | デフォルト値 | 説明                                             |
| :----------: | :----------: | ------------------------------------------------ |
| `product_id` |   `10000`    | CuGo V4.5 のプロダクトID(変更不要)               |
|  `robot_id`  |     `0`      | ロボット識別子(複数台運用時に設定することを想定) |

### 共分散(SLAM・ナビゲーション調整用)

|      パラメータ       | デフォルト値 | 説明                                        |
| :-------------------: | :----------: | ------------------------------------------- |
|     `pose_cov_x`      |    `0.04`    | 位置 X 精度(0.2 m 相当)                     |
|     `pose_cov_y`      |    `0.04`    | 位置 Y 精度(0.2 m 相当)                     |
|     `pose_cov_z`      |   `1.0e9`    | 位置 Z(2次元平面のみのため大きな値に設定)   |
|    `pose_cov_roll`    |   `1.0e9`    | Roll(2次元平面のみのため大きな値に設定)     |
|   `pose_cov_pitch`    |   `1.0e9`    | Pitch(2次元平面のみのため大きな値に設定)    |
|    `pose_cov_yaw`     |    `0.01`    | Yaw 精度(0.1 rad 相当)                      |
| `twist_cov_linear_x`  |   `0.0025`   | 線速度 X 精度(0.05 m/s 相当)                |
| `twist_cov_linear_y`  |   `0.0025`   | 線速度 Y 精度(0.05 m/s 相当)                |
| `twist_cov_angular_z` |   `1.0e9`    | 角速度 Z(2次元平面のみのため大きな値に設定) |

### ログ設定

すべてのログフラグは `false` がデフォルトです。`true` に設定すると、ノード起動時から ログが出力されます。(`cugo_v4_5_launch.py` で読み取る `params.yaml` では一部のログが有効化されています。)

|        パラメータ        | デフォルト値 | 説明                                      |
| :----------------------: | :----------: | ----------------------------------------- |
|     `param_info_log`     |   `false`    | 起動時の設定パラメータ一覧                |
|   `odom_pos_info_log`    |   `false`    | Odometry 位置成分(x, y, yaw)              |
|   `odom_vel_info_log`    |   `false`    | Odometry 速度成分(Vx, Vy, Omega)          |
| `recv_interval_info_log` |   `false`    | データ受信間隔(ms)                        |
| `packet_error_info_log`  |   `false`    | パケットデコードエラー統計(累積件数)      |
|  `connection_info_log`   |   `false`    | 接続状態変化ログ(接続・再接続)            |
|  `connection_lost_log`   |   `false`    | シリアル通信未達 WARN(再接続待機中に出力) |
|   `received_speed_log`   |   `false`    | マイコンから受信した速度(Vx, Vy, Omega)   |
|      `cmd_vel_log`       |   `false`    | `/cmd_vel` で受信した速度指令値           |
|       `tx_cmd_log`       |   `false`    | マイコンへ実際に送信した速度指令値        |
|   `loop_interval_log`    |   `false`    | 制御ループの実行間隔(ms)                  |
|     `handshake_log`      |   `false`    | ハンドシェイク状態遷移の詳細              |
|       `serial_log`       |   `false`    | 送受信パケット内容(COBSデコード後)        |
|     `serial_raw_log`     |   `false`    | 送受信パケット内容(生データ)              |
|      `callback_log`      |   `false`    | コールバック・制御ループの実行フロー      |
|        `rtt_log`         |   `false`    | リクエスト〜レスポンス往復時間(ms)        |

---

### launch 引数

以下のパラメータは `params.yaml` の編集に加えて、launch 引数でも上書きできます。

|     引数     |  デフォルト値   | 説明                                                                                                              |
| :----------: | :-------------: | ----------------------------------------------------------------------------------------------------------------- |
| `comm_type`  |    `serial`     | 通信方式: `serial` / `wifi` / `bluetooth`                                                                         |
|  `tcp_host`  | `192.168.1.100` | WiFiモード時の接続先IPアドレス(APモードは `192.168.42.1`がデフォルト、`params.yaml` では `192.168.42.1` に設定済) |
|  `tcp_port`  |     `8080`      | WiFiモード時の接続先ポート番号                                                                                    |
| `bt_address` |      `""`       | Bluetoothモード時の接続先MACアドレス                                                                              |
| `bt_channel` |       `1`       | Bluetoothモード時の SPP チャンネル番号                                                                            |
| `log_level`  |     `info`      | ログレベル: `debug`, `info`, `warn`, `error`, `fatal`                                                             |
| `use_urdf`   |     `true`      | `true`: URDF をロードして `robot_state_publisher` を起動する。`false`: スキップする                               |



# Topics

### Published Topics

- `/odom` ([nav_msgs/msg/Odometry](https://docs.ros2.org/latest/api/nav_msgs/msg/Odometry.html))  
  ロボットの位置・姿勢・速度情報(オドメトリ)を配信します。マイコンから受け取った車輪速度をもとに計算されます。

- `/tf` ([tf2_msgs/msg/TFMessage](https://docs.ros2.org/latest/api/tf2_msgs/msg/TFMessage.html))  
  `odom` フレームから `base_footprint` フレームへの座標変換を配信します。

- `/handshake_status` ([std_msgs/msg/Bool](https://docs.ros2.org/latest/api/std_msgs/msg/Bool.html))  
  マイコンとのハンドシェイク(接続確立)状態を配信します。接続中は `true`、未接続または切断時は `false` になります。

- `/controller_status` (cugo_v4_5_ros2_msgs/msg/ControllerStatus)  
  CRST01A のコントローラステータスビットを配信します。

- `/controller_error` (cugo_v4_5_ros2_msgs/msg/ControllerError)  
  CRST01A のコントローラエラービットを配信します。

- `/motordriver_error` (cugo_v4_5_ros2_msgs/msg/MotorDriverError)  
  CRST01A のモータドライバエラービットを配信します。

- `/driver_voltage` ([std_msgs/msg/Float32](https://docs.ros2.org/latest/api/std_msgs/msg/Float32.html))  
  ドライバ電源電圧 [V] を配信します。

- `/headlight_status` (cugo_v4_5_ros2_msgs/msg/HeadlightStatus)  
  ヘッドライトの現在状態を配信します。

- `/towerlight_status` (cugo_v4_5_ros2_msgs/msg/TowerlightStatus)  
  タワーライトの現在状態を配信します。

- `/io_input_status` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  外部 4bit デジタル入力の現在状態を配信します。

- `/encoder/motor0` 〜 `/encoder/motor3` ([std_msgs/msg/UInt32](https://docs.ros2.org/latest/api/std_msgs/msg/UInt32.html))  
  各モータのエンコーダカウント値を配信します。

- `/motordriver_temp/motor0` 〜 `/motordriver_temp/motor3` ([std_msgs/msg/UInt16](https://docs.ros2.org/latest/api/std_msgs/msg/UInt16.html))  
  各モータドライバの温度 [℃] を配信します。

- `/motordriver_error_code/motor0` 〜 `/motordriver_error_code/motor3` ([std_msgs/msg/UInt16](https://docs.ros2.org/latest/api/std_msgs/msg/UInt16.html))  
  各モータドライバのエラーコードを配信します。

- `/config/bumper` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  CRST01A フラッシュに保存されたバンパー設定値を配信します。

- `/config/brake` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  CRST01A フラッシュに保存されたブレーキ設定値を配信します。

### Subscribed Topics

- `/cmd_vel` ([geometry_msgs/msg/Twist](https://docs.ros2.org/latest/api/geometry_msgs/msg/Twist.html))  
  ロボットへの速度指令を受信します。`linear.x`(前後)と `angular.z`(旋回)を使用します。

- `/cmd_mode` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  走行モードの切替を受信します。`0x80` = RC モード、`0x81` = CMD モード。

- `/cmd_emergency_decel` ([std_msgs/msg/Empty](https://docs.ros2.org/latest/api/std_msgs/msg/Empty.html))  
  緊急減速トリガを受信します。メッセージを受信した時点で緊急減速が発動します。

- `/cmd_reset_controller_error` (cugo_v4_5_ros2_msgs/msg/ControllerError)  
  解除するコントローラエラービットを受信します。

- `/cmd_reset_motordriver_error` (cugo_v4_5_ros2_msgs/msg/MotorDriverError)  
  解除するモータドライバエラービットを受信します。

- `/cmd_headlight` (cugo_v4_5_ros2_msgs/msg/HeadlightStatus)  
  ヘッドライトの制御指令を受信します。

- `/cmd_towerlight` (cugo_v4_5_ros2_msgs/msg/TowerlightStatus)  
  タワーライトの制御指令を受信します。

- `/cmd_bumper_config` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  バンパー設定値を受信し、CRST01A へ書き込みます。

- `/cmd_brake_config` ([std_msgs/msg/UInt8](https://docs.ros2.org/latest/api/std_msgs/msg/UInt8.html))  
  ブレーキ設定値を受信し、CRST01A へ書き込みます。

# TF

`use_urdf: true` で起動すると、`robot_state_publisher` が `urdf/cugo_v4_5_urdf.urdf` を読み込み、`/robot_description` トピックと TF を配信します。

```
cugo_v4.5_ros2_control
└── urdf
    └── cugo_v4_5_urdf.urdf   ← CuGo V4.5 本体の URDF(メッシュ付き)
└── meshes
    ├── base_footprint.dae    ← 表示用メッシュ
    └── base_footprint.stl    ← コリジョン用メッシュ
```

センサなど追加の部品を取り付ける場合は、`urdf/cugo_v4_5_urdf.urdf` に `<link>` / `<joint>` を直接追記し、`colcon build` を再実行してください。

# Note

通信プロトコルの詳細仕様は [PROTOCOL.md](PROTOCOL.md) を参照してください。

ご不明点は [お問い合わせフォーム](https://cuborex.com/contact/) よりお問い合わせください。


## 通信遅延対策について
<details> <summary> response_lost_timeout の設定方法 </summary> 
<div>

### 概要

`response_lost_timeout` は、ロボットへ送信したリクエストへの応答が得られない場合のレスポンス待機時間です。
通常運用では `0`(無効)のままお使いください。WiFi 接続などで通信遅延が問題になる場合にのみ設定してください。

### 仕組み

本プログラムは、`control_frequency`で設定された周期でロボットへのリクエスト信号(速度指令を含む)を送り続けます。
ロボットはリクエストを受け取ると、現在の速度情報を返答します。
本プログラムが送信するリクエストは、ロボットからのレスポンスの有無に関わらず常に定期実行されます。

このため、リクエストに対するレスポンスが`control_frequency`の周期より遅れてしまった場合、受信側の機器はリクエストを重複して受け取ることとなり、正常な動作を望めなくなります。
有線接続で使用する際は遅延のリスクが少なく本動作で問題はないですが、無線接続で使用する際は本現象が発生するリスクが大きくなります。
この対策として、リクエストの重複送信を防止するパラメータが`response_lost_timeout`です。
`response_lost_timeout`が0では無い場合、一度リクエストを送信したら、`response_lost_timeout`秒経つまでは再度リクエストを送信しません。

実際の処理の流れ：

1. コントロールループで目標速度コマンドを送信(リクエスト)
2. 有効なパケットを 1 件受信(レスポンス)したら、次のリクエスト送信を許可
3. `response_lost_timeout` 秒を超えても応答がない場合、「レスポンスロスト」と判定し次のリクエスト送信を許可

### 実効制御周波数への影響

`response_lost_timeout` を設定すると、応答が得られない期間は リクエストの送信 が抑制されます。最悪ケースでの実効的な制御周波数は以下のようになります。

```
実効制御周波数 ≈ 1 / response_lost_timeout
```

例: `response_lost_timeout = 0.5` の場合、制御周波数は最悪 2 Hz まで低下する可能性があります。

そのため、本機能は低周期での制御で問題がない用途にのみ使用してください。

### 設定値の下限

`response_lost_timeout` は `1 / control_frequency`(制御周期)より大きい値を設定してください。
それ以下の値では、制御ループのたびにタイムアウトが発生し、機能が実質無効と同じになります。

> 例: `control_frequency = 50.0`(20 ms 周期)の場合、`response_lost_timeout > 0.02` を推奨

### 応答遅延パケットの処理

「レスポンスロスト」判定後に次のリクエストを送信した直後、前のリクエストへの遅延応答が届く場合があります。
この場合、遅延応答は有効なパケットとして処理されますが、前回受信からの経過時間(`dt`)が長くなるため、オドメトリの誤差が一時的に増大する可能性があります。

### 推奨値

|                接続方式                |    推奨値     |
| :------------------------------------: | :-----------: |
| USB-Serial接続, BOXコネクタ-Serial接続 |     `0.0`     |
|     WiFi(APモード・Stationモード)      | `0.2`〜`0.5`  |
|            Bluetooth モード            | `0.2`〜 `0.5` |

### 制約事項

`response_lost_timeout` は `serial_timeout` 未満の値を設定してください。
`serial_timeout` 以上の値を設定した場合、起動時に自動的に `0`(無効)に補正され、WARNログが出力されます。

</div>
</details>

## WiFi 通信の安定化について

<details> <summary> 無線通信が安定しない際の対策例 </summary> 
<div>

### WiFi パワーセーブモードの無効化

Linux の WiFi アダプタがパワーセーブモードで動作している場合、受信データが遅延することがあります。
WiFi 接続時に応答が周期的にロストする(`[response lost]` ログが頻繁に出る)場合は、パワーセーブを無効化することで改善する場合があります。

#### 一時的な無効化(再起動で元に戻る)

```bash
sudo iw wlan0 set power_save off
```

有効化されたか確認するには、以下のコマンドを実行してください。`Power Management:off` と表示されれば無効化されています。

```bash
iwconfig wlan0
```

#### 恒久的な無効化(再起動後も有効)

NetworkManager を使用している場合は、設定ファイルを追加することで永続化できます。

```bash
sudo nano /etc/NetworkManager/conf.d/wifi-powersave-off.conf
```

以下の内容を記述して保存してください。

```ini
[connection]
wifi.powersave = 2
```

保存後、NetworkManager を再起動して反映させてください。

```bash
sudo systemctl restart NetworkManager
```

### APモードと Stationモードの通信安定性の比較

WiFi 接続は、APモードよりStationモードの方が通信が安定する傾向があります。
APモードでの通信遅延が顕著な場合、Stationモードでの運用も検討してください。
なお、どちらのモードでも、Linux 側の WiFi パワーセーブを無効化することを推奨します。

</div>
</details>

# License

このプロジェクトは Apache License 2.0 のもとで公開されています。詳細は LICENSE をご覧ください。
