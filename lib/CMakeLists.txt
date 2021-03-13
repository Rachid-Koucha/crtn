INCLUDE_DIRECTORIES(../include)

SET(SRC crtn.c)

if (${HAVE_CRTN_MBX} STREQUAL ON)
  SET(SRC ${SRC} crtn_mbx.c)
endif()

if (${HAVE_CRTN_SEM} STREQUAL ON)
  SET(SRC ${SRC} crtn_sem.c)
endif()

ADD_LIBRARY(crtn SHARED ${SRC})


# Versionning of the library
SET_TARGET_PROPERTIES(crtn PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR})

# Installation of the library
INSTALL(TARGETS crtn
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        COMPONENT crtn)
