/*
   Copyright [2025] [CuboRex Co.,Ltd.]

   cugo_protocol.hpp
   通信プロトコル定義および変換ロジック
*/

#ifndef CUGO_ROS2_CONTROL2_CUGO_PROTOCOL_HPP
#define CUGO_ROS2_CONTROL2_CUGO_PROTOCOL_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

namespace cugo_ros2_control2
{

  // 定数定義
  constexpr size_t DEFAULT_PACKET_SIZE = 72; // デフォルトのパケットサイズ
  constexpr size_t HEADER_SIZE = 8;          // ヘッダサイズ (固定)

  // ==========================================
  // データ型定義 (Types)
  // ==========================================

  /**
   * @brief PCからロボットへ送信する制御コマンド
   */
  struct ControlCommand
  {
    uint16_t product_id;
    uint16_t robot_id;

    double linear_x;  // [m/s] 前後進速度
    double linear_y;  // [m/s] 横移動速度
    double angular_z; // [rad/s] 旋回速度
  };

  /**
   * @brief ロボットからPCへ報告される状態データ
   */
  struct RobotState
  {
    uint16_t product_id;
    uint16_t robot_id;

    double linear_x;  // [m/s]
    double linear_y;  // [m/s]
    double angular_z; // [rad/s]
  };

  /**
   * @brief オドメトリ計算用の2次元ポーズ
   */
  struct Pose2D
  {
    double x;   // [m]
    double y;   // [m]
    double yaw; // [rad]
  };

  // ==========================================
  // プロトコルロジッククラス (Protocol Logic)
  // ==========================================

  class CugoProtocol
  {
  public:
    /**
     * @brief ControlCommand構造体を送信用のバイト列(COBSエンコード済み)に変換します。
     * @param cmd         送信するコマンド構造体
     * @param packet_size 生成するパケットの総サイズ (デフォルト: 72)
     * @return std::vector<uint8_t> 送信可能なバイト列 (失敗時は空のvectorを返します)
     */
    static std::vector<uint8_t> serialize(
        const ControlCommand &cmd,
        size_t packet_size = DEFAULT_PACKET_SIZE);

    /**
     * @brief 受信したパケット(COBSエンコード状態)を解析し、RobotState構造体に変換します。
     * 例外は投げず、エラー発生時はfalseを返し、error_msgに詳細を記述します。
     * @param[in]  packet      受信した1パケット分のデータ
     * @param[out] out_state   解析結果を格納する構造体への参照
     * @param[out] error_msg   エラー発生時に詳細メッセージを格納する文字列
     * @param[in]  packet_size 期待するパケットの総サイズ (デフォルト: 72)
     * @return bool 解析に成功したらtrue、失敗(サイズ違い・破損など)したらfalse
     */
    static bool deserialize(
        const std::vector<uint8_t> &packet,
        RobotState &out_state,
        std::string &error_msg,
        size_t packet_size = DEFAULT_PACKET_SIZE);

    // -------------------------------------------------------
    // ユーティリティ
    // -------------------------------------------------------

    /**
     * @brief COBSエンコード
     * @return エンコードされたデータ（失敗時は空vector）
     */
    static std::vector<uint8_t> encode_cobs(const std::vector<uint8_t> &raw);

    /**
     * @brief COBSデコード
     * @return デコードされたデータ（失敗時は空vector）
     */
    static std::vector<uint8_t> decode_cobs(const std::vector<uint8_t> &encoded);

    /**
     * @brief チェックサム計算 (16bit)
     */
    static uint16_t calc_checksum(const uint8_t *data, size_t size);
  };

} // namespace cugo_ros2_control2

#endif // CUGO_ROS2_CONTROL2_CUGO_PROTOCOL_HPP