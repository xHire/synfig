target_sources(synfig
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/surfacememoryreadwrapper.cpp"
)

include(${CMAKE_CURRENT_LIST_DIR}/optimizer/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt)
