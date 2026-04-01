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

#include "cugo_v4_5_ros2_control/node.hpp"

#include <iomanip>
#include <sstream>

using namespace cugo_v4_5_ros2_control;
Node::Node()
: rclcpp::Node("cugo_v4_5_ros2_control")
{
  // 設定用の一時変数定義
  double control_frequency;
  int pid, rid;
  // std::array は宣言時に初期化
  std::array<double, 36> pose_cov = {0};
  std::array<double, 36> twist_cov = {0};
  double px, py, pz, pr, pp, pyaw;
  double tx, ty, tyaw;

  RCLCPP_INFO(this->get_logger(), "cugo_v4_5_ros2_control has started.");

  // launchファイルからパラメータを取得
  this->declare_parameter("odom_frame_id", "odom");
  this->declare_parameter("base_link_frame_id", "base_link");
  this->declare_parameter("subscribe_topic_name", "/cmd_vel");
  this->declare_parameter("publish_topic_name", "/odom");
  this->declare_parameter("control_frequency", 10.0);
  this->declare_parameter("comm_type", "serial");
  this->declare_parameter("tcp_host", "192.168.1.100");
  this->declare_parameter("tcp_port", 8080);
  this->declare_parameter("serial_port", "/dev/ttyACM0");
  this->declare_parameter("serial_baudrate", 115200);
  this->declare_parameter("cmd_vel_timeout", 0.5); // 秒
  this->declare_parameter("serial_timeout", 0.5);  // 秒
  this->declare_parameter("max_consecutive_errors", 5);
  this->declare_parameter("response_lost_timeout", 0.0);
  this->declare_parameter("product_id", 10000);
  this->declare_parameter("robot_id", 0);
  // オドメトリ積分方式
  this->declare_parameter("odometry_method", std::string("midpoint"));
  this->declare_parameter("odometry_analytic_angular_z_threshold", M_PI / 1000.0);
  // ログ設定パラメータ (INFOレベル)
  this->declare_parameter("param_info_log", false);
  this->declare_parameter("odom_pos_info_log", false);
  this->declare_parameter("odom_vel_info_log", false);
  this->declare_parameter("recv_interval_info_log", false);
  this->declare_parameter("packet_error_info_log", false);
  this->declare_parameter("connection_info_log", false);
  this->declare_parameter("connection_lost_log", false);
  // ログ設定パラメータ (INFOレベル・詳細系)
  this->declare_parameter("received_speed_log", false);
  this->declare_parameter("cmd_vel_log", false);
  this->declare_parameter("tx_cmd_log", false);
  this->declare_parameter("loop_interval_log", false);
  this->declare_parameter("handshake_log", false);
  this->declare_parameter("serial_log", false);
  this->declare_parameter("serial_raw_log", false);
  this->declare_parameter("callback_log", false);
  this->declare_parameter("rtt_log", false);

  // 共分散
  this->declare_parameter("pose_cov_x", 0.04);
  this->declare_parameter("pose_cov_y", 0.04);
  this->declare_parameter("pose_cov_z", 1e9);
  this->declare_parameter("pose_cov_roll", 1e9);
  this->declare_parameter("pose_cov_pitch", 1e9);
  this->declare_parameter("pose_cov_yaw", 0.01);
  this->declare_parameter("twist_cov_linear_x", 0.0025);
  this->declare_parameter("twist_cov_linear_y", 0.0025);
  this->declare_parameter("twist_cov_angular_z", 1e9);

  // パラメータ取得
  this->get_parameter("odom_frame_id", odom_frame_id_);
  this->get_parameter("base_link_frame_id", base_link_frame_id_);
  this->get_parameter("subscribe_topic_name", subscribe_topic_name);
  this->get_parameter("publish_topic_name", publish_topic_name);
  this->get_parameter("control_frequency", control_frequency);
  this->get_parameter("serial_port", serial_port);
  this->get_parameter("serial_baudrate", serial_baudrate);
  this->get_parameter("cmd_vel_timeout", cmd_vel_timeout_);
  this->get_parameter("serial_timeout", serial_timeout_);
  int max_consecutive_errors_int;
  this->get_parameter("max_consecutive_errors", max_consecutive_errors_int);
  max_consecutive_errors_ = static_cast<uint32_t>(std::max(1, max_consecutive_errors_int));
  this->get_parameter("response_lost_timeout", response_lost_timeout_);
  if (response_lost_timeout_ > 0.0 && response_lost_timeout_ >= serial_timeout_) {
    RCLCPP_WARN(
      this->get_logger(),
      "response_lost_timeout (%.2f) >= serial_timeout (%.2f). response_lost_timeout disabled.",
      response_lost_timeout_, serial_timeout_);
    response_lost_timeout_ = 0.0;
  }
  this->get_parameter("product_id", pid);
  this->get_parameter("robot_id", rid);
  std::string odometry_method_str;
  double odometry_analytic_angular_z_threshold;
  this->get_parameter("odometry_method", odometry_method_str);
  this->get_parameter("odometry_analytic_angular_z_threshold", odometry_analytic_angular_z_threshold);
  this->get_parameter("param_info_log", param_info_log_);
  this->get_parameter("odom_pos_info_log", odom_pos_info_log_);
  this->get_parameter("odom_vel_info_log", odom_vel_info_log_);
  this->get_parameter("recv_interval_info_log", recv_interval_info_log_);
  this->get_parameter("packet_error_info_log", packet_error_info_log_);
  this->get_parameter("connection_info_log", connection_info_log_);
  this->get_parameter("connection_lost_log", connection_lost_log_);
  this->get_parameter("received_speed_log", received_speed_log_);
  this->get_parameter("cmd_vel_log", cmd_vel_log_);
  this->get_parameter("tx_cmd_log", tx_cmd_log_);
  this->get_parameter("loop_interval_log", loop_interval_log_);
  this->get_parameter("handshake_log", handshake_log_);
  this->get_parameter("serial_log", serial_log_);
  this->get_parameter("serial_raw_log", serial_raw_log_);
  this->get_parameter("callback_log", callback_log_);
  this->get_parameter("rtt_log", rtt_log_);

  this->get_parameter("pose_cov_x", px);
  this->get_parameter("pose_cov_y", py);
  this->get_parameter("pose_cov_z", pz);
  this->get_parameter("pose_cov_roll", pr);
  this->get_parameter("pose_cov_pitch", pp);
  this->get_parameter("pose_cov_yaw", pyaw);
  this->get_parameter("twist_cov_linear_x", tx);
  this->get_parameter("twist_cov_linear_y", ty);
  this->get_parameter("twist_cov_angular_z", tyaw);

  if (param_info_log_) {
    RCLCPP_INFO(this->get_logger(), "設定パラメータ");
    RCLCPP_INFO(this->get_logger(), "  odom_frame_id           : %s", odom_frame_id_.c_str());
    RCLCPP_INFO(this->get_logger(), "  base_link_frame_id      : %s", base_link_frame_id_.c_str());
    RCLCPP_INFO(this->get_logger(), "  subscribe_topic_name    : %s", subscribe_topic_name.c_str());
    RCLCPP_INFO(this->get_logger(), "  publish_topic_name      : %s", publish_topic_name.c_str());
    RCLCPP_INFO(this->get_logger(), "  control_frequency       : %f", control_frequency);
    RCLCPP_INFO(this->get_logger(), "  serial_port             : %s", serial_port.c_str());
    RCLCPP_INFO(this->get_logger(), "  serial_baudrate         : %d", serial_baudrate);
    RCLCPP_INFO(this->get_logger(), "  cmd_vel_timeout         : %f", cmd_vel_timeout_);
    RCLCPP_INFO(this->get_logger(), "  serial_timeout          : %f", serial_timeout_);
    RCLCPP_INFO(this->get_logger(), "  max_consecutive_errors  : %u", max_consecutive_errors_);
    RCLCPP_INFO(this->get_logger(), "  response_lost_timeout   : %f", response_lost_timeout_);
    RCLCPP_INFO(this->get_logger(), "  product_id              : %d", pid);
    RCLCPP_INFO(this->get_logger(), "  robot_id                : %d", rid);
    RCLCPP_INFO(this->get_logger(), "  odometry_method         : %s", odometry_method_str.c_str());
    RCLCPP_INFO(this->get_logger(), "  analytic_angular_z_thr  : %f", odometry_analytic_angular_z_threshold);
  }

  // 本クラスで実装された通信がサポートされている製品かどうかを、product_idで確認
  // 範囲外の場合はエラーで終了
  if((pid < SUPPORTED_PRODUCT_ID_MIN) || (pid > SUPPORTED_PRODUCT_ID_MAX)) {
    RCLCPP_FATAL(this->get_logger(),
      "product_id %d is not supported by this software. Supported range is 10000-19999. Shutting down.",
      pid);
    rclcpp::shutdown();
    return;
  }

  // 各クラスの初期化
  cugo_ = std::make_unique<CuGo>();
  // Serialの初期化 (デリミタ0x00を指定)
  serial_ = std::make_shared<Serial>(0x00);

  // CuGoセットアップ
  pose_cov[0] = px;
  pose_cov[7] = py;
  pose_cov[14] = pz;
  pose_cov[21] = pr;
  pose_cov[28] = pp;
  pose_cov[35] = pyaw;
  twist_cov[0] = tx;
  twist_cov[7] = ty;
  twist_cov[35] = tyaw;

  cugo_->set_identity(static_cast<uint16_t>(pid), static_cast<uint16_t>(rid));
  cugo_->set_covariance(pose_cov, twist_cov);

  OdometryMethod odometry_method;
  if (odometry_method_str == "analytic") {
    odometry_method = OdometryMethod::ANALYTIC;
  } else {
    if (odometry_method_str != "midpoint") {
      RCLCPP_WARN(
        this->get_logger(),
        "Unknown odometry_method \"%s\". Falling back to \"midpoint\".",
        odometry_method_str.c_str());
    }
    odometry_method = OdometryMethod::MIDPOINT;
  }
  cugo_->set_odometry_config(odometry_method, odometry_analytic_angular_z_threshold);

  // Serial通信の開始
  try {
    serial_->open(serial_port, serial_baudrate);

    serial_->register_callback(std::bind(&Node::serial_data_callback, this, std::placeholders::_1));
  } catch (const std::exception & e) {
    RCLCPP_FATAL(
        this->get_logger(), "Failed to setup serial communication: %s. Shutting down.", e.what());
    rclcpp::shutdown();
    return;
  }

  // 状態変数の初期化
  auto now = this->get_clock()->now();
  last_cmd_vel_time_ = now;
  last_serial_receive_time_ = now;
  handshake_last_action_time_ = now;
  last_control_loop_time_ = now;
  last_tx_time_ = now;

  // ROS トピック通信
  cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      subscribe_topic_name.c_str(), 1,
      std::bind(&Node::cmd_vel_callback, this, std::placeholders::_1));
  odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>(publish_topic_name.c_str(), 10);
  handshake_pub_ = this->create_publisher<std_msgs::msg::Bool>("handshake_status", 10);

  tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

  // ループ処理の開始
  control_timer = this->create_wall_timer(
      std::chrono::milliseconds(static_cast<int>(1000.0 / control_frequency)),
      std::bind(&Node::control_loop, this));
}

