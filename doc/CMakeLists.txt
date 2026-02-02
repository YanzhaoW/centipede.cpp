find_package(Doxygen REQUIRED COMPONENTS dot)

include(FetchContent)
FetchContent_Declare(
    doxygen-awesome-css
    URL
        https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)

# Save the location the files were cloned into
# This allows us to get the path to doxygen-awesome.css
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

# # Generate the Doxyfile
# set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
# set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# set(DOXYGEN_GENERATE_XML YES)

set(DOXYGEN_PROJECT_NAME "Centipede")
set(DOXYGEN_PROJECT_BRIEF "Centipede program")

# set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_SOURCE_BROWSER YES)
# set(DOXYGEN_UML_LOOK YES)
set(DOXYGEN_TEMPLATE_RELATIONS YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_CLASS_GRAPH NO)
set(DOXYGEN_COLLABORATION_GRAPH NO)
set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
set(DOXYGEN_USE_MATHJAX YES)
set(DOXYGEN_MATHJAX_VERSION "MathJax_3")
set(DOXYGEN_MATHJAX_CODEFILE "${PROJECT_SOURCE_DIR}/util/mathjax-config.js")
set(DOXYGEN_RECURSIVE YES)
set(DOXYGEN_IMPLICIT_DIR_DOCS NO)
set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 2)
set(DOXYGEN_TOC_INCLUDE_HEADINGS 6)
set(DOXYGEN_MARKDOWN_SUPPORT YES)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/doc/main.md")
set(DOXYGEN_IMAGE_PATH "${PROJECT_SOURCE_DIR}/doc/pics")
# set(DOXYGEN_EXAMPLE_PATH "${PROJECT_SOURCE_DIR}/neuland/docs/figs")
set(DOXYGEN_PYTHON_DOCSTRING NO)

# Code
set(DOXYGEN_REPEAT_BRIEF NO)
set(DOXYGEN_EXTRACT_ALL NO)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_PRIV_VIRTUAL NO)
set(DOXYGEN_EXTRACT_PACKAGE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_HIDE_SCOPE_NAMES NO)
# set(DOXYGEN_EXCLUDE_SYMBOLS "centipede")

set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
set(DOXYGEN_HIDE_UNDOC_CLASSES NO)
set(DOXYGEN_HIDE_UNDOC_NAMESPACES NO)

# For doxygen-awesome
set(DOXYGEN_HTML_EXTRA_FILES
    "${AWESOME_CSS_DIR}/doxygen-awesome-darkmode-toggle.js"
    "${AWESOME_CSS_DIR}/doxygen-awesome-interactive-toc.js"
)
set(DOXYGEN_HTML_EXTRA_STYLESHEET
    "${AWESOME_CSS_DIR}/doxygen-awesome.css"
    # "${AWESOME_CSS_DIR}/doxygen-awesome.css"
    # "${AWESOME_CSS_DIR}/doxygen-awesome-darkmode-toggle.css"
    # "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only.css"
    # "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
)
set(DOXYGEN_HTML_HEADER "${PROJECT_SOURCE_DIR}/util/doxygen_header.html")
set(DOXYGEN_DISABLE_INDEX NO)
set(DOXYGEN_FULL_SIDEBAR NO)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_HTML_COLORSTYLE LIGHT)
# set(DOXYGEN_EXCLUDE_PATTERNS "ext_h101_*" "EXT_STR_h101_*")
set(DOXYGEN_PAGE_OUTLINE_PANEL NO)
set(DOXYGEN_WARN_AS_ERROR "FAIL_ON_WARNINGS")

# set(DOXYGEN_FILE_PATTERNS "readme.md")
# set(DOXYGEN_EXTENSION_MAPPING "md=md")

# file(GLOB DataSourceFiles ${PROJECT_SOURCE_DIR}/r3bdata/*.cxx)
# file(GLOB DataHeaderFiles ${PROJECT_SOURCE_DIR}/r3bdata/*.h)

doxygen_add_docs(
    doxygen
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/doc
    ${PROJECT_SOURCE_DIR}/test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate doxygen documentation"
)
