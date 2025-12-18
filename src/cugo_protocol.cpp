/*
   Copyright [2025] [CuboRex Co.,Ltd.]
   cugo_protocol.cpp

   通信プロトコル実装

*/

#include "cugo_ros2_control2/cugo_protocol.hpp"

#include <cstring>   // for memcpy
#include <algorithm> // for std::find

namespace cugo_ros2_control2
{

  std::vector<uint8_t> CugoProtocol::serialize(const ControlCommand &cmd, size_t packet_size)
  {
    // 安全性のチェック: ヘッダサイズより小さいパケットは作成不可
    if (packet_size < HEADER_SIZE)
    {
      // 呼び出し元で vector.empty() を確認してもらう
      return {};
    }

    // 1. 生パケット(COBS前)の領域確保
    std::vector<uint8_t> raw_packet(packet_size, 0);

    // 2. ボディサイズ計算
    size_t body_size = packet_size - HEADER_SIZE;

    // ポインタ計算用
    uint8_t *header_ptr = raw_packet.data();
    uint8_t *body_ptr = raw_packet.data() + HEADER_SIZE;

    // 3. ヘッダ情報の作成
    uint16_t length = static_cast<uint16_t>(packet_size);

    std::memcpy(header_ptr + 0, &cmd.product_id, sizeof(uint16_t));
    std::memcpy(header_ptr + 2, &cmd.robot_id, sizeof(uint16_t));
    std::memcpy(header_ptr + 4, &length, sizeof(uint16_t));

    // 4. ボディ情報の書き込み
    // TODO: プロトコルに合わせて適切な形に変える
    // 十分なサイズがある場合のみ書き込む
    if (body_size >= (sizeof(double) * 3))
    {
    //   std::memcpy(body_ptr + 0, &cmd.linear_x, sizeof(double));
    //   std::memcpy(body_ptr + 8, &cmd.linear_y, sizeof(double));
    //   std::memcpy(body_ptr + 16, &cmd.angular_z, sizeof(double));
    }
    else
    {
      // サイズ不足エラー
      return {};
    }

    // 5. チェックサムの計算と設定
    uint16_t checksum = calc_checksum(body_ptr, body_size);
    std::memcpy(header_ptr + 6, &checksum, sizeof(uint16_t));

    // 6. COBSエンコード
    return encode_cobs(raw_packet);
  }

  bool CugoProtocol::deserialize(
      const std::vector<uint8_t> &packet,
      RobotState &out_state,
      std::string &error_msg,
      size_t packet_size)
  {
    // エラーメッセージ初期化
    error_msg.clear();

    // 1. COBSデコード
    std::vector<uint8_t> decoded_data = decode_cobs(packet);

    if (decoded_data.empty())
    {
      error_msg = "COBS Decode Failed or Empty Packet";
      return false;
    }

    // 2. サイズチェック
    if (decoded_data.size() != packet_size)
    {
      error_msg = "Packet Size Mismatch. Expected: " + std::to_string(packet_size) +
                  ", Actual: " + std::to_string(decoded_data.size());
      return false;
    }

    // 3. ボディサイズ計算
    if (packet_size < HEADER_SIZE)
    {
      error_msg = "Invalid packet_size setting (< HEADER_SIZE)";
      return false;
    }
    size_t body_size = packet_size - HEADER_SIZE;

    // 4. チェックサム検証
    const uint8_t *header_ptr = decoded_data.data();
    const uint8_t *body_ptr = decoded_data.data() + HEADER_SIZE;

    uint16_t received_checksum;
    std::memcpy(&received_checksum, header_ptr + 6, sizeof(uint16_t));

    uint16_t calculated_checksum = calc_checksum(body_ptr, body_size);

    if (received_checksum != calculated_checksum)
    {
      error_msg = "Checksum Mismatch. Recv: " + std::to_string(received_checksum) +
                  ", Calc: " + std::to_string(calculated_checksum);
      return false;
    }

    // 5. 構造体への復元
    std::memcpy(&out_state.product_id, header_ptr + 0, sizeof(uint16_t));
    std::memcpy(&out_state.robot_id, header_ptr + 2, sizeof(uint16_t));

    // TODO: プロトコルに合わせて適切に復元
    if (body_size >= (sizeof(double) * 3))
    {
      // std::memcpy(&out_state.linear_x, body_ptr + 0, sizeof(double));
      // std::memcpy(&out_state.linear_y, body_ptr + 8, sizeof(double));
      // std::memcpy(&out_state.angular_z, body_ptr + 16, sizeof(double));
    }
    else
    {
      // データ不足エラー
      error_msg = "Body size too small for RobotState data";
      return false;
    }

    return true; // 成功
  }

