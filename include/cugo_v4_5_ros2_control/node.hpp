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
#include <std_msgs/msg/empty.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <std_msgs/msg/u_int32.hpp>
#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <atomic>
#include <mutex>
#include <vector>

#include "cugo_v4_5_ros2_msgs/msg/controller_status.hpp"
#include "cugo_v4_5_ros2_msgs/msg/controller_error.hpp"
#include "cugo_v4_5_ros2_msgs/msg/motor_driver_error.hpp"
#include "cugo_v4_5_ros2_msgs/msg/headlight_status.hpp"
#include "cugo_v4_5_ros2_msgs/msg/towerlight_status.hpp"

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

  // 追加 I/O サブスクライバ コールバック
  void cmd_mode_callback(const std_msgs::msg::UInt8::SharedPtr msg);
  void cmd_emergency_decel_callback(const std_msgs::msg::Empty::SharedPtr msg);
  void cmd_reset_controller_error_callback(
    const cugo_v4_5_ros2_msgs::msg::ControllerError::SharedPtr msg);
  void cmd_reset_motordriver_error_callback(
    const cugo_v4_5_ros2_msgs::msg::MotorDriverError::SharedPtr msg);
  void cmd_headlight_callback(const cugo_v4_5_ros2_msgs::msg::HeadlightStatus::SharedPtr msg);
  void cmd_towerlight_callback(const cugo_v4_5_ros2_msgs::msg::TowerlightStatus::SharedPtr msg);
  void cmd_bumper_config_callback(const std_msgs::msg::UInt8::SharedPtr msg);
  void cmd_brake_config_callback(const std_msgs::msg::UInt8::SharedPtr msg);

  // サブスクライバーとパブリッシャー
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr handshake_pub_; // ハンドシェイク状態
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  // 追加 I/O サブスクライバ
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr cmd_mode_sub_;
  rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr cmd_emergency_decel_sub_;
  rclcpp::Subscription<cugo_v4_5_ros2_msgs::msg::ControllerError>::SharedPtr
    cmd_reset_controller_error_sub_;
  rclcpp::Subscription<cugo_v4_5_ros2_msgs::msg::MotorDriverError>::SharedPtr
    cmd_reset_motordriver_error_sub_;
  rclcpp::Subscription<cugo_v4_5_ros2_msgs::msg::HeadlightStatus>::SharedPtr cmd_headlight_sub_;
  rclcpp::Subscription<cugo_v4_5_ros2_msgs::msg::TowerlightStatus>::SharedPtr cmd_towerlight_sub_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr cmd_bumper_config_sub_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr cmd_brake_config_sub_;

  // 追加 I/O パブリッシャ
  rclcpp::Publisher<cugo_v4_5_ros2_msgs::msg::ControllerStatus>::SharedPtr
    controller_status_pub_;
  rclcpp::Publisher<cugo_v4_5_ros2_msgs::msg::ControllerError>::SharedPtr
    controller_error_pub_;
  rclcpp::Publisher<cugo_v4_5_ros2_msgs::msg::MotorDriverError>::SharedPtr
    motordriver_error_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr driver_voltage_pub_;
  rclcpp::Publisher<cugo_v4_5_ros2_msgs::msg::HeadlightStatus>::SharedPtr headlight_status_pub_;
  rclcpp::Publisher<cugo_v4_5_ros2_msgs::msg::TowerlightStatus>::SharedPtr towerlight_status_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr io_input_status_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt32>::SharedPtr encoder_motor0_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt32>::SharedPtr encoder_motor1_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt32>::SharedPtr encoder_motor2_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt32>::SharedPtr encoder_motor3_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_temp0_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_temp1_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_temp2_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_temp3_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_error_code0_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_error_code1_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_error_code2_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt16>::SharedPtr motordriver_error_code3_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr bumper_config_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr brake_config_pub_;

  // インスタンス
  std::unique_ptr<cugo_v4_5_ros2_control::CuGo> cugo_;
  std::shared_ptr<cugo_v4_5_ros2_control::Serial> serial_;

  // data_mutex_ で保護するサブスクライバ由来の送信待ちコマンド
  struct PendingCommand
  {
    uint8_t enable_bits             = 0;  // enable_7_14 に OR するビット
    uint8_t mode_switch             = 0;
    uint8_t emergency_decel         = 0;
    uint8_t reset_controller_error  = 0;
    uint8_t reset_motordriver_error = 0;
    uint8_t headlight_control       = 0;
    uint8_t towerlight_control      = 0;
    uint8_t bumper_config           = 0;
    uint8_t brake_config            = 0;
  };

  // データ共有
  std::atomic<ConnectionState> connection_state_{ConnectionState::CONNECTED};
  rclcpp::Time last_reconnect_attempt_time_;
  std::mutex data_mutex_;
  geometry_msgs::msg::Twist latest_cmd_vel_;
  rclcpp::Time last_cmd_vel_time_;
  rclcpp::Time last_serial_receive_time_;
  PendingCommand pending_cmd_;

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
  std::atomic<bool> is_handshake_done_{false};
  double handshake_timeout_{1.0};
  double handshake_retry_interval_{2.0};

  // ログ用フラグ (INFO レベル)
  bool param_info_log_{false};          // 起動時の設定パラメータ一覧
  bool odom_pos_info_log_{false};       // Odometry 位置成分 (x, y, yaw)
  bool odom_vel_info_log_{false};       // Odometry 速度成分 (Vx, Vy, Omega)
  bool recv_interval_info_log_{false};  // データ受信間隔 (ms)
  bool packet_error_info_log_{false};   // パケットデコードエラー統計
  bool connection_info_log_{false};     // 接続状態変化
  bool connection_lost_log_{false};     // シリアル通信未達 WARN

  // ログ用フラグ (INFO レベル・詳細系)
  bool received_speed_log_{false};  // 受信速度 (Vx, Vy, Omega)
  bool cmd_vel_log_{false};         // /cmd_vel で受信した速度指令値
  bool tx_cmd_log_{false};          // 実際に送信した速度指令値
  bool loop_interval_log_{false};   // 制御ループの実行間隔 (ms)
  bool handshake_log_{false};       // ハンドシェイク状態遷移の詳細
  bool serial_log_{false};          // 送受信パケット内容 (COBS後)
  bool serial_raw_log_{false};      // 送受信パケット内容 (生データ)
  bool callback_log_{false};        // コールバック・制御ループの実行フロー
  bool rtt_log_{false};             // リクエスト〜レスポンス往復時間 (ms)

  // ログ統計
  std::atomic<uint32_t> packet_error_count_{0};
  std::atomic<uint32_t> consecutive_error_count_{0};

  // 通信堅牢性設定
  uint32_t max_consecutive_errors_{5};

  // response_lost_timeout 機能
  double response_lost_timeout_{0.0};
  bool waiting_for_response_{false};
  rclcpp::Time last_tx_time_;

  // タイミング計測
  rclcpp::Time last_control_loop_time_;
};

}  // namespace cugo_v4_5_ros2_control
#endif  // CUGO_V4_5_ROS2_CONTROL_NODE_HPP
