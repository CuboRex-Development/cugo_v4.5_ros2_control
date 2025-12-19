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
#include "cugo_ros2_control2/cugo_protocol.hpp"
#include <vector>
#include <cstring>
#include <limits>
#include <cmath>

using namespace cugo_ros2_control2;

// テストフィクスチャ
class CugoProtocolTest : public ::testing::Test {
protected:
  void SetUp() override {
  }
};

// CugoProtocol::calc_checksum のテストケース
TEST_F(CugoProtocolTest, test_calc_checksum)
{
  // ケース1: ボディデータがすべてゼロ
  std::vector<unsigned char> body1(64, 0); // MIN_BODY_SIZE想定
  // 0x00 * 32 words -> sum=0 -> ~0 = 0xFFFF
  EXPECT_EQ(CugoProtocol::calc_checksum(body1.data(), body1.size()), 0xFFFF);

  // ケース2: 特定のパターン (0x01, 0x00... を想定して手計算と比較)
  std::vector<unsigned char> body2(64, 0);
  body2[0] = 0x01; 
  // Word[0] = 0x0001 (little endian: 01 00)
  // Sum = 1 -> Checksum = ~1 = 0xFFFE
  EXPECT_EQ(CugoProtocol::calc_checksum(body2.data(), body2.size()), 0xFFFE);

  // ケース3: 全ビットが1
  std::vector<unsigned char> body3(64, 0xFF);
  // Word = 0xFFFF. 32個の0xFFFF
  // 0xFFFF * 32 = 0x1FFFE0
  // Carry wrap around: 0x1F + 0xFFE0 = 0xFFFF
  // Checksum = ~0xFFFF = 0x0000
  EXPECT_EQ(CugoProtocol::calc_checksum(body3.data(), body3.size()), 0x0000);
}

// COBSエンコードのテスト
TEST_F(CugoProtocolTest, test_encode_cobs)
{
  // ケース1: ゼロを含まない単純なデータ
  std::vector<unsigned char> raw1 = {0x11, 0x22, 0x33};
  std::vector<unsigned char> expected1 = {0x04, 0x11, 0x22, 0x33, 0x00};
  EXPECT_EQ(CugoProtocol::encode_cobs(raw1), expected1);

  // ケース2: ゼロが真ん中にあるデータ
  std::vector<unsigned char> raw2 = {0x11, 0x00, 0x22};
  std::vector<unsigned char> expected2 = {0x02, 0x11, 0x02, 0x22, 0x00};
  EXPECT_EQ(CugoProtocol::encode_cobs(raw2), expected2);

  // ケース3: 先頭がゼロのデータ
  std::vector<unsigned char> raw3 = {0x00, 0x11, 0x22};
  std::vector<unsigned char> expected3 = {0x01, 0x03, 0x11, 0x22, 0x00};
  EXPECT_EQ(CugoProtocol::encode_cobs(raw3), expected3);
}

// COBSデコードのテスト
TEST_F(CugoProtocolTest, test_decode_cobs)
{
  // ケース1: 単純なデータ
  std::vector<unsigned char> encoded1 = {0x04, 0x11, 0x22, 0x33, 0x00};
  std::vector<unsigned char> expected1 = {0x11, 0x22, 0x33};
  EXPECT_EQ(CugoProtocol::decode_cobs(encoded1), expected1);

  // ケース2: ゼロが復元されるデータ
  std::vector<unsigned char> encoded2 = {0x02, 0x11, 0x02, 0x22, 0x00};
  std::vector<unsigned char> expected2 = {0x11, 0x00, 0x22};
  EXPECT_EQ(CugoProtocol::decode_cobs(encoded2), expected2);

  // ケース3: 先頭のゼロが復元されるデータ
  std::vector<unsigned char> encoded3 = {0x01, 0x03, 0x11, 0x22, 0x00};
  std::vector<unsigned char> expected3 = {0x00, 0x11, 0x22};
  EXPECT_EQ(CugoProtocol::decode_cobs(encoded3), expected3);


  // ケース// ケース4: 254バイトの非ゼロデータ (境界値)
  std::vector<unsigned char> encoded4;
  encoded4.push_back(0xFF);
  std::vector<unsigned char> raw4(254, 0xAA);
  encoded4.insert(encoded4.end(), raw4.begin(), raw4.end());
  EXPECT_EQ(CugoProtocol::decode_cobs(encoded4),raw4);


  // ケース5 (異常系): 不正なデータ (エンコード後に0x00は現れないはずの場所に0x00)
  std::vector<unsigned char> invalid_data = {0x02, 0x11, 0x00, 0x02, 0x22};
  std::vector<unsigned char> result = CugoProtocol::decode_cobs(invalid_data);
  EXPECT_TRUE(result.empty()); 
}

// 速度変換精度のテスト (m/s <-> int16_t mm/s)
TEST_F(CugoProtocolTest, test_velocity_conversion)
{
  ControlCommand cmd;
  cmd.product_id = 10000;
  cmd.robot_id = 1;
  
  // テスト値: 1.234 m/s -> 1234 mm/s
  cmd.linear_x = 1.234;
  cmd.linear_y = -0.567; // -> -567 mm/s
  cmd.angular_z = 0.0;

  // シリアライズ (内部で変換が行われる)
  std::vector<uint8_t> packet = CugoProtocol::serialize(cmd);
  ASSERT_FALSE(packet.empty());

  // デコードしてバイナリレベルで確認
  std::vector<uint8_t> decoded = CugoProtocol::decode_cobs(packet);
  
  // ボディのオフセット計算
  const uint8_t* body = decoded.data() + HEADER_SIZE;
  
  int16_t val_x, val_y;
  std::memcpy(&val_x, body + BODY_OFFSET_VEL_LINEAR_X, sizeof(int16_t));
  std::memcpy(&val_y, body + BODY_OFFSET_VEL_LINEAR_Y, sizeof(int16_t));

  EXPECT_EQ(val_x, 1234);
  EXPECT_EQ(val_y, -567);
}

// 通常パケットの生成テスト
// (旧 test_create_packet を適合)
TEST_F(CugoProtocolTest, test_serialize_command)
{
  ControlCommand cmd;
  cmd.product_id = 10000;
  cmd.robot_id = 5;
  cmd.linear_x = 1.0;
  cmd.linear_y = 0.0;
  cmd.angular_z = -1.0;

  std::vector<uint8_t> packet = CugoProtocol::serialize(cmd);

  // サイズチェック (COBSエンコード後なので、最低でも DEFAULT_PACKET_SIZE + オーバーヘッド)
  // 元データが72バイトの場合、COBSで 72 + 2(overhead) + 1(delimiter) = 75バイト程度になる
  ASSERT_GT(packet.size(), DEFAULT_PACKET_SIZE); 
  ASSERT_EQ(packet.back(), 0x00); // デリミタ確認

  // 中身の検証（デシリアライズして戻るか）
  RobotState result;
  std::string err;
  // デシリアライズは expected ID との一致を見るのでセットしておく
  result.product_id = 10000;
  result.robot_id = 5;

  bool success = CugoProtocol::deserialize(packet, result, err);
  EXPECT_TRUE(success) << "自己生成パケットのデシリアライズに失敗: " << err;
  
  // 値の復元確認 (mm/s変換の誤差 0.001 m/s までは許容)
  EXPECT_NEAR(result.linear_x, 1.0, 1e-3);
  EXPECT_NEAR(result.angular_z, -1.0, 1e-3);
}