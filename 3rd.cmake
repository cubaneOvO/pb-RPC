include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://gitcode.com/gh_mirrors/ya/yaml-cpp.git
  GIT_TAG master
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://gitcode.com/gh_mirrors/sp/spdlog.git
  GIT_TAG v1.x
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog
)
FetchContent_MakeAvailable(spdlog)