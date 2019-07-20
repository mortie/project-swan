add_library(core.mod SHARED src/mod.cc)
set_target_properties(core.mod PROPERTIES OUTPUT_NAME mod PREFIX "")
target_link_libraries(core.mod libswan)
file(COPY assets DESTINATION .)
