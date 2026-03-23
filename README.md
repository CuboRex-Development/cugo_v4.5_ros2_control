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

<!-- TODO:図の修正 -->
<img width="2527" height="1116" alt="image" src="https://github.com/user-attachments/assets/a8950d77-9907-4d95-99be-b6ca8f536b85" />

#### 対応製品

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

ROS 2 環境がない場合は [ROS 2 Documentation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html) を参照してください。

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

### 方法 A: `params.yaml` に設定する（常時 WiFi 接続する場合）

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

### 方法 B: launch 引数で一時的に上書きする

`params.yaml` を変更せず、起動時に引数で接続先を指定します。

```bash
ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py \
    comm_type:=wifi tcp_host:=192.168.1.100 tcp_port:=8080
```

## デバッグログの有効化

```bash
ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py log_level:=debug
```

# Parameters

ノードのパラメータは `config/params.yaml` で管理します。
通常の使用では、このファイルの「通信設定」と「制御設定」のみ確認・変更してください。

パラメータを変更した場合は `colcon build` を再実行してください。

---

`params.yaml` で設定できる主なパラメータは以下の通りです。

## params.yaml パラメータ

### 通信設定

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `comm_type` | `serial` | 通信方式: `serial`（USB）または `wifi` |
| `serial_port` | `/dev/ttyACM0` | シリアルポートのパス（USB接続時） |
| `serial_baudrate` | `115200` | ボーレート |
| `tcp_host` | `192.168.1.100` | WiFiモード時の接続先IPアドレス |
| `tcp_port` | `8080` | WiFiモード時の接続先ポート番号 |

> [!NOTE]
> `comm_type`、`tcp_host`、`tcp_port` は launch 引数でも上書きできます。`params.yaml` の値がデフォルトとして使用されます。

### launch 引数

`log_level`（デフォルト: `info`）: ログレベルを指定します。`debug`, `info`, `warn`, `error`, `fatal` から選択できます。

### 制御設定

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `control_frequency` | `10.0` | 制御周期 [Hz]（最大 100） |
| `cmd_vel_timeout` | `0.5` | `/cmd_vel` 受信タイムアウト [秒]（超過で速度ゼロ） |
| `serial_timeout` | `0.5` | マイコン通信タイムアウト [秒] |

### ROS フレーム・トピック設定

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `odom_frame_id` | `odom` | オドメトリフレーム名 |
| `base_link_frame_id` | `base_footprint` | ベースリンクフレーム名 |
| `subscribe_topic_name` | `/cmd_vel` | 速度指令トピック名 |
| `publish_topic_name` | `/odom` | オドメトリトピック名 |

### デバイスID

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `product_id` | `10000` | CuGo V4.5 製品識別子（変更不要） |
| `robot_id` | `0` | ロボット識別子（複数台運用時に設定） |

### 共分散（SLAM・ナビゲーション調整用）

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `pose_cov_x` | `0.04` | 位置 X 精度（0.2 m 相当） |
| `pose_cov_y` | `0.04` | 位置 Y 精度（0.2 m 相当） |
| `pose_cov_yaw` | `0.01` | Yaw 精度（0.1 rad 相当） |
| `twist_cov_linear_x` | `0.0025` | 線速度 X 精度（0.05 m/s 相当） |
| `twist_cov_linear_y` | `0.0025` | 線速度 Y 精度（0.05 m/s 相当） |
| その他 Z / Roll / Pitch | `1e9` | 2次元平面のみのため大きな値に設定 |

### デバッグログ[^debug-log]

| パラメータ | デフォルト値 | 説明 |
|-----------|-------------|------|
| `serial_debug_log` | `false` | 送受信パケット内容（COBSエンコード前/デコード後） |
| `serial_raw_debug_log` | `false` | 送受信パケット内容（生データ） |
| `callback_debug_log` | `false` | コールバック・制御ループの実行フロー |
| `odom_debug_log` | `false` | オドメトリ・速度データ |
| `param_debug_log` | `false` | 起動時のデバイスIDパラメータ |

[^debug-log]: デバッグログを確認するには `log_level:=debug` で起動してください。

# Topics

## Published Topics
- `/odom` ([nav_msgs/msg/Odometry](https://docs.ros2.org/foxy/api/nav_msgs/msg/Odometry.html))
- `/tf` ([tf2_msgs/msg/TFMessage](https://docs.ros2.org/foxy/api/tf2_msgs/msg/TFMessage.html))
- `/handshake_status` ([std_msgs/msg/Bool](https://docs.ros2.org/foxy/api/std_msgs/msg/Bool.html))

## Subscribed Topics

- `/cmd_vel` ([geometry_msgs/msg/Twist](https://docs.ros2.org/foxy/api/geometry_msgs/msg/Twist.html))

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
