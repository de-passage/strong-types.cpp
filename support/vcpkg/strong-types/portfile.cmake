set(VCPKG_BUILD_TYPE release)
vcpkg_from_git(
  OUT_SOURCE_PATH SOURCE_PATH
  URL https://github.com/de-passage/strong-types.cpp
  REF 2a49789d6f7ee884b4aca0c9aa9f449dd088bb07
  HEAD_REF master
)
vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DBUILD_TESTING=OFF
    -DCONFIG_FILE_DESTINATION="${CURRENT_INSTALLED_DIR}/share/strong-types"
)

vcpkg_cmake_install()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
