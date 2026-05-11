import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('cugo_v4_5_ros2_control')

    # URDFを変更する場合は、以下のパスを差し替えてください
    urdf_file = os.path.join(pkg_share, 'urdf', 'cugo_v4_5_urdf.urdf')
    with open(urdf_file, 'r') as f:
        robot_description_content = f.read()
    robot_description = {'robot_description': robot_description_content}

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    joint_state_publisher_gui = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        output='screen',
    )

    rviz2 = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
    )

    return LaunchDescription([
        robot_state_publisher,
        joint_state_publisher_gui,
        rviz2,
    ])
