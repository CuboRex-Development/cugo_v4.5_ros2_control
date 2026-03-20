import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, OpaqueFunction, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
import xacro

# WiFiモード時に生成する仮想シリアルポートのパス
VSERIAL_PATH = '/tmp/cugo_vserial'

# WiFiモード時にノード起動を遅延させる秒数（socatがPTYを生成するまでの待機）
WIFI_NODE_DELAY_SEC = 2.0


def generate_launch_description():
    # ログレベルの変更
    log_level_arg = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='Log level: debug, info, warn, error, fatal'
    )
    log_level = LaunchConfiguration('log_level')

    # 通信方式の選択
    comm_type_arg = DeclareLaunchArgument(
        'comm_type',
        # default_value='wifi',
        default_value='serial',
        description='通信方式: "serial"（デフォルト）または "wifi"'
    )

    # WiFiモード用パラメータ（comm_type:="wifi" のときのみ使用）
    tcp_host_arg = DeclareLaunchArgument(
        'tcp_host',
        default_value='192.168.1.100',
        description='WiFiモード時の接続先IPアドレス'
    )
    tcp_port_arg = DeclareLaunchArgument(
        'tcp_port',
        default_value='8080',
        description='WiFiモード時の接続先ポート番号'
    )

    # ---- robot_state_publisher の設定 ----
    # パッケージの共有ディレクトリのパスを取得
    pkg_share = get_package_share_directory('cugo_v4_5_ros2_control')

    # トップレベルのxacroファイルへのパス
    xacro_file = os.path.join(pkg_share, 'urdf', 'my_cugo_robot.urdf.xacro')
    doc = xacro.process_file(xacro_file)
    robot_description_config = {'robot_description': doc.toxml()}

    # robot_state_publisherノードの定義
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description_config]
    )

    # ---- cugo_v4_5_ros2_control の設定 ----
    # パラメータの定義
    parameters = {
        'odom_frame_id': 'odom',
        'base_link_frame_id': 'base_footprint',
        'subscribe_topic_name': '/cmd_vel',
        'publish_topic_name': '/odom',
        'control_frequency': 10.0,  # MAX:100.0
        'serial_port': '/dev/ttyACM0',
        'serial_baudrate': 115200,
        'cmd_vel_timeout': 0.5,  # 秒
        'serial_timeout': 0.5,  # 秒
        'product_id': 10000,  # cugov4.5の識別子
        'serial_debug_log': True,  # シリアル通信内容のデバッグログ出力 (COBSエンコード前/デコード後)
        'serial_raw_debug_log': False,  # シリアル通信内容のデバッグログ出力 (生データ)
        'callback_debug_log': False,  # コールバック・制御ループの実行フローのデバッグログ出力
        'odom_debug_log': False,  # オドメトリ・速度データのデバッグログ出力
        'param_debug_log': False,  # 起動時のデバイスIDパラメータのデバッグログ出力

        # 共分散の設定
        # SLAMやLocalizationで調整
        # オドメトリで悪影響がある場合は調整してください
        # オドメトリの自己位置
        'pose_cov_x': 0.04,  # 0.2m^2
        'pose_cov_y': 0.04,  # 0.2m^2
        'pose_cov_z': 1e9,
        'pose_cov_roll': 1e9,
        'pose_cov_pitch': 1e9,
        'pose_cov_yaw': 0.01,  # 0.1rad^2

        # ロボットの速度
        'twist_cov_linear_x': 0.0025,  # 0.05m^2
        'twist_cov_linear_y': 0.0025,  # 0.05m^2
        'twist_cov_angular_z': 1e9,
    }

    def launch_setup(context, *args, **kwargs):
        comm_type = LaunchConfiguration('comm_type').perform(context)
        tcp_host = LaunchConfiguration('tcp_host').perform(context)
        tcp_port = LaunchConfiguration('tcp_port').perform(context)

        # 通信方式に応じてシリアルポートを決定
        # WiFiモードではsocatが生成する仮想シリアルポートを使用
        if comm_type == 'wifi':
            serial_port = VSERIAL_PATH
        else:
            serial_port = parameters['serial_port']

        # ノードの定義
        cugo_node = Node(
            package='cugo_v4_5_ros2_control',
            executable='cugo_v4_5_ros2_control',
            name='cugo_v4_5_ros2_control',
            output='screen',
            parameters=[{**parameters, 'serial_port': serial_port}],
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', log_level]
        )

        if comm_type == 'wifi':
            # socatで仮想シリアルポートを生成し、TCP接続にブリッジ
            # PTYの生成を待つため、cugo_nodeはWIFI_NODE_DELAY_SEC秒遅延起動
            socat_process = ExecuteProcess(
                cmd=[
                    'socat',
                    f'PTY,link={VSERIAL_PATH},raw,echo=0',
                    f'TCP:{tcp_host}:{tcp_port}'
                ],
                output='screen',
                name='socat_wifi_bridge'
            )
            return [
                socat_process,
                TimerAction(period=WIFI_NODE_DELAY_SEC, actions=[cugo_node])
            ]
        else:
            return [cugo_node]

    return LaunchDescription([
        log_level_arg,  # DeclareLaunchArgumentをLaunchDescriptionに含める
        comm_type_arg,
        tcp_host_arg,
        tcp_port_arg,
        robot_state_publisher_node,
        OpaqueFunction(function=launch_setup)
    ])
