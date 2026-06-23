add_executable(memory_monitor
    main.c
    src/display.c
    src/sys_mem.c
    src/user_mem.c
)

target_include_directories(memory_monitor
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)

target_compile_features(memory_monitor
    PRIVATE
        c_std_99
)

install(TARGETS memory_monitor
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    COMPONENT tools
)
