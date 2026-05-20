cmake_minimum_required(VERSION 3.20)
project(RatMoney VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SANITIZE "Enable address/undefined sanitizers" OFF)
option(FUZZ     "Build libFuzzer target (requires clang)" OFF)

set(LIB_SOURCES currency.cpp exchange_rate.cpp)
set(LIB_HEADERS currency.h exchange_rate.h iso4217.h)

# ── static library ────────────────────────────────────────────
add_library(ratmoney_static STATIC ${LIB_SOURCES})
set_target_properties(ratmoney_static PROPERTIES
    OUTPUT_NAME           ratmoney
    POSITION_INDEPENDENT_CODE ON)
target_compile_options(ratmoney_static PRIVATE -Wall -Wextra)
target_include_directories(ratmoney_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ratmoney>)
add_library(RatMoney::Static ALIAS ratmoney_static)

# ── shared library ────────────────────────────────────────────
add_library(ratmoney_shared SHARED ${LIB_SOURCES})
set_target_properties(ratmoney_shared PROPERTIES
    OUTPUT_NAME ratmoney
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR})
target_compile_options(ratmoney_shared PRIVATE -Wall -Wextra)
target_include_directories(ratmoney_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ratmoney>)
add_library(RatMoney::Shared ALIAS ratmoney_shared)

if(SANITIZE)
  foreach(tgt ratmoney_static ratmoney_shared)
    target_compile_options(${tgt} PUBLIC
      -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${tgt} PUBLIC
      -fsanitize=address,undefined)
  endforeach()
endif()

# ── main executable ───────────────────────────────────────────
add_executable(main_bin main.cpp)
target_link_libraries(main_bin PRIVATE RatMoney::Static)

# ── tests ─────────────────────────────────────────────────────
find_package(GTest REQUIRED)
enable_testing()

add_executable(test_bin test.cpp)
target_link_libraries(test_bin PRIVATE RatMoney::Static GTest::gtest_main)
add_test(NAME all_tests COMMAND test_bin)

# ── fuzz ──────────────────────────────────────────────────────
if(FUZZ)
  add_executable(fuzz_target fuzz_target.cpp)
  target_link_libraries(fuzz_target PRIVATE RatMoney::Static)
  target_compile_options(fuzz_target PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(fuzz_target PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

# ── install ───────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ratmoney_static ratmoney_shared
    EXPORT  RatMoneyTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${LIB_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ratmoney)

install(EXPORT RatMoneyTargets
    FILE      RatMoneyTargets.cmake
    NAMESPACE RatMoney::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RatMoney)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RatMoneyConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RatMoneyConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RatMoney)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RatMoneyConfigVersion.cmake
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RatMoneyConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RatMoneyConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RatMoney)
