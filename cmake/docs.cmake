if(NOT GLVM_BUILD_DOCS)
  return()
endif()

find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)

if(DOXYGEN_VERSION VERSION_LESS "1.9.5")
  message(WARNING "Doxygen >= 1.9.5 recommended for doxygen-awesome-css (found ${DOXYGEN_VERSION})")
endif()

CPMAddPackage(
  NAME doxygen_awesome_css
  GITHUB_REPOSITORY jothepro/doxygen-awesome-css
  GIT_TAG v2.3.4
)

set(DOXYGEN_AWESOME_DIR "${doxygen_awesome_css_SOURCE_DIR}")

if(NOT DOXYGEN_AWESOME_DIR OR NOT EXISTS "${DOXYGEN_AWESOME_DIR}")
  message(FATAL_ERROR
    "doxygen-awesome-css not found at: '${DOXYGEN_AWESOME_DIR}'\n"
    "Check that CPMAddPackage set doxygen_awesome_css_SOURCE_DIR correctly."
  )
endif()

set(DOXYGEN_OUTPUT_DIR  "${CMAKE_BINARY_DIR}/docs")

set(_DOX_INPUT_LIST  "")
set(_DOX_STRIP_LIST  "")

file(GLOB _CRATE_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/crates/*")
foreach(_DIR IN LISTS _CRATE_DIRS)
  if(IS_DIRECTORY "${_DIR}/include")
    list(APPEND _DOX_INPUT_LIST  "${_DIR}/include")
    list(APPEND _DOX_STRIP_LIST  "${_DIR}/include")
  endif()
endforeach()

file(GLOB _ROOT_MD "${CMAKE_SOURCE_DIR}/*.md")
list(APPEND _DOX_INPUT_LIST ${_ROOT_MD})

if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")
  list(APPEND _DOX_INPUT_LIST "${CMAKE_SOURCE_DIR}/docs")
endif()

list(JOIN _DOX_INPUT_LIST " \\\n" DOXYGEN_INPUT)
list(JOIN _DOX_STRIP_LIST " \\\n" DOXYGEN_STRIP_INC)

set(_DOX_PREDEFINED "")
foreach(_feat IN LISTS _GLVM_ALL_FEATURES)
  list(APPEND _DOX_PREDEFINED "GLVM_FEATURE_${_feat}=1")
endforeach()
list(JOIN _DOX_PREDEFINED " " DOXYGEN_PREDEFINED)

if(DOXYGEN_DOT_FOUND)
  set(DOXYGEN_HAVE_DOT "YES")
else()
  set(DOXYGEN_HAVE_DOT "NO")
endif()

set(DOXYGEN_HEADER_FILE "")
set(DOXYGEN_FOOTER_FILE "")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/docs")

set(_HDR "${CMAKE_BINARY_DIR}/docs/header.html")
set(_FTR "${CMAKE_BINARY_DIR}/docs/footer.html")
set(_CSS "${CMAKE_BINARY_DIR}/docs/default.css")

execute_process(
  COMMAND "${DOXYGEN_EXECUTABLE}" -w html "${_HDR}" "${_FTR}" "${_CSS}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE _HDR_OK
  OUTPUT_QUIET
  ERROR_QUIET
)

if(_HDR_OK EQUAL 0 AND EXISTS "${_HDR}")
  file(READ "${_HDR}" _HDR_CONTENT)
  string(CONCAT _INJECT
    "<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-darkmode-toggle.js\"></script>\n"
    "<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-fragment-copy-button.js\"></script>\n"
    "<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-paragraph-link.js\"></script>\n"
    "<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-interactive-toc.js\"></script>\n"
    "<script type=\"text/javascript\">\n"
    "    DoxygenAwesomeDarkModeToggle.init()\n"
    "    DoxygenAwesomeFragmentCopyButton.init()\n"
    "    DoxygenAwesomeParagraphLink.init()\n"
    "    DoxygenAwesomeInteractiveToc.init()\n"
    "</script>\n"
    "</head>"
  )
  string(REPLACE "</head>" "${_INJECT}" _HDR_CONTENT "${_HDR_CONTENT}")
  file(WRITE "${_HDR}" "${_HDR_CONTENT}")
  file(REMOVE "${_CSS}")
  set(DOXYGEN_HEADER_FILE "${_HDR}")
  set(DOXYGEN_FOOTER_FILE "${_FTR}")
  message(STATUS "Docs: header with dark-mode toggle generated")
else()
  message(WARNING "Docs: failed to generate header; dark-mode toggle unavailable")
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
  "${CMAKE_BINARY_DIR}/Doxyfile"
  @ONLY
)

add_custom_target(docs
  COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Generating docs: ${DOXYGEN_OUTPUT_DIR}/html/index.html"
  VERBATIM
)

message(STATUS "Docs: cmake --build <dir> -t docs")
