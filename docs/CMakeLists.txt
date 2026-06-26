# ---------------------------------------------------------------------------- #
# Documents
# ---------------------------------------------------------------------------- #
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
  )

  add_custom_target(doc
    COMMAND 
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile &> doxygen.log
    WORKING_DIRECTORY ${CMAKE_CURRENT_BIN_DIR}
    COMMENT "Generate ILAng API documentation with Doxygen" VERBATIM
  )

  # ---- Focused R-Map (refinement map) documentation ----
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.rmap.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.rmap
    @ONLY
  )

  add_custom_target(rmap-doc
    COMMAND
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.rmap
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generate ILAng R-Map documentation with Doxygen" VERBATIM
  )

endif()