void Node::cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
  // 先にタイムスタンプを取得
  rclcpp::Time reception_time = this->get_clock()->now();

  if (cmd_vel_log_) {
    RCLCPP_INFO(
        this->get_logger(), "[cmd_vel] Vx=%lf, Vy=%lf, Omega z=%lf",
        msg->linear.x, msg->linear.y, msg->angular.z);
  }

  // lock_guardでmutexをロックし、スコープを抜けたら自動でアンロック
  std::lock_guard<std::mutex> lock(data_mutex_);

  // 共有データを更新
  latest_cmd_vel_ = *msg;
  last_cmd_vel_time_ = reception_time;
}

// シリアルデータ受信時のコールバック (Serialクラスから呼ばれる)
void Node::serial_data_callback(const std::vector<unsigned char> & raw_packet)
{
  if (callback_log_) {
    RCLCPP_INFO(this->get_logger(), "serial_data_callback()");
  }

  if (serial_raw_log_) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : raw_packet) {
      oss << " " << std::setw(2) << static_cast<int>(byte);
    }
    RCLCPP_INFO(this->get_logger(), "[RX raw] %zu bytes:%s", raw_packet.size(), oss.str().c_str());
  }
  if (serial_log_) {
    std::vector<uint8_t> decoded_log = CugoProtocol::decode_cobs(raw_packet);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : decoded_log) {
      oss << " " << std::setw(2) << static_cast<int>(byte);
    }
    RCLCPP_INFO(this->get_logger(), "[RX] %zu bytes:%s", decoded_log.size(), oss.str().c_str());
  }
  rclcpp::Time current_receive_time = this->get_clock()->now();

  // パブリッシュを実施するかどうかのフラグ
  bool should_publish = false;

  // ---  ハンドシェイク確認 (cugo_へのアクセスを含むためロック) ---
  bool is_hs_done_local = false;

  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    is_hs_done_local = is_handshake_done_;

    if (!is_hs_done_local) {
      // --- ハンドシェイク未完了時の処理 ---
      if (cugo_->validate_handshake_response(raw_packet)) {
        if (connection_info_log_) {
          RCLCPP_INFO(this->get_logger(), "[connection] Handshake Successful! State: -> CONNECTED");
        }
        is_handshake_done_ = true;

        connection_state_ = ConnectionState::CONNECTED;
        handshake_state_ = HandshakeState::COMPLETE;
        last_serial_receive_time_ = current_receive_time;
        last_cmd_vel_time_ = current_receive_time;  // cmd_velタイムアウトをリセット
        is_first_serial_data_ = false;

        // ハンドシェイク完了を通知するためにPublishする(位置・速度は変化なし)
        should_publish = true;
      }
    }
  }

  // ハンドシェイク未完了、かつ今回も成功しなかった場合はここで終了
  if (!is_hs_done_local && !should_publish) {
    return;
  }
  // ハンドシェイク成功直後の場合はPublishへ飛ぶ
  if (should_publish) {
    publish_odom_and_tf();
    return;
  }

  // --- 通常通信時の処理 ---
  // 1. プロトコルデコード (deserialize)
  RobotState state;
  std::string error_msg;
  bool success = CugoProtocol::deserialize(raw_packet, state, error_msg);

  if (!success) {
    packet_error_count_++;
    consecutive_error_count_++;
    RCLCPP_WARN_THROTTLE(
        this->get_logger(), *this->get_clock(), 1000,
        "Packet Decode Error: %s", error_msg.c_str());
    if (packet_error_info_log_) {
      RCLCPP_INFO_THROTTLE(
          this->get_logger(), *this->get_clock(), 1000,
          "[packet error] cumulative count: %u", packet_error_count_.load());
    }
    if (consecutive_error_count_ >= max_consecutive_errors_) {
      serial_->flush_buffer();
      consecutive_error_count_ = 0;
      RCLCPP_WARN(
          this->get_logger(),
          "[packet error] %u consecutive errors detected. Buffer flushed for resync.",
          max_consecutive_errors_);
    }
    return;
  } else {
    consecutive_error_count_ = 0;
    if (received_speed_log_) {
      RCLCPP_INFO(
          this->get_logger(), "[RX speed] Vx=%lf, Vy=%lf, Omega z=%lf",
          state.linear_x, state.linear_y, state.angular_z);
    }
  }

  // 2. 状態を更新(Mutexで保護)
  double dt_for_log = 0.0;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);

    // 応答受信フラグをリセット (response_lost_timeout 機能用)
    waiting_for_response_ = false;

    // 最初のデータ受信時は、前回値として保存するだけ
    if (is_first_serial_data_) {
      last_serial_receive_time_ = current_receive_time;
      is_first_serial_data_ = false;
    } else {
      // 2回目以降の受信
      double dt = (current_receive_time - last_serial_receive_time_).seconds();
      if (dt <= 0.0) { // 時間が進んでいない場合は計算しない
        RCLCPP_WARN(this->get_logger(), "dt is zero or negative. Skipping odometry calculation.");
        return;
      } else {

        // ★ 状態更新はCuGoに依頼 (ロック内で行う)
        cugo_->update_state(state, dt);

        // 受信時間の保存
        last_serial_receive_time_ = current_receive_time;
        should_publish = true;
        dt_for_log = dt;
      }
    }
  } // Mutex unlock

  if (recv_interval_info_log_ && dt_for_log > 0.0) {
    RCLCPP_INFO(this->get_logger(), "[recv interval] %.1f ms", dt_for_log * 1000.0);
  }
  if (rtt_log_) {
    double rtt_ms = (current_receive_time - last_tx_time_).seconds() * 1000.0;
    RCLCPP_INFO(this->get_logger(), "[RTT] %.1f ms", rtt_ms);
  }


  // 3. 計算したオドメトリとTFを発行
  // publish_odom_and_tf内でもロックを取得するため、ここでは一旦ロックを開放してから呼ぶ
  // (cugo_の状態はupdate_stateで更新済み)
  if (should_publish) {
    publish_odom_and_tf();
    if (callback_log_) {
      RCLCPP_INFO(this->get_logger(), "serial_data_callback() published");
    }
  }
}

