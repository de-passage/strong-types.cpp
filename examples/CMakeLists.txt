add_executable(basic-example basic.cpp)
target_include_directories(basic-example PUBLIC ${LIBRARY_INCLUDE_DIRECTORY})
set_target_options(basic-example)
