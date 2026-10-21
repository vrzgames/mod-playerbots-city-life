# Optional City Life cosmetics use the local mod-transmog API only inside the common static modules library.
# Missing, disabled, unpatched or dynamically linked providers keep City Life's real BiS equipment unchanged.
if(NOT TARGET modules OR
    NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../mod-transmog/src/Transmogrification.h")
  return()
endif()
if(";${DISABLED_AC_MODULES};" MATCHES ";mod-transmog;" OR
    ";${DISABLED_AC_MODULES};" MATCHES ";mod-playerbots-city-life;")
  return()
endif()

set(CITY_LIFE_TRANSMOG_LINKAGE "${MODULE_MOD-TRANSMOG}")
set(CITY_LIFE_MODULE_LINKAGE "${MODULE_MOD-PLAYERBOTS-CITY-LIFE}")
foreach(CITY_LIFE_LINKAGE_VARIABLE CITY_LIFE_TRANSMOG_LINKAGE CITY_LIFE_MODULE_LINKAGE)
  if("${${CITY_LIFE_LINKAGE_VARIABLE}}" STREQUAL "default")
    if(MODULES MATCHES "static")
      set(${CITY_LIFE_LINKAGE_VARIABLE} "static")
    else()
      set(${CITY_LIFE_LINKAGE_VARIABLE} "disabled")
    endif()
  endif()
endforeach()

if(CITY_LIFE_TRANSMOG_LINKAGE STREQUAL "static" AND CITY_LIFE_MODULE_LINKAGE STREQUAL "static")
  target_compile_definitions(modules PRIVATE CITY_LIFE_TRANSMOG_BRIDGE=1)
endif()
