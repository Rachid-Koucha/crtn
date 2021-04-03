# Installation of the header file
INSTALL(FILES ${CMAKE_BINARY_DIR}/include/crtn.h
        DESTINATION include
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        COMPONENT crtn)

