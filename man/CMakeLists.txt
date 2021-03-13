# Search for gzip program
INCLUDE (../FindGZIP.cmake)

SET(crtn_man_src_3 crtn.3
                   crtn_attr_new.3
                   crtn_errno.3
                   crtn_join.3
                   crtn_self.3
                   crtn_set_attr_stack_size.3
                   crtn_spawn.3
                   crtn_yield.3
                   crtn_attr_delete.3
                   crtn_cancel.3
                   crtn_exit.3
                   crtn_set_attr_type.3
                   crtn_wait.3)

SET(crtn_man_src_7 crtn.7)

if (${HAVE_CRTN_MBX} STREQUAL ON)
  SET(crtn_man_src_3 ${crtn_man_src_3} 
                     crtn_mbx.3
                     crtn_mbx_new.3
                     crtn_mbx_delete.3
                     crtn_mbx_wait.3
                     crtn_mbx_post.3
                     crtn_mbx_alloc.3
                     crtn_mbx_free.3
                     crtn_mbx_format.3)
endif()

if (${HAVE_CRTN_SEM} STREQUAL ON)
  SET(crtn_man_src_3 ${crtn_man_src_3}
                     crtn_sem.3
                     crtn_sem_new.3
                     crtn_sem_delete.3
                     crtn_sem_v.3
                     crtn_sem_p.3)
endif()

STRING(REGEX REPLACE ".3" ".3.gz" crtn_man_gz_3 "${crtn_man_src_3}")
STRING(REGEX REPLACE ".7" ".7.gz" crtn_man_gz_7 "${crtn_man_src_7}")


# Compression of the manuals
FOREACH(man ${crtn_man_src_3} ${crtn_man_src_7})
  ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
                     COMMAND ${GZIP_TOOL} -c ${man} > ${man}.gz
                     DEPENDS ${man}
                     COMMENT "Building ${man}.gz"
                    )
ENDFOREACH(man)


# Build the manuals
ADD_CUSTOM_TARGET(crtn_man ALL DEPENDS ${crtn_man_gz_3} ${crtn_man_gz_7})

# Installation of the manuals
FOREACH(man ${crtn_man_gz_3})
  INSTALL(FILES ${man}
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man3
          PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
          COMPONENT crtn)
ENDFOREACH(man)

FOREACH(man ${crtn_man_gz_7})
  INSTALL(FILES ${man}
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
          PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
          COMPONENT crtn)
ENDFOREACH(man)

