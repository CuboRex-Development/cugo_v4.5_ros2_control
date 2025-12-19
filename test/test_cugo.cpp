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

#include <gtest/gtest.h>
#include "cugo_ros2_control2/cugo.hpp"

using namespace cugo_ros2_control2;

// テストフィクスチャ
class CuGoTest : public ::testing::Test
{
protected:
  // パラメータなしコンストラクタを使用
  CuGo cugo; 

  void SetUp() override
  {
    // テスト用にIDを設定
    cugo.set_identity(10000, 1);
  }

  void TearDown() override
  {
    // 必要に応じてクリーンアップ
  }
};

// ハンドシェイク応答検証ロジックのテスト (新規追加)
// 以前の validate_handshake_response の修正が効いているか確認
TEST_F(CuGoTest, test_validate_handshake)
{
  // 1. 期待されるIDを持つパケットを作成
  RobotState expected;
  expected.product_id = 10000;
  expected.robot_id = 1;
  std::vector<uint8_t> valid_packet = CugoProtocol::serialize_handshake(expected);

  // 検証: 成功すべき
  bool result = cugo.validate_handshake_response(valid_packet);
  EXPECT_TRUE(result) << "正しいIDのパケットは承認されるべき";

  // 2. 異なるIDを持つパケットを作成
  RobotState wrong = expected;
  wrong.robot_id = 99;
  std::vector<uint8_t> invalid_packet = CugoProtocol::serialize_handshake(wrong);

  // 検証: 失敗すべき
  result = cugo.validate_handshake_response(invalid_packet);
  EXPECT_FALSE(result) << "誤ったIDのパケットは拒否されるべき";
}

// オドメトリ計算のテスト
// RobotStateの累積が正しいかどうか
TEST_F(CuGoTest, test_update_state_odometry)
{
  // 初期状態確認
  Pose2D pose = cugo.get_pose();
  ASSERT_NEAR(pose.x, 0.0, 1e-4);
  ASSERT_NEAR(pose.y, 0.0, 1e-4);
  ASSERT_NEAR(pose.yaw, 0.0, 1e-4);

  // 1. 直進: 0.5 m/s で 0.1秒
  RobotState state;
  state.linear_x = 0.5;
  state.linear_y = 0.0;
  state.angular_z = 0.0;
  double dt = 0.1;

  cugo.update_state(state, dt);
  pose = cugo.get_pose();

  ASSERT_NEAR(pose.x, 0.05, 1e-4);
  ASSERT_NEAR(pose.y, 0.0, 1e-4);
  ASSERT_NEAR(pose.yaw, 0.0, 1e-4);

  // 2. 旋回: 0.3 m/s, 1.57 rad/s で 0.1秒
  state.linear_x = 0.3;
  state.angular_z = 1.57;
  
  cugo.update_state(state, dt);
  pose = cugo.get_pose();

  // 期待値計算 (オイラー積分: x += v*cos(yaw)*dt, yaw += w*dt)
  // yaw_new = 0.0 + 0.157 = 0.157
  // x_new = 0.05 + 0.3 * cos(0.157) * 0.1 
  // 
  // next_yaw = 0.157
  // dx = 0.3 * cos(0.157) * 0.1 = 0.03 * 0.9876 = 0.0296
  // dy = 0.3 * sin(0.157) * 0.1 = 0.03 * 0.1564 = 0.00469
  
  ASSERT_NEAR(pose.x, 0.05 + (0.3 * std::cos(0.157) * 0.1), 1e-4); 
  ASSERT_NEAR(pose.y, 0.0 + (0.3 * std::sin(0.157) * 0.1), 1e-4);
  ASSERT_NEAR(pose.yaw, 0.157, 1e-4);

  // 2. 旋回: 0.5 m/s, -3.14 rad/s で 0.1秒
  state.linear_x = 0.5;
  state.angular_z = -3.14;
  
  cugo.update_state(state, dt);
  pose = cugo.get_pose();

  // 期待値計算 (オイラー積分: x += v*cos(yaw)*dt, yaw += w*dt)
  // yaw_new = 0.157 + (-0.314) = -0.157
  // x_new = prev_x + 0.5 * cos(-0.157) * 0.1 
  // y_new = prev_y + 0.5 * sin(0.157) * 0.1

  ASSERT_NEAR(pose.x, 0.05 + (0.3 * std::cos(0.157) * 0.1) + (0.5 * std::cos(-0.157) * 0.1), 1e-4); 
  ASSERT_NEAR(pose.y, 0.0  + (0.3 * std::sin(0.157) * 0.1) + (0.5 * std::sin(-0.157) * 0.1), 1e-4);
  ASSERT_NEAR(pose.yaw, -0.157, 1e-4);
}



// Identity一致確認ロジックのテスト
TEST_F(CuGoTest, test_match_identity)
{
  RobotState state;
  state.product_id = 10000;
  state.robot_id = 1;
  EXPECT_TRUE(cugo.match_identity(state));

  state.robot_id = 2;
  EXPECT_FALSE(cugo.match_identity(state));

  state.product_id = 10001;
  state.robot_id = 1;
  EXPECT_FALSE(cugo.match_identity(state));
}