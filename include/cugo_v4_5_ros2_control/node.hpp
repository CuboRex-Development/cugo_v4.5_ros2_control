/*
   Copyright [2025] [CuboRex Co.,Ltd.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef CUGO_V4_5_ROS2_CONTROL_NODE_HPP
#define CUGO_V4_5_ROS2_CONTROL_NODE_HPP

#include <diagnostic_updater/diagnostic_updater.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/bool.hpp>
#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <mutex>
#include <vector>

#include "cugo_v4_5_ros2_control/cugo.hpp"
#include "cugo_v4_5_ros2_control/serial.hpp"
#include "cugo_v4_5_ros2_control/cugo_protocol.hpp"
#include "tf2/LinearMath/Matrix3x3.h"

#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/transform_broadcaster.h"

namespace cugo_v4_5_ros2_control
{

  // 本ノードがサポートするプロダクトIDの範囲
constexpr uint16_t SUPPORTED_PRODUCT_ID_MIN = 10000;
constexpr uint16_t SUPPORTED_PRODUCT_ID_MAX = 19999;

enum class ConnectionState
{
  CONNECTED,
  DISCONNECTED,
  RECONNECTING
};

// ハンドシェイクの状態定義
enum class HandshakeState
{
  INIT,           // 初期化
  SENDING,        // 送信中
  WAITING_ACK,    // 応答待ち
  FAILED_WAIT,    // 失敗・再試行待ち
  COMPLETE        // 完了
};

class Node : public rclcpp::Node
{
public:
  Node();

private:
  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg);
  void serial_data_callback(const std::vector<unsigned char> & raw_packet);
  void control_loop();
  void publish_odom_and_tf();

  // サブスクライバーとパブリッシャー
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr handshake_pub_; // ハンドシェイク状態
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  // インスタンス
  std::unique_ptr<cugo_v4_5_ros2_control::CuGo> cugo_;
  std::shared_ptr<cugo_v4_5_ros2_control::Serial> serial_;

  // データ共有
  ConnectionState connection_state_{ConnectionState::CONNECTED};
  rclcpp::Time last_reconnect_attmpt_time_;
  std::mutex data_mutex_;
  geometry_msgs::msg::Twist latest_cmd_vel_;
  rclcpp::Time last_cmd_vel_time_;
  rclcpp::Time last_serial_receive_time_;

  bool is_first_serial_data_{true};


  // タイマーコールバック
  rclcpp::TimerBase::SharedPtr control_timer;

  // launchファイルのパラメータ
  std::string odom_frame_id_;
  std::string base_link_frame_id_;
  std::string subscribe_topic_name;
  std::string publish_topic_name;

  std::string serial_port;
  int serial_baudrate;
  double cmd_vel_timeout_;
  double serial_timeout_;


  // ハンドシェイクバッファ
  HandshakeState handshake_state_{HandshakeState::INIT};
  rclcpp::Time handshake_last_action_time_;
  bool is_handshake_done_{false};
  double handshake_timeout_{1.0};
  double handshake_retry_interval_{2.0};
};

}  // namespace cugo_v4_5_ros2_control
#endif  // CUGO_V4_5_ROS2_CONTROL_NODE_HPP
