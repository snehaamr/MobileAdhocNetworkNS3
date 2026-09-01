# Used when this directory is copied or symlinked into an ns-3 tree as
# scratch/mobile-adhoc-tree (ns-3.40+ / CMake).
#
#   cd ns-3-dev
#   ln -s /path/to/MobileAdhocNetworkNS3 scratch/mobile-adhoc-tree
#   ./ns3 configure --enable-examples
#   ./ns3 run "mobile-adhoc-tree --nWifi=40 --bufferSize=10"

set(source_files
    main.cc
    MobileAdhocTree.cc
    TreeStructureApp.cc
    WiFiNetworkSetup.cc
)

set(header_files
    MobileAdhocTree.h
    TreeStructureApp.h
    WiFiNetworkSetup.h
)

get_filename_component(scratch_dir_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

build_exec(
    EXECNAME mobile-adhoc-tree
    SOURCE_FILES ${source_files}
    HEADER_FILES ${header_files}
    LIBRARIES_TO_LINK ${libcore}
                      ${libnetwork}
                      ${libinternet}
                      ${libwifi}
                      ${libmobility}
                      ${libapplications}
                      ${libflow-monitor}
    EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/${scratch_dir_name}
)
