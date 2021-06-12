# get lib info from the git cmds
execute_process(COMMAND git config --get remote.origin.url
    OUTPUT_VARIABLE CMAKE_PROJECT_HOMEPAGE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(FIND "${CMAKE_PROJECT_HOMEPAGE_URL}" ".git" has_git_ext REVERSE)
if(has_git_ext GREATER -1)
    string(SUBSTRING "${CMAKE_PROJECT_HOMEPAGE_URL}" 0 ${has_git_ext} CMAKE_PROJECT_HOMEPAGE_URL)
endif()

# use URL to get repo owner as Contact/Maintainer name
string(REGEX REPLACE "^http[s]?:\/\/github\\.com\/(.+)\/.+$" "\\1" CPACK_PACKAGE_CONTACT "${CMAKE_PROJECT_HOMEPAGE_URL}")

# use URL to get the repo name as the Lib Name. Note that we don't use the folder name for this
string(REGEX REPLACE "^http[s]?:\/\/github\\.com\/.+\/(.+)$" "\\1" LibName "${CMAKE_PROJECT_HOMEPAGE_URL}")

# convert the LibName to lower case
string(TOLOWER ${LibName} LibTargetName)

# have to hard-code the Repo Summary (which is copied from the parent repo's "About" info)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TCP/IP (RF24Ethernet) and RF24Network Gateway")

# Use repo README.md to get the project description
file(READ "README.md" CPACK_PACKAGE_DESCRIPTION)
string(FIND "${CPACK_PACKAGE_DESCRIPTION}" "## RF24Gateway" README_TITLE)
math(EXPR README_TITLE "${README_TITLE} + 15") # compensates for '\n' char
string(FIND "${CPACK_PACKAGE_DESCRIPTION}" "### Documentation" DOC_LINK_TITLE)
math(EXPR DESCRIPTION_LENGTH "${DOC_LINK_TITLE} - ${README_TITLE}")
string(SUBSTRING "${CPACK_PACKAGE_DESCRIPTION}" ${README_TITLE} ${DESCRIPTION_LENGTH} CPACK_PACKAGE_DESCRIPTION)

# parse the version information into pieces.
execute_process(COMMAND git describe --tags OUTPUT_VARIABLE VERSION)
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")

# this is the library version
set(${LibName}_VERSION_MAJOR ${VERSION_MAJOR})
set(${LibName}_VERSION_MINOR ${VERSION_MINOR})
set(${LibName}_VERSION_PATCH ${VERSION_PATCH})
set(${LibName}_VERSION_STRING ${${LibName}_VERSION_MAJOR}.${${LibName}_VERSION_MINOR}.${${LibName}_VERSION_PATCH})

message(STATUS "${LibName} library version: ${${LibName}_VERSION_STRING}")
