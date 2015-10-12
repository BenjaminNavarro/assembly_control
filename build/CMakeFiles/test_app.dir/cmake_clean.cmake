file(REMOVE_RECURSE
  "../bin/test_app.pdb"
  "../bin/test_app"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/test_app.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
