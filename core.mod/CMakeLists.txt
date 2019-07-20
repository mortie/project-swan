add_library(core.mod SHARED src/mod.cc)
set_target_properties(core.mod PROPERTIES OUTPUT_NAME mod PREFIX "")
file(COPY assets DESTINATION .)
