import os
import yaml

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

# Bluetoothモード時のRFCOMMデバイスパス
BT_RFCOMM_PATH = '/dev/rfcomm0'

# Bluetoothモード時にノード起動を遅延させる秒数（rfcomm bindが完了するまでの待機）
BT_NODE_DELAY_SEC = 2.0


def _load_launch_defaults(params_file: str) -> dict:
    """params.yaml を読み込み、launch用デフォルト値を返す。
    ファイルが見つからない場合は空の dict を返す。"""
    try:
        with open(params_file, 'r') as f:
            data = yaml.safe_load(f)
        return (data or {}).get('cugo_v4_5_ros2_control', {}).get('ros__parameters', {})
    except FileNotFoundError:
        return {}


def generate_launch_description():
    pkg_share = get_package_share_directory('cugo_v4_5_ros2_control')
    params_file = os.path.join(pkg_share, 'config', 'params.yaml')

    # params.yaml からlaunch用デフォルト値を取得
    launch_defaults = _load_launch_defaults(params_file)

    log_level_arg = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='ログレベル: debug, info, warn, error, fatal'
    )

    comm_type_arg = DeclareLaunchArgument(
        'comm_type',
        default_value=launch_defaults.get('comm_type', 'serial'),
        description='通信方式: "serial"（デフォルト）/ "wifi" / "bluetooth"'
    )

    tcp_host_arg = DeclareLaunchArgument(
        'tcp_host',
        default_value=launch_defaults.get('tcp_host', '192.168.1.100'),
        description='WiFiモード時の接続先IPアドレス'
    )
    tcp_port_arg = DeclareLaunchArgument(
        'tcp_port',
        default_value=str(launch_defaults.get('tcp_port', '8080')),
        description='WiFiモード時の接続先ポート番号'
    )

    bt_address_arg = DeclareLaunchArgument(
        'bt_address',
        default_value=launch_defaults.get('bt_address', ''),
        description='Bluetoothモード時の接続先MACアドレス'
    )
    bt_channel_arg = DeclareLaunchArgument(
        'bt_channel',
        default_value=str(launch_defaults.get('bt_channel', 1)),
        description='Bluetoothモード時のSPPチャンネル番号'
    )

    xacro_file = os.path.join(pkg_share, 'urdf', 'my_cugo_robot.urdf.xacro')
    doc = xacro.process_file(xacro_file)
    robot_description_config = {'robot_description': doc.toxml()}

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description_config]
    )

    def launch_setup(context, *args, **kwargs):
        comm_type = LaunchConfiguration('comm_type').perform(context)
        tcp_host = LaunchConfiguration('tcp_host').perform(context)
        tcp_port = LaunchConfiguration('tcp_port').perform(context)
        bt_address = LaunchConfiguration('bt_address').perform(context)
        bt_channel = LaunchConfiguration('bt_channel').perform(context)

        # 通信モードに応じて serial_port を上書き
        if comm_type == 'wifi':
            serial_port_override = {'serial_port': VSERIAL_PATH}
        elif comm_type == 'bluetooth':
            serial_port_override = {'serial_port': BT_RFCOMM_PATH}
        else:
            serial_port_override = {}

        cugo_node = Node(
            package='cugo_v4_5_ros2_control',
            executable='cugo_v4_5_ros2_control',
            name='cugo_v4_5_ros2_control',
            output='screen',
            parameters=[params_file, serial_port_override],
            emulate_tty=True,
            arguments=['--ros-args', '--log-level', LaunchConfiguration('log_level')]
        )

        if comm_type == 'wifi':
            # socatで仮想シリアルポートを生成し、TCP接続にブリッジ
            # PTYの生成を待つため、cugo_nodeはWIFI_NODE_DELAY_SEC秒遅延起動
            socat_process = ExecuteProcess(
                cmd=[
                    'socat',
                    f'PTY,link={VSERIAL_PATH},raw,echo=0',
                    f'TCP:{tcp_host}:{tcp_port},nodelay'
                ],
                output='screen',
                name='socat_wifi_bridge'
            )
            return [
                socat_process,
                TimerAction(period=WIFI_NODE_DELAY_SEC, actions=[cugo_node])
            ]
        elif comm_type == 'bluetooth':
            # rfcomm bind で /dev/rfcomm0 を作成し、シリアルポートとして使用
            # デバイス作成を待つため、cugo_nodeはBT_NODE_DELAY_SEC秒遅延起動
            # 事前に: sudo usermod -aG bluetooth $USER でグループ追加が必要
            rfcomm_process = ExecuteProcess(
                cmd=[
                    'sudo', 'rfcomm', 'bind', BT_RFCOMM_PATH, bt_address, bt_channel
                ],
                output='screen',
                name='rfcomm_bt_bind'
            )
            return [
                rfcomm_process,
                TimerAction(period=BT_NODE_DELAY_SEC, actions=[cugo_node])
            ]
        else:
            return [cugo_node]

    return LaunchDescription([
        log_level_arg,
        comm_type_arg,
        tcp_host_arg,
        tcp_port_arg,
        bt_address_arg,
        bt_channel_arg,
        robot_state_publisher_node,
        OpaqueFunction(function=launch_setup)
    ])
