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

#include "cugo_ros2_control2/serial.hpp"

namespace cugo_ros2_control2
{

Serial::Serial(uint8_t delimiter)
: delimiter_(delimiter), serial_port_(io_context_)
{
  work_guard_.emplace(boost::asio::make_work_guard(io_context_.get_executor()));
  io_thread_ = std::thread([this]() {
        try {
          io_context_.run();
        } catch (...) {
        }
  });
  std::cout << "[Serial INFO] Serial object created with delimiter: "
            << static_cast<int>(delimiter_) << std::endl;
}

Serial::~Serial()
{
  std::cout << "[Serial INFO] Destroying Serial object..." << std::endl;
  close();
  if (io_thread_.joinable()) {
    io_thread_.join();
  }
  std::cout << "[Serial INFO] Serial object destroyed." << std::endl;
}

void Serial::open(const std::string & port, int baudrate)
{
  std::cout << "[Serial INFO] open()" << std::endl;
  if (serial_port_.is_open()) {
    std::cerr << "[Serial WARN] Serial port " << port << " alrady open." << std::endl;
    return;
  }

  try {
    serial_port_.open(port);
    serial_port_.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
    serial_port_.set_option(boost::asio::serial_port_base::character_size(8));
    serial_port_.set_option(boost::asio::serial_port_base::parity(
        boost::asio::serial_port_base::parity::none));
    serial_port_.set_option(boost::asio::serial_port_base::stop_bits(
        boost::asio::serial_port_base::stop_bits::one));
    serial_port_.set_option(boost::asio::serial_port_base::flow_control(
        boost::asio::serial_port_base::flow_control::none));

    std::cout << "[Serial INFO] Serial port " << port << " opened with baudrate " << baudrate
              << std::endl;

    // I/O スレッド開始 (エラーハンドリング)
    if (!io_thread_.joinable()) {
      io_thread_ = std::thread(
        [this]() {
          try {
            io_context_.run();
            std::cout << "[Serial INFO] io_context_ finished." << std::endl;
          } catch (const std::exception & e) {
            std::cerr << "[Serial FATAL] io_context_ exception: " << e.what() << std::endl;
          }
        });
    }
    start_read();  // ポートが開いたら読み取りを開始
  } catch (const boost::system::system_error & e) {
    std::cerr << "[Serial ERROR] Error opening serial port " << port << ": " << e.what()
              << std::endl;
    std::cout
      << "[Serial INFO] シリアルポートの指定が正しいか、読み書き権限があるか確認してください"
      << std::endl;
    throw e;
  }
}

void Serial::close()
{
  std::cout << "[Serial INFO] close()" << std::endl;
  // io_context_に与えていたダミーワークをリセットしてrun()から脱出
  work_guard_.reset();

  if (!io_context_.stopped()) {
    io_context_.stop();
  }

  if (io_thread_.joinable()) {
    io_thread_.join();
  }

  if (serial_port_.is_open()) {
    boost::system::error_code ec;
    serial_port_.close(ec);
    if (ec) {std::cerr << "[Serial ERROR] Error closing: " << ec.message() << std::endl;}
  }
}

bool Serial::reconnect(const std::string & port, int baudrate)
{
  std::cout << "[Serial INFO] Reconnecting..." << std::endl;
  close();

  // io_contextを再利用可能な状態に戻す
  if (io_context_.stopped()) {
    io_context_.restart();
  }
  // work_guard_の再構築
  work_guard_.emplace(boost::asio::make_work_guard(io_context_.get_executor()));

  try {
    open(port, baudrate);
    std::cout << "[Serial INFO] Reconnect successful." << std::endl;
    return true;
  } catch (const std::exception & e) {
    std::cerr << "[Serial WARN] Reconnect failed: " << e.what() << std::endl;
    return false;
  }
}

bool Serial::handshake()
{
  std::cout << "[Serial INFO] Handshaking..." << std::endl;
  if (!serial_port_.is_open()) {return false;}

  // TODO: ハンドシェイク処理をここに記述する
  // 例: 通信相手に接続要求を送り、ACKとともにハードウェア情報が保存されていれば確認する

  std::cout << "[Serial INFO] Handshake done." << std::endl;
  return true;
}

bool Serial::is_open() const
{
  return serial_port_.is_open();
}

void Serial::register_callback(DataCallback callback)
{
  data_callback_ = callback;
}

void Serial::start_read()
{
  serial_port_.async_read_some(
    boost::asio::buffer(raw_read_buffer_),
    boost::bind(
      &Serial::handle_read, this, boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

void Serial::handle_read(const boost::system::error_code & error, std::size_t bytes_transferred)
{
  // --- 1. エラーチェック ---
  if (error) {
    if (error == boost::asio::error::operation_aborted) {
      // ポートが閉じられたなどの正常な終了
      std::cout << "[Serial INFO][handle_read] Read operation aborted." << std::endl;
    } else {
      // その他の予期せぬエラー
      std::cerr << "[Serial ERROR][handle_read] Read error: " << error.message() << std::endl;
    }
    return;  // エラー時はループを継続しない
  }

  if (bytes_transferred > 0) {
    packet_buffer_.insert(packet_buffer_.end(), raw_read_buffer_.begin(),
        raw_read_buffer_.begin() + bytes_transferred);
  }

  // --- 3. バッファからパケットを探索・処理するループ ---
  while (true) {
    // デリミタ変数を使用
    auto it = std::find(packet_buffer_.begin(), packet_buffer_.end(), delimiter_);

    // デリミタが見つからなければ、まだパケットが全部届いていない。
    // ループを抜けて、次のデータ受信を待つ。
    if (it == packet_buffer_.end()) {
      break;
    }

    // --- 4. 完全なパケットが見つかったので、処理する ---
    // デリミタの位置+1までのデータを1つのパケットとして抽出
    std::vector<unsigned char> received_packet(packet_buffer_.begin(), it + 1);
    packet_buffer_.erase(packet_buffer_.begin(), it + 1);

    if (data_callback_) {
      data_callback_(received_packet);
    }
  }

  // --- 5. 次の非同期読み込みを開始 ---
  start_read();
}

void Serial::write(const std::vector<unsigned char> & encoded_packet)
{
  // Step 1: エンコードされたパケットを非同期で送信する
  boost::asio::async_write(
    serial_port_, boost::asio::buffer(encoded_packet),
    boost::bind(
      &Serial::handle_write, this, boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

void Serial::handle_write(
  const boost::system::error_code & error,
  std::size_t /*bytes_transferred*/)
{
  if (error) {
    // ポートを閉じたことによる正常な中断はエラーとして扱わない
    if (error == boost::asio::error::operation_aborted) {
      std::cout << "[Serial INFO][handle_write] Write operation aborted." << std::endl;
    } else {
      // その他の書き込みエラー
      std::cerr << "[Serial ERROR][handle_write] Write error: " << error.message() << std::endl;
    }
    return;
  }
}

} // namespace cugo_ros2_control2
