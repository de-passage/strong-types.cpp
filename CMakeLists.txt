cmake_minimum_required(VERSION 3.14.0)
project(strong-types VERSION 1.1.0)

include(CTest)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CPack)

###############
# Google Test #
###############

if (BUILD_TESTING)
  # Download and unpack googletest at configure time
  configure_file(gtest.CMakeLists.txt googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                   ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)

  # The gtest/gtest_main targets carry header search path
  # dependencies automatically when using CMake 2.8.11 or
  # later. Otherwise we have to add them here ourselves.
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

###########
# Sources #
###########

set (TEST_SRC_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tests/)
set (EXAMPLE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/examples/)
set (LIBRARY_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/)

add_library(strong-types INTERFACE)
target_include_directories(strong-types INTERFACE
  $<BUILD_INTERFACE:${LIBRARY_INCLUDE_DIRECTORY}>
  $<INSTALL_INTERFACE:include>
)

set(EXPORT_INCLUDE_FILES strong_types.hpp strong_types/iostream.hpp)
list(TRANSFORM EXPORT_INCLUDE_FILES PREPEND "${LIBRARY_INCLUDE_DIRECTORY}/")
target_sources(strong-types INTERFACE FILE_SET
  HEADERS FILES ${EXPORT_INCLUDE_FILES}
  BASE_DIRS "${LIBRARY_INCLUDE_DIRECTORY}"
)

function(set_target_options TARGET)
if(MSVC)
  target_compile_options(${TARGET} PRIVATE /W3 /WX)
else()
  target_compile_options(${TARGET} PRIVATE -pedantic -Wall -Wextra)
endif(MSVC)
endfunction()

if (BUILD_TESTING)
  set (TEST_SRC_FILES basic.cpp iostream.cpp)
  list (TRANSFORM TEST_SRC_FILES PREPEND ${TEST_SRC_DIRECTORY})

  add_executable(tests ${TEST_SRC_FILES})
  target_link_libraries(tests gtest_main strong-types)
  set_target_options(tests)
  add_test(NAME gtests COMMAND tests)

endif()
add_subdirectory(${EXAMPLE_DIRECTORY})

###########
# Install #
###########
set(CONFIG_FILE_DESTINATION "lib/cmake/strong-types" CACHE PATH "Path to the CMake config file. Used to configure the vcpkg install")

install(TARGETS strong-types EXPORT stTargets FILE_SET HEADERS)
install(EXPORT stTargets  NAMESPACE strong-types:: FILE strong-types-config.cmake DESTINATION "${CONFIG_FILE_DESTINATION}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/strong-types-config-version.cmake"
  COMPATIBILITY AnyNewerVersion
  ARCH_INDEPENDENT
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/strong-types-config-version.cmake"
  DESTINATION "${CONFIG_FILE_DESTINATION}")
