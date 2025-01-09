cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("gamelib")

file(GLOB_RECURSE HEADER_FILES zskip/*.h aoi/*.h)
file(GLOB_RECURSE SOURCE_FILES zskip/*.cpp aoi/*.cpp)

message(STATUS "zsk 文件目录" ${HEADER_FILES} ${SOURCE_FILES})

add_executable (main "main.cpp" ${HEADER_FILES} ${SOURCE_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET main PROPERTY CXX_STANDARD 20)
endif()