void Node::control_loop()
{
  if (callback_log_) {
    RCLCPP_INFO(this->get_logger(), "control_loop()");
  }
  auto now = this->get_clock()->now();

  if (loop_interval_log_) {
    double interval_ms = (now - last_control_loop_time_).seconds() * 1000.0;
    RCLCPP_INFO(this->get_logger(), "[loop interval] %.1f ms", interval_ms);
  }
  last_control_loop_time_ = now;

  // フラグのローカルコピーを取得
  bool is_hs_done_local;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    is_hs_done_local = is_handshake_done_;
  }

  // ステータスPublish (スレッドセーフ)
  std_msgs::msg::Bool status_msg;
  status_msg.data = is_hs_done_local;
  handshake_pub_->publish(status_msg);

  // --- 再接続処理 (ハンドシェイク前に実行し、接続状態に関わらず到達できるようにする) ---
  if (connection_state_ == ConnectionState::RECONNECTING) {
    // 3秒に一回だけ再接続を試みる
    if ((now - last_reconnect_attempt_time_).seconds() > 3.0) {
      if (connection_info_log_) {
        RCLCPP_INFO(this->get_logger(), "[connection] Trying to reconnect...");
      }
      bool reconnect_success = false;
      try {
        if (serial_->reconnect(serial_port, serial_baudrate)) {
          reconnect_success = true;
        }
      } catch (const std::exception & e) {
        RCLCPP_WARN(this->get_logger(), "Reconnect failed: %s", e.what());
      }

      if (reconnect_success) {
        if (connection_info_log_) {
          RCLCPP_INFO(this->get_logger(), "[connection] Reconnect successful! State: RECONNECTING -> CONNECTED");
        }
        connection_state_ = ConnectionState::CONNECTED;

        std::lock_guard<std::mutex> lock(data_mutex_);
        is_handshake_done_ = false;
        handshake_state_ = HandshakeState::INIT;
        is_first_serial_data_ = true;
        last_serial_receive_time_ = now;
        last_cmd_vel_time_ = now;  // cmd_velタイムアウトをリセット
        latest_cmd_vel_ = geometry_msgs::msg::Twist();  // 再接続後は速度ゼロから開始
        waiting_for_response_ = false;  // response_lost_timeout 機能をリセット
      }
      last_reconnect_attempt_time_ = now;
    }

    // 再接続待機中はゼロ速度オドメトリを発行して終了
    // (再接続成功時は connection_state_ が CONNECTED に戻るためこのブロックに入らない)
    if (connection_state_ == ConnectionState::RECONNECTING) {
      if (connection_lost_log_) {
        RCLCPP_WARN(this->get_logger(), "シリアル通信未達。接続を確認してください。");
      }
      Pose2D current_pose;
      {
        std::lock_guard<std::mutex> lock(data_mutex_);
        current_pose = cugo_->get_pose();
      }

      geometry_msgs::msg::TransformStamped t;
      t.header.stamp = now;
      t.header.frame_id = odom_frame_id_;
      t.child_frame_id = base_link_frame_id_;
      t.transform.translation.x = current_pose.x;
      t.transform.translation.y = current_pose.y;
      t.transform.translation.z = 0.0;

      tf2::Quaternion q;
      q.setRPY(0, 0, current_pose.yaw);
      t.transform.rotation = tf2::toMsg(q);
      tf_broadcaster_->sendTransform(t);

      nav_msgs::msg::Odometry lost_odom;
      lost_odom.header.stamp = now;
      lost_odom.header.frame_id = odom_frame_id_;
      lost_odom.child_frame_id = base_link_frame_id_;
      lost_odom.pose.pose.position.x = current_pose.x;
      lost_odom.pose.pose.position.y = current_pose.y;
      lost_odom.pose.pose.orientation = t.transform.rotation;
      lost_odom.twist.twist.linear.x = 0.0;
      lost_odom.twist.twist.linear.y = 0.0;
      lost_odom.twist.twist.angular.z = 0.0;
      lost_odom.pose.covariance.fill(1e9);
      lost_odom.twist.covariance.fill(1e9);
      odom_pub_->publish(lost_odom);

      if (callback_log_) {
        RCLCPP_INFO(this->get_logger(), "control_loop() zero /cmd_vel published");
      }
      return;
    }
  }

  // --- ハンドシェイク処理 ---
  if (!is_hs_done_local) {
    std::lock_guard<std::mutex> lock(data_mutex_);

    switch (handshake_state_) {
      case HandshakeState::INIT:
        if (handshake_log_) {
          RCLCPP_INFO(this->get_logger(), "[handshake] INIT -> SENDING");
        }
        handshake_state_ = HandshakeState::SENDING;
        // fallthrough

      case HandshakeState::SENDING:
        {
        // ハンドシェイク専用パケットの生成 (CuGo -> Protocol)
          std::vector<uint8_t> handshake_packet = cugo_->create_handshake_packet();
          if (!handshake_packet.empty()) {
            if (serial_raw_log_) {
              std::ostringstream oss;
              oss << std::hex << std::setfill('0');
              for (auto byte : handshake_packet) {
                oss << " " << std::setw(2) << static_cast<int>(byte);
              }
              RCLCPP_INFO(
                this->get_logger(), "[TX raw] %zu bytes:%s",
                handshake_packet.size(), oss.str().c_str());
            }
            if (serial_log_) {
              std::vector<uint8_t> raw_log = CugoProtocol::decode_cobs(handshake_packet);
              std::ostringstream oss;
              oss << std::hex << std::setfill('0');
              for (auto byte : raw_log) {
                oss << " " << std::setw(2) << static_cast<int>(byte);
              }
              RCLCPP_INFO(
                this->get_logger(), "[TX] %zu bytes:%s",
                raw_log.size(), oss.str().c_str());
            }
            serial_->write(handshake_packet);
            handshake_last_action_time_ = now;
            handshake_state_ = HandshakeState::WAITING_ACK;
            if (handshake_log_) {
              RCLCPP_INFO(this->get_logger(), "[handshake] SENDING -> WAITING_ACK (request sent)");
            }
          }
          break;
        }

      case HandshakeState::WAITING_ACK:
        if ((now - handshake_last_action_time_).seconds() > handshake_timeout_) {
          RCLCPP_WARN(this->get_logger(), "Handshake Timeout. Retrying later...");
          if (handshake_log_) {
            RCLCPP_INFO(this->get_logger(), "[handshake] WAITING_ACK -> FAILED_WAIT (timeout)");
          }
          handshake_state_ = HandshakeState::FAILED_WAIT;
          handshake_last_action_time_ = now;
        }
        break;

      case HandshakeState::FAILED_WAIT:
        if ((now - handshake_last_action_time_).seconds() > handshake_retry_interval_) {
          if (handshake_log_) {
            RCLCPP_INFO(this->get_logger(), "[handshake] FAILED_WAIT -> INIT (retry)");
          }
          handshake_state_ = HandshakeState::INIT;
        }
        break;

      case HandshakeState::COMPLETE:
        // callback側でフラグが立ったらここに来る
        break;
    }
    // ハンドシェイク中は他の送信を行わない
    return;
  }

  // --- 通常制御ループ ---
  geometry_msgs::msg::Twist local_cmd_vel;
  rclcpp::Time local_last_cmd_vel_time;
  rclcpp::Time local_last_serial_receive_time;

  bool local_waiting_for_response = false;

  // cmd_velにアクセスしてすぐにロック解除
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    local_cmd_vel = latest_cmd_vel_;
    local_last_cmd_vel_time = last_cmd_vel_time_;
    local_last_serial_receive_time = last_serial_receive_time_;
    local_waiting_for_response = waiting_for_response_;
  }

  double vx = local_cmd_vel.linear.x;
  double vy = local_cmd_vel.linear.y;
  double wz = local_cmd_vel.angular.z;

  if ((now - local_last_cmd_vel_time).seconds() > cmd_vel_timeout_) {
    vx = 0.0;
    vy = 0.0;
    wz = 0.0;
  }

  if (tx_cmd_log_) {
    RCLCPP_INFO(
        this->get_logger(), "[TX cmd] Vx=%lf, Vy=%lf, Omega z=%lf", vx, vy, wz);
  }

  // create_command_packetはconstメソッド相当だが、安全のためcugoアクセスとしてロック推奨
  // ただし頻度が低いパラメータ(ID)参照のみならロックなしでも動くが、設計の一貫性のためにロック
  std::vector<uint8_t> packet;
  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    packet = cugo_->create_command_packet(vx, vy, wz);
  }

  // --- response_lost_timeout チェック ---
  bool skip_tx = false;
  if (response_lost_timeout_ > 0.0 && local_waiting_for_response) {
    double elapsed = (now - last_tx_time_).seconds();
    if (elapsed < response_lost_timeout_) {
      skip_tx = true;  // 応答待ち中のためTXをスキップ
    } else {
      // タイムアウト → response lost: フラグリセットして次のTXを許可
      RCLCPP_WARN(
        this->get_logger(),
        "[response lost] No response received within %.2f s. Sending next request.",
        response_lost_timeout_);
      std::lock_guard<std::mutex> lock(data_mutex_);
      waiting_for_response_ = false;
    }
  }

    // 3. 送信
  if (packet.empty()) {
    RCLCPP_ERROR(this->get_logger(), "Failed to serialize command packet.");
  } else if (!skip_tx) {
    if (serial_raw_log_) {
      std::ostringstream oss;
      oss << std::hex << std::setfill('0');
      for (auto byte : packet) {
        oss << " " << std::setw(2) << static_cast<int>(byte);
      }
      RCLCPP_INFO(
        this->get_logger(), "[TX raw] %zu bytes:%s", packet.size(), oss.str().c_str());
    }
    if (serial_log_) {
      std::vector<uint8_t> raw_log = CugoProtocol::decode_cobs(packet);
      std::ostringstream oss;
      oss << std::hex << std::setfill('0');
      for (auto byte : raw_log) {
        oss << " " << std::setw(2) << static_cast<int>(byte);
      }
      RCLCPP_INFO(
        this->get_logger(), "[TX] %zu bytes:%s", raw_log.size(), oss.str().c_str());
    }
    serial_->write(packet);
    last_tx_time_ = now;  // RTTログ計測・response_lost_timeout 機能共通
    // 応答待ち状態に移行 (response_lost_timeout が有効な場合のみ)
    if (response_lost_timeout_ > 0.0) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      waiting_for_response_ = true;
    }
  }

  // --- シリアルタイムアウト監視 ---
  bool is_serial_timeout = (now - local_last_serial_receive_time).seconds() > serial_timeout_;

  if (connection_state_ == ConnectionState::CONNECTED) {
    if (is_serial_timeout) {
      RCLCPP_WARN(
          this->get_logger(), "Serial connection timeout detected. Entering reconnect mode...");
      if (connection_info_log_) {
        RCLCPP_INFO(this->get_logger(), "[connection] State: CONNECTED -> RECONNECTING");
      }
      connection_state_ = ConnectionState::RECONNECTING;
      last_reconnect_attempt_time_ = now;

      std::lock_guard<std::mutex> lock(data_mutex_);
      is_handshake_done_ = false;
      handshake_state_ = HandshakeState::INIT;
    }
  }
}

