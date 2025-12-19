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

#include "cugo_ros2_control2/cugo.hpp"

namespace cugo_ros2_control2
{

CuGo::CuGo()
{
  current_pose_ = {0.0, 0.0, 0.0};
  current_state_ = {0, 0, 0.0, 0.0, 0.0};
  pose_covariance_.fill(0.0);
  twist_covariance_.fill(0.0);
}

void CuGo::set_identity(uint16_t product_id, uint16_t robot_id)
{
  product_id_ = product_id;
  robot_id_ = robot_id;
}

void CuGo::set_covariance(const std::array<double, 36>& pose_cov, const std::array<double, 36>& twist_cov)
{
  pose_covariance_ = pose_cov;
  twist_covariance_ = twist_cov;
}

// ハンドシェイクパケット作成 (RobotStateを使用)
std::vector<uint8_t> CuGo::create_handshake_packet()
{
  // 送信用にRobotStateを作成（IDのみセット、速度は0）
  RobotState data;
  data.product_id = product_id_;
  data.robot_id = robot_id_;
  data.linear_x = 0.0;
  data.linear_y = 0.0;
  data.angular_z = 0.0;

  return CugoProtocol::serialize_handshake(data);
}

// ハンドシェイク応答検証 (RobotStateを使用)
bool CuGo::validate_handshake_response(const std::vector<uint8_t>& packet)
{
  RobotState data;
  RobotState expected_state;
  expected_state.product_id = product_id_;
  expected_state.robot_id = robot_id_;
  // 専用のデシリアライズを使用
  bool protocol_compatible =  CugoProtocol::deserialize_handshake(packet, expected_state, data);

  if(protocol_compatible){
    if(product_id_ != data.product_id){
      // プロダクトIDが異なるが、プロトコルは互換性があるため通信可能な場合
      std::cout << "Product ID Mismatch in Handshake Response. Expected: " << get_state().product_id
                << ", Received: " << data.product_id << std::endl;
    }

    if(robot_id_ != data.robot_id){
      // ロボットIDが異なるが、プロダクトIDは一致しているので通信可能な場合
      std::cout << "Robot ID Mismatch in Handshake Response. Expected: " << get_state().robot_id
                << ", Received: " << data.robot_id << std::endl;
    }

    return true;
  }
  else{
    
    return false;
  }
}

bool CuGo::match_identity(const RobotState& state) const
{
  return (state.product_id == product_id_) && (state.robot_id == robot_id_);
}

void CuGo::update_state(const RobotState& state, double dt)
{
  // 速度情報の保存
  current_state_ = state;

  // オドメトリ計算 (単純なオイラー積分)
  double next_yaw = current_pose_.yaw + state.angular_z * dt;
  double delta_x = (state.linear_x * std::cos(next_yaw) - state.linear_y * std::sin(next_yaw)) * dt;
  double delta_y = (state.linear_x * std::sin(next_yaw) + state.linear_y * std::cos(next_yaw)) * dt;

  current_pose_.x += delta_x;
  current_pose_.y += delta_y;
  current_pose_.yaw = next_yaw;
}

std::vector<uint8_t> CuGo::create_command_packet(double linear_x, double linear_y, double angular_z)
{
  ControlCommand cmd;
  cmd.product_id = product_id_; // 保存しているIDを使用
  cmd.robot_id = robot_id_;
  cmd.linear_x = linear_x;
  cmd.linear_y = linear_y;
  cmd.angular_z = angular_z;

  // Protocolに処理を委譲
  return CugoProtocol::serialize(cmd);
}

Pose2D CuGo::get_pose() const
{
  return current_pose_;
}

RobotState CuGo::get_state() const
{
  return current_state_;
}

const std::array<double, 36>& CuGo::get_pose_covariance() const
{
  return pose_covariance_;
}

const std::array<double, 36>& CuGo::get_twist_covariance() const
{
  return twist_covariance_;
}

} // namespace cugo_ros2_control2