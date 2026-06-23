add_executable(debug_study
    debug_study.c
)

install(TARGETS debug_study
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    COMPONENT tools
)
