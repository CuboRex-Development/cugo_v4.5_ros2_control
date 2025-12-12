cmake_minimum_required(VERSION 3.8)
project(cugo_ros2_control2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 依存パッケージの検索
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost libraries not found")
  message(STATUS "Please install with the following command.")
  message(STATUS "sudo apt install libboost-all-dev")
endif()

# インクルードディレクトリの追加
include_directories(
  include
  ${Boost_INCLUDE_DIRS}
)

# ---------------------------------------------------------
# ライブラリの定義
# ---------------------------------------------------------

# 1. Protocol Library (通信プロトコル・データ型)
#    ROS依存は少なく純粋なロジックのみですが、プロジェクトの一部としてビルドします
add_library(cugo_protocol_lib src/cugo_protocol.cpp)
# プロトコル自体は外部ROSパッケージに依存していないため ament_target_dependencies は不要ですが、
# 将来的な拡張に備えて rclcpp だけ入れておくか、何も指定しなくてもOKです。

# 2. CuGo Library (オドメトリ計算など)
add_library(cugo_lib src/cugo.cpp)
# cugo.cpp は protocol.hpp の型定義(Pose2D等)を使うためリンクする
target_link_libraries(cugo_lib cugo_protocol_lib)
ament_target_dependencies(cugo_lib
  rclcpp
  geometry_msgs
)

# 3. Serial Library (通信の土管)
add_library(serial_lib src/serial.cpp)
target_link_libraries(serial_lib ${Boost_LIBRARIES})
ament_target_dependencies(serial_lib
  rclcpp
)

# 4. Node Library (ROSノードの実体)
add_library(node_lib src/node.cpp)
# node.cpp は全ての機能を使用するため、全ライブラリをリンク
target_link_libraries(node_lib cugo_lib serial_lib cugo_protocol_lib)
ament_target_dependencies(node_lib
  rclcpp
  std_msgs
  nav_msgs
  geometry_msgs
  sensor_msgs
  tf2_ros
  tf2_geometry_msgs
  diagnostic_updater
)

# ---------------------------------------------------------
# 実行ファイルの生成
# ---------------------------------------------------------

add_executable(cugo_ros2_control2 src/main.cpp)
target_link_libraries(cugo_ros2_control2 cugo_lib node_lib serial_lib cugo_protocol_lib ${Boost_LIBRARIES})
ament_target_dependencies(
  cugo_ros2_control2
  rclcpp
  std_msgs
  nav_msgs
  sensor_msgs
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
  diagnostic_updater
)

target_compile_features(cugo_ros2_control2 PUBLIC cxx_std_17)

# ---------------------------------------------------------
# インストール設定
# ---------------------------------------------------------

install(TARGETS 
  cugo_ros2_control2
  cugo_lib
  node_lib
  serial_lib
  cugo_protocol_lib
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include
  DESTINATION include/
)
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)
install(DIRECTORY urdf
  DESTINATION share/${PROJECT_NAME}/
)

# ---------------------------------------------------------
# テスト設定
# ---------------------------------------------------------

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # Protocolのテスト
  # TODO:Protocolのテストコードを実装したら追記する
  # ament_add_gtest(test_protocol test/test_protocol.cpp)
  # target_link_libraries(test_protocol cugo_protocol_lib)

  # node.cppのテスト
  ament_add_gtest(test_node test/test_node.cpp src/node.cpp)
  target_link_libraries(test_node cugo_lib serial_lib cugo_protocol_lib)
  ament_target_dependencies(test_node
    rclcpp
    std_msgs
    nav_msgs
    geometry_msgs
    sensor_msgs
    tf2_ros
    tf2_geometry_msgs
    diagnostic_updater
  )

  # cugo.cppのテスト
  ament_add_gtest(test_cugo test/test_cugo.cpp)
  target_link_libraries(test_cugo cugo_lib cugo_protocol_lib)
  ament_target_dependencies(test_cugo
    rclcpp
    std_msgs
    nav_msgs
    geometry_msgs
    diagnostic_updater
  )

  # serial.cppのテスト
  ament_add_gtest(test_serial test/test_serial.cpp)
  target_link_libraries(test_serial serial_lib ${Boost_LIBRARIES})
  ament_target_dependencies(test_serial
    rclcpp
    std_msgs
    nav_msgs
    geometry_msgs
    diagnostic_updater
  )
endif()

ament_package()