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

#ifndef CUGO_ROS2_CONTROL2_CUGO_HPP
#define CUGO_ROS2_CONTROL2_CUGO_HPP
#include "cugo_ros2_control2/cugo_protocol.hpp"
#include <math.h>

namespace cugo_ros2_control2
{

class CuGo
{
public:
  CuGo();

  /**
   * @brief オドメトリ計算
   * @param current_pose 現在の位置姿勢
   * @param state ロボットからのフィードバック（速度情報）
   * @param dt 経過時間 [s]
   * @return 更新後の位置姿勢
   */
  Pose2D calc_odom(const Pose2D& current_pose, const RobotState& state, double dt);
};

}  // namespace cugo_ros2_control2
#endif  // CUGO_ROS2_CONTROL2_CUGO_HPP