// オドメトリとTFを発行するヘルパー関数
void Node::publish_odom_and_tf()
{
  rclcpp::Time now = this->get_clock()->now();

  Pose2D pose;
  RobotState state;
  std::array<double, 36> pose_cov;
  std::array<double, 36> twist_cov;

  {
    std::lock_guard<std::mutex> lock(data_mutex_);

    // ★ 現在の状態をCuGoから取得
    pose = cugo_->get_pose();
    state = cugo_->get_state();

    // ★ 共分散をCuGoから復元 (通信断時に上書きされた値を正常値に戻す)
    pose_cov = cugo_->get_pose_covariance();
    twist_cov = cugo_->get_twist_covariance();
  } // Unlock

  geometry_msgs::msg::TransformStamped t;
  t.header.stamp = now;
  t.header.frame_id = odom_frame_id_;
  t.child_frame_id = base_link_frame_id_;

  // OdometryメッセージのPose情報をそのまま使う
  t.transform.translation.x = pose.x;
  t.transform.translation.y = pose.y;
  t.transform.translation.z = 0.0;

  tf2::Quaternion q;
  q.setRPY(0, 0, pose.yaw);
  t.transform.rotation = tf2::toMsg(q);
  tf_broadcaster_->sendTransform(t);

  nav_msgs::msg::Odometry odom;
  odom.header.stamp = now;
  odom.header.frame_id = odom_frame_id_;
  odom.child_frame_id = base_link_frame_id_;

  odom.pose.pose.position.x = pose.x;
  odom.pose.pose.position.y = pose.y;
  odom.pose.pose.orientation = t.transform.rotation;
  odom.pose.covariance = pose_cov;

  odom.twist.twist.linear.x = state.linear_x;
  odom.twist.twist.linear.y = state.linear_y;
  odom.twist.twist.angular.z = state.angular_z;
  odom.twist.covariance = twist_cov;

  odom_pub_->publish(odom);

  if (odom_pos_info_log_) {
    RCLCPP_INFO(
        this->get_logger(), "[odom pos] X=%lf, Y=%lf, Yaw=%lf",
        pose.x, pose.y, pose.yaw);
  }
  if (odom_vel_info_log_) {
    RCLCPP_INFO(
        this->get_logger(), "[odom vel] Vx=%lf, Vy=%lf, Omega z=%lf",
        state.linear_x, state.linear_y, state.angular_z);
  }
}
