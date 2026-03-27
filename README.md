<!-- TODO:図の修正 -->
![image](https://github.com/user-attachments/assets/be603edd-43dd-42b7-8215-2a89df03e3c2)

# cugo_v4.5_ros2_control

CuGo V4.5 用 ROS 2 コントロールノードです。

`/cmd_vel` を Subscribe してロボットに速度指令を送信し、`/odom` と `/tf` を Publish します。
マイコン側のスケッチ [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) と組み合わせて使用します。

ROS 2 Humble 以降で動作します。

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

<img width="4077" height="2541" alt="cugo_v4 5_ros2_control" src="https://github.com/user-attachments/assets/49c19ceb-2415-420f-85b9-ba4f251c39da" />


本ノードを実行しているROS2 PC とCuGo V4.5 との通信方式は USB-Serial をデフォルトとしていますが、その他の通信方式も選択することができます。

# Connection

### USB-Serial 接続 (デフォルト)

PC と Raspberry Pi Pico 2 WH を USB ケーブルで直接接続する方式です。  
追加機器なしに最も簡単に使用できます。

<img width="4194" height="769" alt="USB" src="https://github.com/user-attachments/assets/66af2424-27d8-4efd-8315-5d9fad49f2af" />

### BOXコネクタ-Serial接続

CuGo V4.5 本体の BOX コネクタを介して PC とシリアル通信する方式です。  
USB-UART 変換アダプタなどを使用して PC と接続するほか、UARTポートを持つ機器とUSBなしに接続が可能です。接続ケーブルは、ご自身でご用意ください。
ROS 側の設定はは USB-Serial 接続と同じ `serial` モードのまま使用できます。必要に応じて、シリアルポートのパス（`serial_port`）のみ変更してください。

<img width="4193" height="769" alt="UART_BOX" src="https://github.com/user-attachments/assets/6bbd0c41-efdf-4b65-a32e-bf5ceedd2683" />

### WiFi APモード

Raspberry Pi Pico 2 WH 自身がアクセスポイントとして動作する方式です。  
WiFi ルータを用意せずに無線でロボットを操作できます。
接続には事前にロボット側の WiFi 設定が必要です。（[cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照）

<img width="4193" height="977" alt="WiFi_AP" src="https://github.com/user-attachments/assets/72a4a2ee-1ad5-43bc-9510-1fcc29f251fc" />


### WiFi Stationモード（外部ルータ経由）

外部 WiFi ルータ経由で PC とロボット間を TCP 接続する方式です。  
通信を行うために、WiFiルータが必要です。WiFiルータは、ご自身でご用意ください。
接続には事前にロボット側の WiFi 設定が必要です。（[cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照）

<img width="4193" height="1559" alt="WiFI_Station" src="https://github.com/user-attachments/assets/c32b6186-fb94-409d-bde8-2cfe2e692ac0" />


# Requirements

### OS / ROS ディストリビューション
  - Ubuntu 24.04 LTS / ROS 2 Jazzy Jalisco

### 依存パッケージ
  - xacro
  - robot_state_publisher
  - joint_state_publisher_gui（URDF確認・デバッグ用）
  - socat（WiFiモード〔APモード・Stationモード〕を使用する場合のみ）

# Installation

以下のコマンド中の `your_ros2_ws` は、ご自身のワークスペース名に置き換えてください。

1. ROS 2 環境を用意します。

   本ノードを実行するPCに ROS 2 環境が導入されていない場合は、 [ROS 2 Documentation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html) を参照し、ROS 2 のインストールを実施してください。


2. cugo_v4.5_ros2_control ノードをクローンします。

   ```bash
   cd ~/your_ros2_ws/src
   git clone https://github.com/CuboRex-Development/cugo_v4.5_ros2_control.git
   ```

3. rosdepで依存パッケージをインストールします。

   ```bash
   rosdep install -i --from-paths ~/your_ros2_ws/src/cugo_v4.5_ros2_control
   ```

4. cugo_v4.5_ros2_control ノードをビルドします。

   ```bash
   cd ~/your_ros2_ws
   colcon build --symlink-install
   source ~/your_ros2_ws/install/local_setup.bash
   ```



# Usage

> [!IMPORTANT]
> いずれの通信モードでも、事前にマイコン（Raspberry Pi Pico 2 WH）に [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) のスケッチを書き込んでおく必要があります。事前に、cugo_v4.5_ros2_motorcontrollerのInstallationの手順を実施してください。

### USB-Serial 接続（デフォルト）

1. CuGo V4.5 の Raspberry Pi Pico 2 WH と PC を USB ケーブルで接続します。

2. シリアルポートへのアクセス権を付与します（環境に合わせてポート名やアクセス権限を変更してください）。

   ```bash
   sudo chmod 777 /dev/ttyACM0
   ```

3. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```


### BOXコネクタ-Serial接続

1. CuGo V4.5 本体の BOX コネクタと PC を USB-シリアル変換アダプタで接続します。
   
   <img width="2248" height="723" alt="ボックスコネクタ" src="https://github.com/user-attachments/assets/ac5aba45-c2ac-437f-b912-c875c6647a30" />

   **J28 ピンアサイン**
   
   | ピン番号 | Raspberry Pi Pico 2WH GP |                      機能 |
   | :-----: | :----------------------: | :-----------------------: |
   |       7 |           GP8 (UART1 TX) | TxD (ROS側機器のRxDを接続) |
   |       8 |           GP9 (UART1 RX) | RxD (ROS側機器のTxDを接続) |
   |      20 |                      GND |                       GND |

3. デバイスが認識されているか確認します。

   ```bash
   ls /dev/ttyUSB*
   ```

4. シリアルポートへのアクセス権を付与します（環境に合わせてポート名やアクセス権限を変更してください）。

   ```bash
   sudo chmod 777 /dev/ttyUSB0
   ```

5. `config/params.yaml` の `serial_port` を接続したポートに変更します（`comm_type: serial` のままで問題ありません）。

   ```yaml
   cugo_v4_5_ros2_control:
     ros__parameters:
       comm_type: serial
       serial_port: /dev/ttyUSB0   # ← BOXコネクタ接続時のポート名
   ```

6. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugo_v4_5_launch.py
   ```

### WiFi APモード（ルータ不要）

ロボット側の WiFi AP 設定は [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照してください。

1. PC の WiFi を、ロボット側で設定したアクセスポイント（デフォルト SSID: `CuGo_AP`）に接続します。

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

### WiFi Stationモード（外部ルータ経由）

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

ロボットモデルを RViz2 で表示して URDF を確認できます。

```bash
ros2 launch cugo_v4_5_ros2_control display.launch.py
```

RViz2 が起動したら、以下の設定を行ってください。

1. Fixed Frame を `base_footprint` に設定
2. Add → `RobotModel` を追加し、`Description Topic` を `/robot_description` に設定

# Parameters

ノードのパラメータは `config/params.yaml` で管理します。
通常の使用では、このファイルの「通信設定」と「制御設定」のみ確認・変更してください。

パラメータを変更した場合は `colcon build` を再実行してください。

`params.yaml` で設定できる主なパラメータは以下の通りです。


### 通信設定

| パラメータ        | デフォルト値    | 説明                                                                                                                |
| ----------------- | --------------- | ------------------------------------------------------------------------------------------------------------------- |
| `comm_type`       | `serial`        | 通信方式: `serial`（USB）または `wifi`（APモード・Stationモード共通）                                               |
| `serial_port`     | `/dev/ttyACM0`  | シリアルポートのパス（USB接続時）                                                                                   |
| `serial_baudrate` | `115200`        | ボーレート                                                                                                          |
| `tcp_host`        | `192.168.1.100` | WiFiモード時の接続先IPアドレス（APモードは `192.168.42.1`がデフォルト、`params.yaml` では `192.168.42.1` に設定済） |
| `tcp_port`        | `8080`          | WiFiモード時の接続先ポート番号                                                                                      |

### launch 引数

以下のパラメータは `params.yaml` の編集に加えて、launch 引数でも上書きできます。

| 引数        | デフォルト値    | 説明                                                                                                                |
| ----------- | --------------- | ------------------------------------------------------------------------------------------------------------------- |
| `comm_type` | `serial`        | 通信方式: `serial` または `wifi`                                                                                    |
| `tcp_host`  | `192.168.1.100` | WiFiモード時の接続先IPアドレス（APモードは `192.168.42.1`がデフォルト、`params.yaml` では `192.168.42.1` に設定済） |
| `tcp_port`  | `8080`          | WiFiモード時の接続先ポート番号                                                                                      |
| `log_level` | `info`          | ログレベル: `debug`, `info`, `warn`, `error`, `fatal`                                                               |

### 制御設定

| パラメータ               | デフォルト値 | 説明                                                                                                                                                                                                                                                                                                                    |
| ------------------------ | ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `control_frequency`      | `10.0`       | 制御周期 [Hz]（通常 10Hz、最大 20Hz 程度（無線接続時）、最大 50Hz（優先接続時））                                                                                                                                                                                                                                                                                            |
| `cmd_vel_timeout`        | `0.5`        | 他ノードからの`/cmd_vel` 受信タイムアウト [秒]（超過で速度ゼロ）                                                                                                                                                                                                                                                        |
| `serial_timeout`         | `0.5`        | マイコンからの受信タイムアウト [秒]（超過で再接続）。WiFi接続（APモード・Stationモード）では通信レイテンシが有線より高いため、`1.0`〜`2.0` を推奨                                                                                                                                                                       |
| `max_consecutive_errors` | `5`          | 連続デコードエラーの許容回数。この回数を超えると受信バッファをフラッシュしてフレーミングを再同期します。通信品質が低い環境では大きくし、即座に再同期したい場合は小さくしてください                                                                                                                                      |
| `response_lost_timeout`  | `0.0`        | 応答ロスト判定時間 [秒]（`0.0` で無効）。ロボットへのリクエスト送信後、この時間内に応答がなければ再度リクエストを送信します。**値により制御周期が大きく低下する可能性があります。** 通信遅延への対策としてのみ使用してください。詳細は [Note > 通信遅延対策について](#通信遅延対策について) を参照 |

> [!CAUTION]
> `response_lost_timeout` は通信遅延の対策としてのみ使用してください。0.0以外の値を設定すると制御周期が低下する可能性があります。設定前に必ず [Note > 通信遅延対策について](#通信遅延対策について) を熟読し、適切な値を設定してください。

### ROS フレーム・トピック設定

| パラメータ             | デフォルト値 | 説明                                                                          |
| ---------------------- | ------------ | ----------------------------------------------------------------------------- |
| `odom_frame_id`        | `odom`       | オドメトリフレーム名                                                          |
| `base_link_frame_id`   | `base_link`  | ベースリンクフレーム名(params.yaml では `base_footprint`に設定されています。) |
| `subscribe_topic_name` | `/cmd_vel`   | 速度指令トピック名                                                            |
| `publish_topic_name`   | `/odom`      | オドメトリトピック名                                                          |

### デバイスID

| パラメータ   | デフォルト値 | 説明                                               |
| ------------ | ------------ | -------------------------------------------------- |
| `product_id` | `10000`      | CuGo V4.5 のプロダクトID（変更不要）               |
| `robot_id`   | `0`          | ロボット識別子（複数台運用時に設定することを想定） |

### 共分散（SLAM・ナビゲーション調整用）

| パラメータ            | デフォルト値 | 説明                                          |
| --------------------- | ------------ | --------------------------------------------- |
| `pose_cov_x`          | `0.04`       | 位置 X 精度（0.2 m 相当）                     |
| `pose_cov_y`          | `0.04`       | 位置 Y 精度（0.2 m 相当）                     |
| `pose_cov_z`          | `1.0e9`      | 位置 Z（2次元平面のみのため大きな値に設定）   |
| `pose_cov_roll`       | `1.0e9`      | Roll（2次元平面のみのため大きな値に設定）     |
| `pose_cov_pitch`      | `1.0e9`      | Pitch（2次元平面のみのため大きな値に設定）    |
| `pose_cov_yaw`        | `0.01`       | Yaw 精度（0.1 rad 相当）                      |
| `twist_cov_linear_x`  | `0.0025`     | 線速度 X 精度（0.05 m/s 相当）                |
| `twist_cov_linear_y`  | `0.0025`     | 線速度 Y 精度（0.05 m/s 相当）                |
| `twist_cov_angular_z` | `1.0e9`      | 角速度 Z（2次元平面のみのため大きな値に設定） |

### ログ設定

すべてのログフラグは `false` がデフォルトです。`true` に設定すると、ノード起動時から ログが出力されます。(`cugo_v4_5_launch.py` で読み取る `params.yaml` では一部のログが有効化されています。)

| パラメータ               | デフォルト値 | 説明                                        |
| ------------------------ | ------------ | ------------------------------------------- |
| `param_info_log`         | `false`      | 起動時の設定パラメータ一覧                  |
| `odom_pos_info_log`      | `false`      | Odometry 位置成分（x, y, yaw）              |
| `odom_vel_info_log`      | `false`      | Odometry 速度成分（Vx, Vy, Omega）          |
| `recv_interval_info_log` | `false`      | データ受信間隔（ms）                        |
| `packet_error_info_log`  | `false`      | パケットデコードエラー統計（累積件数）      |
| `connection_info_log`    | `false`      | 接続状態変化ログ（接続・再接続）            |
| `connection_lost_log`    | `false`      | シリアル通信未達 WARN（再接続待機中に出力） |
| `received_speed_log`     | `false`      | マイコンから受信した速度（Vx, Vy, Omega）   |
| `cmd_vel_log`            | `false`      | `/cmd_vel` で受信した速度指令値             |
| `tx_cmd_log`             | `false`      | マイコンへ実際に送信した速度指令値          |
| `loop_interval_log`      | `false`      | 制御ループの実行間隔（ms）                  |
| `handshake_log`          | `false`      | ハンドシェイク状態遷移の詳細                |
| `serial_log`             | `false`      | 送受信パケット内容（COBSデコード後）        |
| `serial_raw_log`         | `false`      | 送受信パケット内容（生データ）              |
| `callback_log`           | `false`      | コールバック・制御ループの実行フロー        |


# Topics

### Published Topics

- `/odom` ([nav_msgs/msg/Odometry](https://docs.ros2.org/foxy/api/nav_msgs/msg/Odometry.html))  
  ロボットの位置・姿勢・速度情報（オドメトリ）を配信します。マイコンから受け取った車輪速度をもとに計算されます。

- `/tf` ([tf2_msgs/msg/TFMessage](https://docs.ros2.org/foxy/api/tf2_msgs/msg/TFMessage.html))  
  `odom` フレームから `base_footprint` フレームへの座標変換を配信します。

- `/handshake_status` ([std_msgs/msg/Bool](https://docs.ros2.org/foxy/api/std_msgs/msg/Bool.html))  
  マイコンとのハンドシェイク（接続確立）状態を配信します。接続中は `true`、未接続または切断時は `false` になります。

### Subscribed Topics

- `/cmd_vel` ([geometry_msgs/msg/Twist](https://docs.ros2.org/foxy/api/geometry_msgs/msg/Twist.html))  
  ロボットへの速度指令を受信します。`linear.x`（前後）と `angular.z`（旋回）を使用します。

# TF

CuGo を活用したロボットで TF を構築するために xacro を利用します。
ご自身のロボットに取り付けた部品を記述した xacro を `urdf/parts` に格納してください。

```
cugo_v4.5_ros2_control
└── urdf
    ├── my_cugo_robot.urdf.xacro   ← 部品 xacro を読み込むトップレベルファイル
    └── parts
        ├── cugo_v4_5_base.urdf.xacro   ← CuGo 本体の位置関係
        └── mid360.urdf.xacro          ← センサ搭載サンプル（デフォルトで無効）
```

部品を追加する場合は `my_cugo_robot.urdf.xacro` に追記し、`colcon build` を再実行してください。

# Note

通信プロトコルの詳細仕様は [PROTOCOL.md](PROTOCOL.md) を参照してください。

ご不明点は [お問い合わせフォーム](https://cuborex.com/contact/) よりお問い合わせください。


## 通信遅延対策について
<details> <summary> response_lost_timeout の設定方法 </summary> 
<div>

### 概要

`response_lost_timeout` は、ロボットへ送信したリクエストへの応答が得られない場合のレスポンス待機時間です。
通常運用では `0`（無効）のままお使いください。WiFi 接続などで通信遅延が問題になる場合にのみ設定してください。

### 仕組み

1. コントロールループで目標速度コマンドを送信（リクエスト）
2. 有効なパケットを 1 件受信（レスポンス）したら、次のリクエスト送信を許可
3. `response_lost_timeout` 秒を超えても応答がない場合、「レスポンスロスト」と判定し次のリクエスト送信を許可

### 実効制御周波数への影響

`response_lost_timeout` を設定すると、応答が得られない期間は TX が抑制されます。最悪ケースでの実効的な制御周波数は以下のようになります。

```
実効制御周波数 ≈ 1 / response_lost_timeout
```

例: `response_lost_timeout = 0.5` の場合、制御周波数は最悪 2 Hz まで低下する可能性があります。
低周期での制御で問題がない用途にのみ使用してください。

### 設定値の下限

`response_lost_timeout` は `1 / control_frequency`（制御周期）より大きい値を設定してください。
それ以下の値では、制御ループのたびにタイムアウトが発生し、機能が実質無効と同じになります。

> 例: `control_frequency = 50.0`（20 ms 周期）の場合、`response_lost_timeout > 0.02` を推奨

### 応答遅延パケットの処理

「レスポンスロスト」判定後に次のリクエストを送信した直後、前のリクエストへの遅延応答が届く場合があります。
この場合、遅延応答は有効なパケットとして処理されますが、前回受信からの経過時間（`dt`）が長くなるため、オドメトリの誤差が一時的に増大する可能性があります。

### 推奨値

| 接続方式                        | 推奨値       |
| ------------------------------- | ------------ |
| USB-Serial（有線）              | `0.0`        |
| WiFi（APモード・Stationモード） | `0.5`〜`1.0` |

### 制約事項

`response_lost_timeout` は `serial_timeout` 未満の値を設定してください。
`serial_timeout` 以上の値を設定した場合、起動時に自動的に `0`（無効）に補正され、WARNログが出力されます。

</div>
</details>

## WiFi 通信の安定化について

<details> <summary> 無線通信が安定しない際の対策例 </summary> 
<div>

### WiFi パワーセーブモードの無効化

Linux の WiFi アダプタがパワーセーブモードで動作している場合、受信データが遅延することがあります。
WiFi 接続時に応答が周期的にロストする（`[response lost]` ログが頻繁に出る）場合は、パワーセーブを無効化することで改善する場合があります。

#### 一時的な無効化（再起動で元に戻る）

```bash
sudo iw wlan0 set power_save off
```

有効化されたか確認するには、以下のコマンドを実行してください。`Power Management:off` と表示されれば無効化されています。

```bash
iwconfig wlan0
```

#### 恒久的な無効化（再起動後も有効）

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

### AP モードと Station モードの通信安定性の比較

WiFi 接続は、APモードよりStation モードの方が通信が安定する傾向があります。
APモードでの通信遅延が顕著な場合、Stationモードでの運用も検討してください。
なお、どちらのモードでも、Linux 側の WiFi パワーセーブを無効化することを推奨します。

</details>

# License

このプロジェクトは Apache License 2.0 のもとで公開されています。詳細は LICENSE をご覧ください。
