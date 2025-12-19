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
#include <rclcpp/rclcpp.hpp>
#include "cugo_v4_5_ros2_control/node.hpp"
#include <memory>
#include <chrono>

using namespace cugo_v4_5_ros2_control;
using namespace std::chrono_literals;

class NodeTest : public ::testing::Test
{
protected:
  // テストスイート全体で一度だけ実行されるセットアップ
  static void SetUpTestSuite()
  {
    // ROS 2 の初期化
    rclcpp::init(0, nullptr);
  }

  // テストスイート全体で一度だけ実行されるティアダウン
  static void TearDownTestSuite()
  {
    // ROS 2 のシャットダウン
    rclcpp::shutdown();
  }

  // 各テストケース前に実行されるセットアップ
  void SetUp() override
  {
  }

  // 各テストケース後に実行されるティアダウン
  void TearDown() override
  {
  }
};

// ノードの初期化とパラメータ読み込みのテスト
// 実機がない場合でもインスタンス化でクラッシュしないことを確認
TEST_F(NodeTest, test_initialization)
{
  // ポートが存在しなくても例外で落ちないよう、内部でtry-catchされているか確認
  // (Nodeコンストラクタは通信失敗時に rclcpp::shutdown() を呼ぶ仕様なので、
  //  ここではインスタンス化の試行を行う)

  std::shared_ptr<Node> node;

  // パラメータオーバーライドを使ってテスト
  rclcpp::NodeOptions options;
  options.append_parameter_override("product_id", 10000);
  options.append_parameter_override("serial_port", "/dev/ttyFAKE");

  EXPECT_NO_THROW({
    node = std::make_shared<Node>();
  });

  if (node) {
    EXPECT_STREQ(node->get_name(), "cugo_v4_5_ros2_control");

    // パラメータが正しく宣言されているか
    EXPECT_TRUE(node->has_parameter("product_id"));
    EXPECT_TRUE(node->has_parameter("control_frequency"));
  }
}