  uint16_t CugoProtocol::calc_checksum(const uint8_t *data, size_t size)
  {
    if (size % 2 != 0)
    {
      return 0;
    }

    uint32_t sum = 0;
    for (size_t i = 0; i < size; i += 2)
    {
      uint16_t word = (static_cast<uint16_t>(data[i + 1]) << 8) | static_cast<uint16_t>(data[i]);
      sum += word;
    }

    if (sum >> 16)
    {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~static_cast<uint16_t>(sum);
  }

  std::vector<uint8_t> CugoProtocol::encode_cobs(const std::vector<uint8_t> &raw)
  {
    const size_t source_size = raw.size();
    if (source_size == 0)
      return {0x01, 0x00};

    size_t max_encoded_size = source_size + (source_size / 254) + 1;
    std::vector<uint8_t> encoded_packet(max_encoded_size);

    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    uint8_t code = 1;

    while (read_index < source_size)
    {
      if (raw[read_index] == 0)
      {
        encoded_packet[code_index] = code;
        code = 1;
        code_index = write_index++;
        read_index++;
      }
      else
      {
        encoded_packet[write_index++] = raw[read_index++];
        code++;

        if (code == 0xFF)
        {
          encoded_packet[code_index] = code;
          code = 1;
          code_index = write_index++;
        }
      }
    }

    encoded_packet[code_index] = code;
    encoded_packet.resize(write_index);
    encoded_packet.push_back(0x00);

    return encoded_packet;
  }

  std::vector<uint8_t> CugoProtocol::decode_cobs(const std::vector<uint8_t> &encoded)
  {
    if (encoded.empty())
      return {};

    size_t end_pos = encoded.size();
    if (encoded.back() == 0x00)
      end_pos--;
    if (end_pos == 0)
      return {};

    const std::vector<uint8_t> encoded_body(encoded.begin(), encoded.begin() + end_pos);
    const size_t source_size = encoded_body.size();

    std::vector<uint8_t> decoded;
    decoded.reserve(source_size);

    size_t read_index = 0;
    while (read_index < source_size)
    {
      uint8_t code = encoded_body[read_index];

      // エラー: コード0はありえない -> 空を返して呼び出し元でハンドリング
      if (code == 0)
        return {};

      read_index++;

      for (uint8_t i = 1; i < code; i++)
      {
        if (read_index >= source_size)
          return {}; // データ不足エラー
        decoded.push_back(encoded_body[read_index++]);
      }

      if (code != 0xFF && read_index < source_size)
        decoded.push_back(0x00);
    }
    return decoded;
  }

  std::vector<unsigned char> CugoProtocol::float_to_bin(float value) {
      std::vector<unsigned char> data(sizeof(float));
      std::memcpy(data.data(), &value, sizeof(float));
      return data;
  }
  float CugoProtocol::bin_to_float(const unsigned char * data) {
      float value;
      std::memcpy(&value, data, sizeof(float));
      return value;
  }
  std::vector<unsigned char> CugoProtocol::int32_to_bin(int32_t value) {
      std::vector<unsigned char> data(sizeof(int32_t));
      std::memcpy(data.data(), &value, sizeof(int32_t));
      return data;
  }
  int32_t CugoProtocol::bin_to_int32(const unsigned char * data) {
      int32_t value;
      std::memcpy(&value, data, sizeof(int32_t));
      return value;
  }

} // namespace cugo_ros2_control2