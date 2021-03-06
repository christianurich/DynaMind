FILE(GLOB PYTHON_CPPS *.cpp)
FILE(GLOB PYTHON_HEADER *.h)

MESSAGE(STATUS SWIG ${CMAKE_BINARY_DIR})
SET(CMAKE_SWIG_OUTDIR ${OUTPUT_DIR})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(../DMToolboxes)
INCLUDE(${SWIG_USE_FILE})

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
        MESSAGE(STATUS "SWIG Warnings enabled")
ELSE()
        MESSAGE(STATUS "SWIG Warnings disabled")
        SET(SWIG_ARGS -w501,473,509,517,401)
ENDIF()

IF(NOT EXISTS swigruntime.h)
    EXECUTE_PROCESS(COMMAND ${SWIG_EXECUTABLE} -python ${SWIG_ARGS} -c++ -threads -external-runtime swigruntime.h)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(CMAKE_SWIG_FLAGS "")
SET_SOURCE_FILES_PROPERTIES(pydynamind.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(pydynamind.i PROPERTIES SWIG_FLAGS "-threads")


SET_SOURCE_FILES_PROPERTIES(pydmtoolbox.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(pydmtoolbox.i PROPERTIES SWIG_FLAGS "-threads")

SWIG_ADD_MODULE(pydynamind python pydynamind.i dmpythonenv.cpp)
SWIG_LINK_LIBRARIES(pydynamind dynamindcore ${PYTHON_LIBRARIES})

SWIG_ADD_MODULE(pydmtoolbox python pydmtoolbox.i)
SWIG_LINK_LIBRARIES(pydmtoolbox dynamindtoolbox dynamindcore)

#ADD_DEPENDENCIES(pydynamind.i dynamindcore dynamindtoolbox )
#ADD_DEPENDENCIES(pydmtoolbox.i dynamindcore )
