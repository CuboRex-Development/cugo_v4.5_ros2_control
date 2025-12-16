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

#include <iostream>

using namespace cugo_ros2_control2;

CuGo::CuGo()
{
}



Pose2D CuGo::calc_odom(const Pose2D& current_pose, const RobotState& state, double dt)
{
  Pose2D next_pose;
  next_pose.yaw = current_pose.yaw + state.angular_z * dt;

  // 移動量の計算 (単純なオイラー積分)
  // ロボット座標系での速度(linear_x, linear_y)をワールド座標系へ変換
  double delta_x = (state.linear_x * std::cos(next_pose.yaw) - state.linear_y * std::sin(next_pose.yaw)) * dt;
  double delta_y = (state.linear_x * std::sin(next_pose.yaw) + state.linear_y * std::cos(next_pose.yaw)) * dt;

  // 位置の更新
  next_pose.x = current_pose.x + delta_x;
  next_pose.y = current_pose.y + delta_y;

  return next_pose;
}