<!-- TODO:図の修正 -->
![image](https://github.com/user-attachments/assets/be603edd-43dd-42b7-8215-2a89df03e3c2)

# cugo_v4.5_ros2_control

CuGo V4.5 用 ROS 2 コントロールノードです。

`/cmd_vel` を Subscribe してロボットに速度指令を送信し、`/odom` と `/tf` を Publish します。
マイコン側のスケッチ [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) と組み合わせて使用します。

ROS 2 Humble 以降で動作します。

# Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Parameters](#parameters)
- [Topics](#topics)
- [TF](#tf)
- [Note](#note)
- [License](#license)

# Features

`/cmd_vel` で受け取った速度指令をロボットのマイコンに送信します。
マイコン（[cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller)）から受け取った現在速度をもとにオドメトリを計算し、`/odom` として Publish します。

<img width="4077" height="2541" alt="cugo_v4 5_ros2_control" src="https://github.com/user-attachments/assets/49c19ceb-2415-420f-85b9-ba4f251c39da" />

## 通信モード

### USB-Serial 接続 (デフォルト)

PC と Raspberry Pi Pico 2 WH を USB ケーブルで直接接続する方式です。
追加機器なしに最も簡単に使用できます。

<img width="4194" height="769" alt="USB" src="https://github.com/user-attachments/assets/66af2424-27d8-4efd-8315-5d9fad49f2af" />

### WiFi 接続

外部 WiFi ルータ経由で PC とロボット間を TCP 接続する方式です。
ケーブルを使わずに無線でロボットを操作できます。
接続には事前にロボット側の WiFi 設定が必要です（[cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照）。

<img width="4194" height="1360" alt="WiFi" src="https://github.com/user-attachments/assets/8f83a93d-7343-4791-ad11-65093e72138a" />

## 対応製品

<!-- TODO: V4.5のリンクを貼る -->
* [CuGo V4.5](null)

# Requirements

- OS / ROS ディストリビューション
  - Ubuntu 22.04 LTS / ROS 2 Humble Hawksbill
  - Ubuntu 24.04 LTS / ROS 2 Jazzy Jalisco
- xacro
- robot_state_publisher
- socat（WiFi接続モードを使用する場合のみ）

  ```bash
  sudo apt install socat
  ```

# Installation

ROS 2 環境がない場合は [ROS 2 Documentation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html) を参照し、ROS 2 のインストールを実施してください。

```bash
cd ~/your_ros2_ws/src
git clone https://github.com/CuboRex-Development/cugo_v4.5_ros2_control.git
cd ../..
colcon build --symlink-install
source ~/your_ros2_ws/install/local_setup.bash
```

ビルドエラーが発生する場合は、依存パッケージをインストールしてから再度 `colcon build` してください。

```bash
rosdep install -i --from-paths ~/your_ros2_ws/src/cugo_v4.5_ros2_control
cd ~/your_ros2_ws
colcon build --symlink-install
source ~/your_ros2_ws/install/local_setup.bash
```

# Usage

> [!IMPORTANT]
> いずれの通信モードでも、事前にマイコン（Raspberry Pi Pico 2 WH）に [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) のスケッチを書き込んでおく必要があります。

## USB-Serial 接続（デフォルト）

1. CuGo V4.5 の Raspberry Pi Pico 2 WH と PC を USB ケーブルで接続します。

2. シリアルポートへのアクセス権を付与します（環境に合わせて変更してください）。

   ```bash
   sudo chmod 777 /dev/ttyACM0
   ```

3. ノードを起動します。

   ```bash
   ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py
   ```

> [!TIP]
> 正常に通信が開始されない場合は、USB ケーブルを抜き差しして再接続してください。

## WiFi 接続（外部ルータ経由）

ロボット側の WiFi 設定は [cugo_v4.5_ros2_motorcontroller](https://github.com/CuboRex-Development/cugo_v4.5_ros2_motorcontroller) を参照してください。

launchファイルが内部で socat を自動起動します。

`config/params.yaml` を編集して `comm_type`、`tcp_host`、`tcp_port` を設定します。

```yaml
cugo_v4_5_ros2_control:
  ros__parameters:
    comm_type: wifi
    tcp_host: 192.168.1.100   # ← ロボットの IP アドレス
    tcp_port: 8080
```

その後、通常通り起動します。

```bash
ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py
```

# Parameters

ノードのパラメータは `config/params.yaml` で管理します。
通常の使用では、このファイルの「通信設定」と「制御設定」のみ確認・変更してください。

パラメータを変更した場合は `colcon build` を再実行してください。

---

`params.yaml` で設定できる主なパラメータは以下の通りです。

## params.yaml パラメータ

### 通信設定

| パラメータ        | デフォルト値    | 説明                                   |
| ----------------- | --------------- | -------------------------------------- |
| `comm_type`       | `serial`        | 通信方式: `serial`（USB）または `wifi` |
| `serial_port`     | `/dev/ttyACM0`  | シリアルポートのパス（USB接続時）      |
| `serial_baudrate` | `115200`        | ボーレート                             |
| `tcp_host`        | `192.168.1.100` | WiFiモード時の接続先IPアドレス         |
| `tcp_port`        | `8080`          | WiFiモード時の接続先ポート番号         |

> [!NOTE]
> `comm_type`、`tcp_host`、`tcp_port` は launch 引数でも上書きできます。`params.yaml` の値がデフォルトとして使用されます。

### launch 引数

`log_level`（デフォルト: `info`）: ログレベルを指定します。`debug`, `info`, `warn`, `error`, `fatal` から選択できます。

### 制御設定

| パラメータ          | デフォルト値 | 説明                                               |
| ------------------- | ------------ | -------------------------------------------------- |
| `control_frequency` | `10.0`       | 制御周期 [Hz]（最大 100）                          |
| `cmd_vel_timeout`   | `0.5`        | `/cmd_vel` 受信タイムアウト [秒]（超過で速度ゼロ） |
| `serial_timeout`    | `0.5`        | マイコン通信タイムアウト [秒]                      |

### ROS フレーム・トピック設定

| パラメータ             | デフォルト値     | 説明                   |
| ---------------------- | ---------------- | ---------------------- |
| `odom_frame_id`        | `odom`           | オドメトリフレーム名   |
| `base_link_frame_id`   | `base_footprint` | ベースリンクフレーム名 |
| `subscribe_topic_name` | `/cmd_vel`       | 速度指令トピック名     |
| `publish_topic_name`   | `/odom`          | オドメトリトピック名   |

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

### デバッグログ

| パラメータ             | デフォルト値 | 説明                                                    |
| ---------------------- | ------------ | ------------------------------------------------------- |
| `serial_debug_log`     | `false`      | 送受信パケット内容（COBSエンコード前/デコード後）のログ |
| `serial_raw_debug_log` | `false`      | 送受信パケット内容（生データ）のログ                    |
| `callback_debug_log`   | `false`      | コールバック・制御ループの実行フローのログ              |
| `odom_debug_log`       | `false`      | オドメトリ・速度データのログ                            |
| `param_debug_log`      | `false`      | 起動時のデバイスIDパラメータのログ                      |

> [!NOTE]
> デバッグログを確認するには、各パラメータを `true` に設定した上で `log_level:=debug` で起動してください。
>
> ```bash
> ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py log_level:=debug
> ```

# Topics

## Published Topics

- `/odom` ([nav_msgs/msg/Odometry](https://docs.ros2.org/foxy/api/nav_msgs/msg/Odometry.html))
  ロボットの位置・姿勢・速度情報（オドメトリ）を配信します。マイコンから受け取った車輪速度をもとに計算されます。

- `/tf` ([tf2_msgs/msg/TFMessage](https://docs.ros2.org/foxy/api/tf2_msgs/msg/TFMessage.html))
  `odom` フレームから `base_footprint` フレームへの座標変換を配信します。

- `/handshake_status` ([std_msgs/msg/Bool](https://docs.ros2.org/foxy/api/std_msgs/msg/Bool.html))
  マイコンとのハンドシェイク（接続確立）状態を配信します。接続中は `true`、未接続または切断時は `false` になります。

## Subscribed Topics

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
        ├── cugov4_5_base.urdf.xacro   ← CuGo 本体の位置関係
        └── mid360.urdf.xacro          ← センサ搭載サンプル（デフォルトで無効）
```

部品を追加する場合は `my_cugo_robot.urdf.xacro` に追記し、`colcon build` を再実行してください。

# Note

通信プロトコルの詳細仕様は [PROTOCOL.md](PROTOCOL.md) を参照してください。

ご不明点は [お問い合わせフォーム](https://cuborex.com/contact/) よりお問い合わせください。

# License

このプロジェクトは Apache License 2.0 のもとで公開されています。詳細は LICENSE をご覧ください。
