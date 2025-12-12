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



Odom CuGo::calc_odom(Odom input_odom, Twist twist, double dt)
{
  Odom output_odom;
  output_odom.yaw = input_odom.yaw + twist.angular_z * dt;

  double delta_x = (twist.linear_x * cos(output_odom.yaw) - twist.linear_y * sin(output_odom.yaw)) * dt;
  double delta_y = (twist.linear_x * sin(output_odom.yaw) + twist.linear_y * cos(output_odom.yaw)) * dt;

  output_odom.x = input_odom.x + delta_x;
  output_odom.y = input_odom.y + delta_y;

  return output_odom;
}