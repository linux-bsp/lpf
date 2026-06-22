add_executable(pdm_uapi_layout_test test_uapi_layout.c)
target_include_directories(pdm_uapi_layout_test PRIVATE
    "${SDK_PATH}/uapi"
)

add_test(NAME pdm_uapi_layout COMMAND pdm_uapi_layout_test)
