/*
   Copyright [2025] [CuboRex Co.,Ltd.]

   cugo_protocol.hpp
   通信プロトコル定義および変換ロジック
*/

#ifndef CUGO_V4_5_ROS2_CONTROL_CUGO_PROTOCOL_HPP
#define CUGO_V4_5_ROS2_CONTROL_CUGO_PROTOCOL_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <cmath>
#include <limits>

namespace cugo_v4_5_ros2_control
{

  // 定数定義
constexpr size_t HANDSHAKE_PACKET_SIZE = 72;   // ハンドシェイク用パケットサイズ
constexpr size_t DEFAULT_PACKET_SIZE = 72;     // デフォルトのパケットサイズ

constexpr size_t HEADER_SIZE = 8;              // ヘッダサイズ (固定)

constexpr size_t HEADER_OFFSET_PRODUCT_ID = 0;
constexpr size_t HEADER_OFFSET_ROBOT_ID = 2;
constexpr size_t HEADER_OFFSET_LENGTH = 4;
constexpr size_t HEADER_OFFSET_CHECKSUM = 6;

constexpr size_t MIN_BODY_SIZE = 64;
constexpr size_t BODY_OFFSET_VEL_LINEAR_X = 0;
constexpr size_t BODY_OFFSET_VEL_LINEAR_Y = 2;
constexpr size_t BODY_OFFSET_VEL_ANGULAR_Z = 4;
constexpr size_t BODY_OFFSET_PRODUCT_ID = 60;
constexpr size_t BODY_OFFSET_ROBOT_ID = 62;

// PC→RPi 追加フィールド body オフセット (ControlCommand)
constexpr size_t BODY_OFFSET_ENABLE_7_14           = 6;
constexpr size_t BODY_OFFSET_MODE_SWITCH           = 7;
constexpr size_t BODY_OFFSET_EMERGENCY_DECEL       = 8;
constexpr size_t BODY_OFFSET_RESET_CTRL_ERROR      = 9;
constexpr size_t BODY_OFFSET_RESET_MD_ERROR        = 10;
constexpr size_t BODY_OFFSET_HEADLIGHT_CMD         = 11;
constexpr size_t BODY_OFFSET_TOWERLIGHT_CMD        = 12;
constexpr size_t BODY_OFFSET_BUMPER_CONFIG_CMD     = 13;
constexpr size_t BODY_OFFSET_BRAKE_CONFIG_CMD      = 14;

// RPi→PC 追加フィールド body オフセット (RobotState)
constexpr size_t BODY_OFFSET_CONTROLLER_STATUS     = 6;
constexpr size_t BODY_OFFSET_CONTROLLER_ERROR      = 7;
constexpr size_t BODY_OFFSET_MOTORDRIVER_ERROR     = 8;
constexpr size_t BODY_OFFSET_DRIVER_VOLTAGE        = 9;   // uint16_t, 実電圧 = raw × 0.1 [V]
constexpr size_t BODY_OFFSET_HEADLIGHT_STATUS      = 11;
constexpr size_t BODY_OFFSET_TOWERLIGHT_STATUS     = 12;
constexpr size_t BODY_OFFSET_IO_INPUT_STATUS       = 13;
constexpr size_t BODY_OFFSET_ENCODER_MOTOR0        = 14;
constexpr size_t BODY_OFFSET_ENCODER_MOTOR1        = 18;
constexpr size_t BODY_OFFSET_ENCODER_MOTOR2        = 22;
constexpr size_t BODY_OFFSET_ENCODER_MOTOR3        = 26;
constexpr size_t BODY_OFFSET_MD_TEMP0              = 30;
constexpr size_t BODY_OFFSET_MD_TEMP1              = 32;
constexpr size_t BODY_OFFSET_MD_TEMP2              = 34;
constexpr size_t BODY_OFFSET_MD_TEMP3              = 36;
constexpr size_t BODY_OFFSET_MD_ERROR_CODE0        = 38;
constexpr size_t BODY_OFFSET_MD_ERROR_CODE1        = 40;
constexpr size_t BODY_OFFSET_MD_ERROR_CODE2        = 42;
constexpr size_t BODY_OFFSET_MD_ERROR_CODE3        = 44;
constexpr size_t BODY_OFFSET_BUMPER_CONFIG         = 46;
constexpr size_t BODY_OFFSET_BRAKE_CONFIG          = 47;

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

  double linear_x;    // [m/s] 前後進速度
  double linear_y;    // [m/s] 横移動速度
  double angular_z;   // [rad/s] 旋回速度

  // 追加 I/O フィールド (body[6-14])
  uint8_t enable_7_14            = 0;  // body[6]: body[7-14] の有効ビットフィールド
  uint8_t mode_switch            = 0;  // body[7]: 0x80=RCモード / 0x81=CMDモード
  uint8_t emergency_decel        = 0;  // body[8]: 緊急減速トリガ
  uint8_t reset_controller_error = 0;  // body[9]: クリアするコントローラエラービット
  uint8_t reset_motordriver_error = 0; // body[10]: クリアするモータドライバエラービット
  uint8_t headlight_control      = 0;  // body[11]: ヘッドライト on/off ビットフィールド
  uint8_t towerlight_control     = 0;  // body[12]: タワーライト点灯モード
  uint8_t bumper_config          = 0;  // body[13]: バンパー設定値
  uint8_t brake_config           = 0;  // body[14]: ブレーキ設定値
};

  /**
   * @brief ロボットからPCへ報告される状態データ
   */
