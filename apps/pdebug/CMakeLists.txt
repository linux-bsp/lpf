add_executable(pdebug
    pdebug.c
)

target_link_libraries(pdebug PRIVATE pdi osal)
target_include_directories(pdebug PRIVATE
    ${CMAKE_SOURCE_DIR}/user/pdi/include
    ${CMAKE_SOURCE_DIR}/uapi
    ${CMAKE_SOURCE_DIR}/include
)

install(TARGETS pdebug
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    COMPONENT tools
)
