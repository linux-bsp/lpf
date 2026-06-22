add_executable(pdm_architecture_boundaries_test
    test_lpf_architecture_boundaries.c
)

target_compile_definitions(pdm_architecture_boundaries_test PRIVATE
    SDK_SOURCE_DIR="${SDK_PATH}"
)

add_test(NAME pdm_architecture_boundaries
    COMMAND pdm_architecture_boundaries_test
)
