add_executable(lpf_uapi_layout_test test_uapi_layout.c)
target_include_directories(lpf_uapi_layout_test PRIVATE
    "${SDK_PATH}/uapi"
)

add_test(NAME lpf_uapi_layout COMMAND lpf_uapi_layout_test)
