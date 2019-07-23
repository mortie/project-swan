add_library(core.mod SHARED
	src/main.cc
	src/WGDefault.cc
	src/entities/EntPlayer.cc)
set_target_properties(core.mod PROPERTIES OUTPUT_NAME mod PREFIX "")
target_link_libraries(core.mod libswan)
file(COPY assets DESTINATION .)
