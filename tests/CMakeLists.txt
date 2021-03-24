INCLUDE_DIRECTORIES(../include)

if (CHECK_FOUND)

  MESSAGE("CHECK_INCLUDE_DIR=" "${CHECK_INCLUDE_DIR}")
  MESSAGE("CHECK_LIBRARIES=" "${CHECK_LIBRARIES}")

  add_executable(check_all check_all.c
                           check_crtn_errcodes.c
                           check_crtn_api.c
                           check_crtn_prog.c
                           check_util.c
                           )

  target_link_libraries(check_all ${CHECK_LIBRARIES} crtn pthread)

  add_test(crtn_tests check_all)

  #
  # Test coverage
  #
  if (CMAKE_COVERAGE)

    setup_target_for_coverage(NAME all_coverage
                              SRCDIR .
                              EXCLUDES '/usr/include/*'  '/*/tests/*'
                              EXECUTABLE tests/check_all
                              DEPENDENCIES crtn)
  endif()

endif(CHECK_FOUND)

#
# Examples and utilities related to timer algos
#

ADD_EXECUTABLE(tcrtn tcrtn.c)
TARGET_LINK_LIBRARIES(tcrtn crtn)

ADD_EXECUTABLE(mywc wc.c)
TARGET_LINK_LIBRARIES(mywc crtn)

ADD_EXECUTABLE(mywc1 wc1.c)
TARGET_LINK_LIBRARIES(mywc1 crtn)

if (${HAVE_CRTN_MBX} STREQUAL ON)
  ADD_EXECUTABLE(mbx mbx.c)
  TARGET_LINK_LIBRARIES(mbx crtn)

  ADD_EXECUTABLE(mywc2 wc2.c)
  TARGET_LINK_LIBRARIES(mywc2 crtn)
endif()

ADD_EXECUTABLE(mywc3 wc3.c)
TARGET_LINK_LIBRARIES(mywc3 crtn)

ADD_EXECUTABLE(mywc4 wc4.c)
TARGET_LINK_LIBRARIES(mywc4 crtn)

if (${HAVE_CRTN_SEM} STREQUAL ON)
  ADD_EXECUTABLE(mywc5 wc5.c)
  TARGET_LINK_LIBRARIES(mywc5 crtn)
endif()

ADD_EXECUTABLE(mywc6 wc6.c)
TARGET_LINK_LIBRARIES(mywc6 crtn)

ADD_EXECUTABLE(mywc7 wc7.c)
TARGET_LINK_LIBRARIES(mywc7 crtn)

ADD_EXECUTABLE(fibonacci fibonacci.c)
TARGET_LINK_LIBRARIES(fibonacci crtn)

ADD_EXECUTABLE(switch_ctx2 switch_ctx2.c)
TARGET_LINK_LIBRARIES(switch_ctx2 crtn)

ADD_EXECUTABLE(switch_ctx3 switch_ctx3.c)
TARGET_LINK_LIBRARIES(switch_ctx3 crtn)

ADD_EXECUTABLE(sig sig.c)
TARGET_LINK_LIBRARIES(sig crtn)


