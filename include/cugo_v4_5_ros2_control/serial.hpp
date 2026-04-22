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
#ifndef CUGO_V4_5_ROS2_CONTROL_SERIAL_HPP
#define CUGO_V4_5_ROS2_CONTROL_SERIAL_HPP

#include <algorithm>
#include <array>
#include <boost/asio.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <optional>

namespace cugo_v4_5_ros2_control
{

class Serial
{
public:
  // コールバック関数の型エイリアス
  using DataCallback = std::function<void (const std::vector<uint8_t> &)>;

  Serial(uint8_t delimiter = 0x00);
  ~Serial();

  void open(const std::string & port, int baudrate);
  void close();
  bool reconnect(const std::string & port, int baudrate);

  bool is_open() const;

  void start_read();
  void register_callback(DataCallback callback);
  void flush_buffer();

  // packet_buffer_ がこのサイズを超えた場合にフラッシュしてフレーミングを再同期する
  static constexpr size_t MAX_BUFFER_SIZE = 1024;

  // 送信メソッド: 既にエンコード済みのバイト列を受け取る
  void write(const std::vector<uint8_t> & data);

  uint8_t delimiter_;
  boost::asio::io_context io_context_;
  boost::asio::serial_port serial_port_;
  std::thread io_thread_;
  std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>
  work_guard_;

private:
  std::array<uint8_t, 256> raw_read_buffer_;
  std::vector<uint8_t> packet_buffer_;
  DataCallback data_callback_;

  void handle_read(const boost::system::error_code & error, std::size_t bytes_transferred);
  void handle_write(const boost::system::error_code & error, std::size_t bytes_transferred);
};

}  // namespace cugo_v4_5_ros2_control
#endif  // CUGO_V4_5_ROS2_CONTROL_SERIAL_HPP
