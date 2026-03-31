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

#ifndef CUGO_V4_5_ROS2_CONTROL_CUGO_HPP
#define CUGO_V4_5_ROS2_CONTROL_CUGO_HPP
#include "cugo_v4_5_ros2_control/cugo_protocol.hpp"
#include <cmath>
#include <array>
#include <string>
#include <vector>

namespace cugo_v4_5_ros2_control
{

enum class OdometryMethod
{
  MIDPOINT,  // 修正オイラー法 (中点法)
  ANALYTIC,  // 解析解 (弧積分)
};

class CuGo
{
public:
  CuGo();

  // --- 設定系 ---
  // ロボットID設定
  void set_identity(uint16_t product_id, uint16_t robot_id);
  // 共分散設定
  void set_covariance(
    const std::array<double, 36> & pose_cov,
    const std::array<double, 36> & twist_cov);
  // オドメトリ積分方式設定
  void set_odometry_config(OdometryMethod method, double angular_z_threshold);

  // --- ハンドシェイク・判定系 ---
  /**
   * @brief ハンドシェイク用パケットを作成
   */
  std::vector<uint8_t> create_handshake_packet();

  /**
   * @brief 受信パケットがハンドシェイク応答として正しいか検証
   */
  bool validate_handshake_response(const std::vector<uint8_t> & packet);

  /**
   * @brief 受信データのIDが自身と一致するか確認する (通常パケット用)
   */
  bool match_identity(const RobotState & state) const;


  // --- 更新・計算系 ---
  // 状態更新 (物理計算)
  void update_state(const RobotState & state, double dt);

  // 指令生成 (Protocolを使ってバイト列を作成)
  std::vector<uint8_t> create_command_packet(double linear_x, double linear_y, double angular_z);

  // --- ゲッター ---
  Pose2D get_pose() const;
  RobotState get_state() const;
  const std::array<double, 36> & get_pose_covariance() const;
  const std::array<double, 36> & get_twist_covariance() const;

private:
  // ロボットID
  uint16_t product_id_{0};
  uint16_t robot_id_{0};

  // 内部状態
  Pose2D current_pose_;
  RobotState current_state_;

  // パラメータ
  std::array<double, 36> pose_covariance_;
  std::array<double, 36> twist_covariance_;

  // オドメトリ積分設定
  OdometryMethod odometry_method_{OdometryMethod::MIDPOINT};
  double angular_z_threshold_{M_PI / 1000.0};
};

}  // namespace cugo_v4_5_ros2_control
#endif
