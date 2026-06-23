add_executable(smart_monitor
    src/main.c
    vendor/generic/generic.c
    vendor/longsys/longsys.c
    vendor/micron/micron.c
    vendor/samsung/samsung.c
)

target_include_directories(smart_monitor
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/vendor
)

install(TARGETS smart_monitor
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    COMPONENT tools
)
