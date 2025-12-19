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
#include "cugo_ros2_control2/serial.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <thread>

using namespace cugo_ros2_control2;

// テストフィクスチャ
class SerialTest : public ::testing::Test
{
protected:
  // テスト用の仮想ポート名
  // socat -d -d pty,raw,echo=0,link=/tmp/ttyV0 pty,raw,echo=0,link=/tmp/ttyV1
  const std::string TEST_PORT_MASTER = "/tmp/ttyV0";
  
  std::shared_ptr<Serial> test_serial;

  void SetUp() override
  {
    // テスト用のSerialインスタンスを初期化
    test_serial = std::make_shared<Serial>(0x00);
  }

  void TearDown() override
  {
    // クリーンアップ
        if (test_serial && test_serial->is_open()) {
      test_serial->close();
    }
  }

  bool port_exists(const std::string& port) {
    std::ifstream f(port.c_str());
    return f.good();
  }
};

// USBとRaspberryPiPicoを接続してテスト、または仮想ポートを使用
TEST_F(SerialTest, test_open)
{
  std::cout << "[Serial TEST] test_open" << std::endl;

  // 仮想ポートまたは実機が存在する場合のみオープンを試行
  // (CI環境などで落ちないようにチェック)
  std::string port_to_use = "/dev/ttyACM0";
  if (port_exists(TEST_PORT_MASTER)) {
    port_to_use = TEST_PORT_MASTER;
  } else if (!port_exists(port_to_use)) {
    std::cout << "[SKIP] No serial port found. Skipping open test." << std::endl;
    return; 
  }

  // シリアルポートを開くテスト
  EXPECT_NO_THROW({test_serial->open(port_to_use, 115200);});
  EXPECT_TRUE(test_serial->is_open());

  // 存在しないポートを開こうとすると例外を投げるテスト
  test_serial->close();
  EXPECT_THROW({test_serial->open("/dev/null", 115200);}, boost::system::system_error);
}

TEST_F(SerialTest, test_close)
{
  std::cout << "[Serial TEST] test_close" << std::endl;
  
  std::string port_to_use = "/dev/ttyACM0";
  if (port_exists(TEST_PORT_MASTER)) {
    port_to_use = TEST_PORT_MASTER;
  } else if (!port_exists(port_to_use)) {
    return; // スキップ
  }

  // ポートを開いてから閉じるテスト
  EXPECT_NO_THROW({test_serial->open(port_to_use, 115200);});
  EXPECT_TRUE(test_serial->is_open());

  EXPECT_NO_THROW({test_serial->close();});
  EXPECT_FALSE(test_serial->is_open());
}

TEST_F(SerialTest, test_reconnect)
{
  std::cout << "[Serial TEST] test_reconnect" << std::endl;
  
  std::string port_to_use = "/dev/ttyACM0";
  if (port_exists(TEST_PORT_MASTER)) {
    port_to_use = TEST_PORT_MASTER;
  } else if (!port_exists(port_to_use)) {
    return; // スキップ
  }


  // 正常にオープンできることを確認
  ASSERT_NO_THROW(
  {
    test_serial->open(port_to_use, 115200);
  });
  ASSERT_TRUE(test_serial->is_open());

    // 一度閉じる
  test_serial->close();
  // io_contextが止まると再開できないのでリスタートする
  test_serial->io_context_.restart();
  test_serial->io_thread_ = std::thread([&]() {test_serial->io_context_.run();});
  ASSERT_FALSE(test_serial->is_open());

  // 再接続処理を呼び出す
  bool success = false;
  ASSERT_NO_THROW(
  {
    success = test_serial->reconnect(port_to_use, 115200);
  });

  // 再接続が成功し、ポートが開いていることを確認
  EXPECT_TRUE(success);
  EXPECT_TRUE(test_serial->is_open());
}