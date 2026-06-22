# Current tests cover stable UAPI layouts and userspace PDI wrappers.
# Old runtime/config/MCU/smoke architecture tests targeted the removed
# pre-bus framework and need to be rewritten for the Linux bus model.
add_subdirectory(abi)
add_subdirectory(pdi)