struct RobotState
{
  uint16_t product_id;
  uint16_t robot_id;

  double linear_x;    // [m/s]
  double linear_y;    // [m/s]
  double angular_z;   // [rad/s]

  // 追加 I/O フィールド (body[6-47])
  uint8_t  controller_status        = 0;  // body[6]
  uint8_t  controller_error         = 0;  // body[7]
  uint8_t  motordriver_error        = 0;  // body[8]
  uint16_t driver_voltage_raw       = 0;  // body[9-10]: 実電圧 = raw × 0.1 [V]
  uint8_t  headlight_status         = 0;  // body[11]
  uint8_t  towerlight_status        = 0;  // body[12]
  uint8_t  io_input_status          = 0;  // body[13]: 外部4bitデジタル入力
  uint32_t encoder_motor0           = 0;  // body[14-17]
  uint32_t encoder_motor1           = 0;  // body[18-21]
  uint32_t encoder_motor2           = 0;  // body[22-25]
  uint32_t encoder_motor3           = 0;  // body[26-29]
  uint16_t motordriver_temp0        = 0;  // body[30-31] [℃]
  uint16_t motordriver_temp1        = 0;  // body[32-33] [℃]
  uint16_t motordriver_temp2        = 0;  // body[34-35] [℃]
  uint16_t motordriver_temp3        = 0;  // body[36-37] [℃]
  uint16_t motordriver_error_code0  = 0;  // body[38-39]
  uint16_t motordriver_error_code1  = 0;  // body[40-41]
  uint16_t motordriver_error_code2  = 0;  // body[42-43]
  uint16_t motordriver_error_code3  = 0;  // body[44-45]
  uint8_t  bumper_config            = 0;  // body[46]: フラッシュ保存値の読み出し
  uint8_t  brake_config             = 0;  // body[47]: フラッシュ保存値の読み出し
};

  /**
   * @brief オドメトリ計算用の2次元ポーズ
   */
struct Pose2D
{
  double x;     // [m]
  double y;     // [m]
  double yaw;   // [rad]
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
    const ControlCommand & cmd,
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
    const std::vector<uint8_t> & packet,
    RobotState & out_state,
    std::string & error_msg,
    size_t packet_size = DEFAULT_PACKET_SIZE);

    // --- ハンドシェイク用 (RobotStateを流用) ---
    /**
     * @brief ハンドシェイク用パケット生成
     * RobotState内の PID, RID のみを使用し、独自形式でシリアライズする
     */
  static std::vector<uint8_t> serialize_handshake(
    const RobotState & expected_state);

    /**
     * @brief ハンドシェイク用パケット解析
     * 受信した独自形式を RobotState の PID, RID に復元する。速度は0とする。
     */
  static bool deserialize_handshake(
    const std::vector<uint8_t> & packet,
    const RobotState & expected_state,
    RobotState & out_data);

    // -------------------------------------------------------
    // ユーティリティ
    // -------------------------------------------------------

    /**
     * @brief COBSエンコード
     * @return エンコードされたデータ(失敗時は空vector)
     */
  static std::vector<uint8_t> encode_cobs(const std::vector<uint8_t> & raw);

    /**
     * @brief COBSデコード
     * @return デコードされたデータ(失敗時は空vector)
     */
  static std::vector<uint8_t> decode_cobs(const std::vector<uint8_t> & encoded);

    /**
     * @brief チェックサム計算 (16bit)
     */
  static uint16_t calc_checksum(const uint8_t *data, size_t size);

private:
    /**
     * @brief 引数に渡された2つのPIDが、通信プロトコルに対応しているか確認します。
     * @return 対応していればtrue、そうでなければfalse
     */
  static bool is_protocol_compatible(
    const uint16_t received_product_id,
    const uint16_t expected_product_id);

    /**
     * @brief 速度変換: m/s (double) -> mm/s (int16_t)
     * 1000倍し、int16の範囲(-32768 ~ 32767)に丸め込みます。
     */
  static int16_t velocity_to_int16(double velocity_ms);

    /**
     * @brief 速度変換: mm/s (int16_t) -> m/s (double)
     * 1/1000倍して double に変換します。
     */
  static double velocity_to_double(int16_t velocity_mms);

    // バイナリ変換
  static std::vector<unsigned char> float_to_bin(float value);
  static float bin_to_float(const unsigned char * data);
  static std::vector<unsigned char> int32_to_bin(int32_t value);
  static int32_t bin_to_int32(const unsigned char * data);
};

} // namespace cugo_v4_5_ros2_control

#endif // CUGO_V4_5_ROS2_CONTROL_CUGO_PROTOCOL_HPP
