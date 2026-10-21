# mod-playerbots-city-life
# AzerothCore discovers module source files and conf/*.conf.dist automatically.

# The PvP implementation is now part of City Life. The old standalone sources
# would define the same manager and scripts a second time.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../mod-playerbots-pvp-life/src/PvPLifeMgr.cpp")
  message(FATAL_ERROR
    "PvP Life is included in City Life. Move modules/mod-playerbots-pvp-life "
    "outside the modules directory before configuring. Keep your existing "
    "mod_playerbots_pvp_life.conf and WORLD database tables.")
endif()

