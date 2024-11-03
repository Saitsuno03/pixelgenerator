cmake_minimum_required(VERSION 3.15)
project(PixelBackgroundGenerator)

set(CMAKE_CXX_STANDARD 11)

# DirectX and Windows dependencies
include_directories(C:/PathToDirectXSDK/include)
link_directories(C:/PathToDirectXSDK/lib)

add_executable(PixelBackgroundGenerator src/main.cs)

target_link_libraries(PixelBackgroundGenerator d3d11 d3dcompiler dxgi)
