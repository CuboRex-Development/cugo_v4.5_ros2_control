![image](https://github.com/user-attachments/assets/be603edd-43dd-42b7-8215-2a89df03e3c2)

# cugo_v4.5_ros2_control

CuGo V4.5 のROS 2ノードです。

<!-- TODO: cugo_v4.5_ros2_motorcontroller2のリンクを追記する -->
ROS 2 topicの`/cmd_vel`をSubscribeし、`/odom`をPublishします。
セットでArduinoスケッチの[cugo_v4.5_ros2_motorcontroller2](null)、と使用します。

ROS 2 Humble以降でご利用いただけます。

# Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Topics and Parameters](#topics-and-parameters)
- [Note](#note)
- [License](#license)

# Features

ROS 2 プログラムとロボットのプログラムとの仲介を行います。

#### 対応製品

<!-- TODO: V4.5のリンクを貼る -->
* [クローラロボット開発プラットフォーム CuGo V4.5](null)

#### 内部処理

Subscribeした`/cmd_vel`をロボットのマイコンに送信します。

<!-- TODO: cugo_v4.5_ros2_motorcontroller2のリンクを追記する -->
また、[cugo_v4.5_ros2_motorcontroller2](null)が書き込まれたロボットのマイコンから、CRST01Aの計算したオドメトリを受け取ります。受け取ったオドメトリを`/odom`としてPublishします。

<!-- TODO:図の修正 -->
<img width="2527" height="1116" alt="image" src="https://github.com/user-attachments/assets/a8950d77-9907-4d95-99be-b6ca8f536b85" />


# Requirements
- OS: Ubuntu 22.04.4 LTS / ROS Distribution: ROS 2 Humble Hawksbill
- OS: Ubuntu 24.04.4 LTS / ROS Distribution: ROS 2 Jazzy Jalisco
- xacro
- robot_state_publisher


# Installation
ROS 2環境がない場合は[ROS 2 Documentation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html)を参照しROS 2をインストールしてください。


ROS 2のワークスペース内でgit cloneしたのち、colcon buildしてください。
~~~
$ cd ~/your_ros2_ws/src
$ git clone https://github.com/CuboRex-Development/cugo_v4.5_ros2_control.git
$ cd ../..
$ colcon build --symlink-install
$ source ~/your_ros2_ws/install/local_setup.bash
~~~

ビルドエラーが発生する場合、依存パッケージをインストールしてから再度`colcon build`してください。
~~~
$ rosdep install -i --from-paths ~/your_ros2_ws/src/cugo_v4.5_ros2_control
$ cd ~/your_ros2_ws
$ colcon build --symlink-install
$ source ~/your_ros2_ws/install/local_setup.bash
~~~

# Usage

下記のコマンドでcugo_v4.5_ros2_controlノードが起動します。
付属のRaspberryPiPicoとUSBケーブルで接続をしたのち、お客様環境にあった権限設定をしてからlaunchファイルを実行してください。

~~~
# RaspberryPiPicoの権限付与例
# お客様環境に合わせてコマンドを実行してください。
$ sudo chmod 777 /dev/ttyACM0

# launch ファイルを実行
$ ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py
~~~

launchファイルのパラメータを変更することで微調整することもできます。詳細は[Parameters](#parameters)の項目を参照してください。


#### ロボット側の操作
クローラロボット開発プラットフォーム付属のRaspberryPiPicoに[こちらのスケッチ](https://github.com/CuboRex-Development/cugo_ros2_motorcontroller2)を書き込み、ROS 2 PCとRaspberryPiPicoをUSBケーブルで接続してください。
その後ROSパッケージを実行してください。自動で通信開始します。

スケッチの書き換えはROS PCである必要性はありません。

もし、うまく `/cmd_vel` 通りに走行を開始しない場合は、一度USBケーブルを抜き、
ロボットの電源を入れなおしてから再度PCとRaspberryPiPicoをUSBケーブルで接続してください。

  

# Topics and Parameters
## Published Topics
- `/odom` ([nav_msgs/msg/Odometry](https://docs.ros2.org/foxy/api/nav_msgs/msg/Odometry.html))
- `/tf` ([tf2_msgs/msg/TFMessage](https://docs.ros2.org/foxy/api/tf2_msgs/msg/TFMessage.html))
- `/handshake_status` ([std_msgs/msg/Bool](https://docs.ros2.org/foxy/api/std_msgs/msg/Bool.html))

## Subscription Topic
- `/cmd_vel` ([geometry_msgs/msg/Twist](https://docs.ros2.org/foxy/api/geometry_msgs/msg/Twist.html))

## Parameters
- `odom_frame_id (string, default: odom)`
  - オドメトリフレーム名の指定
- `base_link_frame_id (string, default: base_link)`
  - ベースリンクフレーム名の指定
  - `cugov4_5_launch.py` では、`base_footprint`としています。
- `subscribe_topic_name (string, default: /cmd_vel)`
  - Twist指示のトピック名の指定
- `publish_topic_name (string, default: /odom)`
  - Odometry出力のトピック名の指定
- `control_frequency (float, default: 10.0)`
  - マイコンへの指示、OdomのPublishの更新周期（最大100Hz）
- `serial_port (string, default: /dev/ttyACM0)`
  - RaspberryPi Picoのシリアル通信のポート名の指定
- `serial_baudrate (int, default: 115200)`
  - シリアル通信のボーレート
- `cmd_vel_timeout (float, default: 0.5)`
  - /cmd_velの通信途絶判定を決めるタイムアウト時間[秒]
  - タイムアウトしたら速度0を上書きして強制的に停止
- `serial_timeout (float, default: 0.5)`
  - マイコンの通信途絶判定を決めるタイムアウト時間[秒]
  - タイムアウトしたらodom.twistの値を0にして仮想ロボット速度をリセット
- `pose_cov_x (float, default: 0.04)`
  - Odometryの`pose.x`の共分散
  - 0.2[m]程度の精度を見込んでいる(0.2^2)
- `pose_cov_y (float, default: 0.04)`
  - Odometryの`pose.y`の共分散
  - 0.2[m]程度の精度を見込んでいる(0.2^2)
- `pose_cov_z (float, default: 1e9)`
  - Odometryの`pose.z`の共分散
  - 2次元平面のみの定義であるため、大きな値に設定している
- `pose_cov_roll (float, default: 1e9)`
  - Odometryの`orientation.q`のroll成分の共分散
  - 2次元平面のみの定義であるため、大きな値に設定している
- `pose_cov_pitch (float, default: 1e9)`
  - Odometryの`orientation.q`のpitch成分の共分散
  - 2次元平面のみの定義であるため、大きな値に設定している
- `pose_cov_yaw (float, default: 0.01)`
  - Odometryの`orientation.q`のyaw成分の共分散
  - 0.1[rad]程度の精度を見込んでいる(0.1^2)
- `twist_cov_linear_x (float, default: 0.0025)`
  - Twistの`linear.x`の共分散
  - 0.05[m/s]程度の精度を見込んでいる(0.05^2)
- `twist_cov_linear_y (float, default: 0.0025)`
  - Twistの`linear.y`の共分散
  - 0.05[m/s]程度の精度を見込んでいる(0.05^2)
- `twist_cov_angular_z (float, default: 1e9)`
  - Twistの`angular.z`の共分散
  - 2次元平面のみの定義であるため、大きな値に設定している
- `product_id (uint16_t, default: 10000)`
  - [プロダクトID](#プロダクトid) を参照
- `robot_id (uint16_t, default: 0)`
  - [ロボットID](#ロボットid) を参照
- `serial_debug_log (bool, default: false)`
  - `true` にすると、RaspberryPi Picoとの送受信パケットをDEBUGレベルでログ出力します
  - 送信パケットは `[TX]`、受信パケットは `[RX]` のプレフィックスで区別されます
  - 出力を確認するにはログレベルをdebugに設定する必要があります

    ~~~bash
    ros2 launch cugo_v4_5_ros2_control cugov4_5_launch.py log_level:=debug
    ~~~

上記のパラメータはlaunchファイルで設定されています。

# TF
CuGoを活用したロボットでTFを構築するためにxacroを利用します。

ご自身のロボットに取り付けられている部品を説明するxacroを`/urdf/parts`ディレクトリに格納してください。
デフォルトでは、
- CuGoそのものの位置関係を表現したurdfの`cugo_base.urdf.xacro`
- 部品を追加したサンプルとしてのurdfの`mid360.urdf.xacro` (MID-360は製品には付属していません。コメントアウトで無効化されています)
が格納されています。
~~~
cugo_v4.5_ros2_control
└── urdf
    ├── my_cugo_robot.urdf.xacro
    └── parts
        ├── cugov4_5_base.urdf.xacro
        └── mid360.urdf.xacro
~~~

`parts`内にある部品xacroを`my_cugo_robot.urdf.xacro`が読み込むことでロボット全体のTFを構築することができます。
`my_cugo_robot.urdf.xacro` は `robot_state_publisher` によって、ロボット構成のTFを配信します。
部品を追加する場合、`my_cugo_robot.urdf.xacro`にご自身で追加したxacro名を追記してください。

追記した後は`colcon build`を行ってください。追加したファイルが反映されます。

# Protocol
<!-- TODO: cugo_v4.5_ros2_motorcontroller2のリンクを追記する -->
[cugo_v4.5_ros2_motorcontroller2](null)と、送信・受信ともにヘッダ8バイト・ボディ64バイトの合計72バイトから構成されるデータを通信しています。
ボディデータに格納されるデータの一覧は以下の通りになります。
ボディの残りの領域は今後拡張できるように確保されているだけで、現在は00を送受信しています。

正しいデバイスと通信できているかを確認するため、初回のデータ通信でPCとロボットが想定するデバイスかを確認します。
確認が取れるまで、ロボットは走行を開始しません。

### Arduinoドライバへの送信データ

#### ヘッダ
Data Name      | Data Type  | Data Size(byte) | Start Address in PacketHeader | Data Description
---------------|------------|-----------------|-------------------------------|--------------------
product_id     | uint16_t   | 2               | 0                             | 接続先として期待する[プロダクトID](#プロダクトid)
robot_id       | uint16_t   | 2               | 2                             | 接続先として期待する[ロボットID](#ロボットid)
length         | uint16_t   | 2               | 4                             | ヘッダを含む通信データの長さ、72固定
checksum       | uint16_t   | 2               | 6                             | ボディデータのチェックサム



#### ボディ

Data Name            | Data Type  | Data Size(byte) | Start Address in PacketBody | Data Description
---------------------|------------|-----------------|-----------------------------|--------------------
target_x_speed       | int16_t    | 2               | 0                           | x方向目標速度(x0.001 m/s)
target_y_speed       | int16_t    | 2               | 2                           | y方向目標速度(x0.001 m/s)、0固定
target_theta_speed   | int16_t    | 2               | 4                           | z軸回りの回転方向目標速度(x0.001 rad/s)
Nan                  | Nan        | 54              | 6                           | 使用しない、0
product_id           | uint16_t   | 2               | 60                          | 接続先として期待する[プロダクトID](#プロダクトid)
robot_id             | uint16_t   | 2               | 62                          | 接続先として期待する[ロボットID](#ロボットid)


### Arduinoドライバからの受信データ

#### ヘッダ
Data Name      | Data Type  | Data Size(byte) | Start Address in PacketHeader | Data Description
---------------|------------|-----------------|-------------------------------|--------------------
product_id     | uint16_t   | 2               | 0                             | 自身の[プロダクトID](#プロダクトid)
robot_id       | uint16_t   | 2               | 2                             | 自身の[ロボットID](#ロボットid)
length         | uint16_t   | 2               | 4                             | ヘッダを含む通信データの長さ、72固定
checksum       | uint16_t   | 2               | 6                             | ボディデータのチェックサム

#### ボディ

Data Name             | Data Type  | Data Size(byte) | Start Address in PacketBody | Data Description
----------------------|------------|-----------------|-----------------------------|-----------------
current_x_speed       | int16_t    | 2               | 0                           | x方向現在速度(x0.001 m/s)
current_y_speed       | int16_t    | 2               | 2                           | y方向現在速度(x0.001 m/s)、0固定
current_theta_speed   | int16_t    | 2               | 4                           | z軸回りの回転方向現在速度(x0.001 rad/s)
Nan                   | Nan        | 54              | 6                           | 使用しない、0
product_id            | uint16_t   | 2               | 60                          | 自身の[プロダクトID](#プロダクトid)
robot_id              | uint16_t   | 2               | 62                          | 自身の[ロボットID](#ロボットid)

### プロダクトID
CuboRexから販売する製品ごとに、プロダクトIDを設定しています。
通信プロトコルに互換性があるデバイスは、プロダクトIDの10000 の位を同じ値としています。
プロダクトIDの10000 の位が異なるデバイスを接続した際には、通信に失敗します。


| 製品    | プロダクトID |
| ------- | -----------: |
| CuGo V4 |       10000 |

### ロボットID
ロボットを複数台使用する際に、ロボットの識別のために使用する値です。
現在のバージョンではロボットIDに応じたプログラムは実装していないため、ロボットIDを変えても処理は変わりません。


# Note

ご不明点がございましたら、[お問い合わせフォーム](https://cuborex.com/contact/)にてお問い合わせください。回答いたします。


# License
このプロジェクトはApache License 2.0のもと、公開されています。詳細はLICENSEをご覧ください。
