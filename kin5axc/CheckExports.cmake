# Does the shared library let anything out but its C interface? Run by ctest as kin5axcExports on the
# platforms with nm; -DLIB=<the shared object> says what to look at.
if(NOT DEFINED LIB)
  message(FATAL_ERROR "CheckExports.cmake needs -DLIB=<the shared library to look at>")
endif()

find_program(NM_PROGRAM nm)
if(NOT NM_PROGRAM)
  message(STATUS "nm is not here, so the exported symbols cannot be checked")
  return()
endif()

execute_process(COMMAND ${NM_PROGRAM} -D --defined-only ${LIB}
  OUTPUT_VARIABLE sOutput RESULT_VARIABLE nResult OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT nResult EQUAL 0)
  message(FATAL_ERROR "nm could not read ${LIB}")
endif()

string(REPLACE "\n" ";" vLines "${sOutput}")
set(vStrangers "")
set(nExported 0)
foreach(sLine IN LISTS vLines)
  # "<address> <type> <name>", and only the names of the global text and data symbols matter
  if(sLine MATCHES "^[0-9a-fA-F]+ +[A-Za-z] +(.+)$")
    set(sName "${CMAKE_MATCH_1}")
    math(EXPR nExported "${nExported}+1")
    if(NOT sName MATCHES "^k5_")
      list(APPEND vStrangers "${sName}")
    endif()
  endif()
endforeach()

if(nExported EQUAL 0)
  message(FATAL_ERROR "nm found no exported symbol at all in ${LIB}: the check is not looking at the right thing")
endif()
if(vStrangers)
  list(LENGTH vStrangers nStrangers)
  string(REPLACE ";" "\n  " sStrangers "${vStrangers}")
  message(FATAL_ERROR
    "${nStrangers} of the ${nExported} exported symbols of ${LIB} are not part of the C interface:\n  ${sStrangers}")
endif()
message(STATUS "${nExported} exported symbols, all of them k5_*")
