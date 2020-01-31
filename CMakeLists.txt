cmake_minimum_required(VERSION 3.10)
project(openvr-driver-for-diy VERSION 1.0.0)

if(LINUX)
    set(CMAKE_INSTALL_PREFIX $ENV{HOME}/.local/share/Steam/steamapps/common/SteamVR CACHE FORCE)
endif()

add_subdirectory(OpenVR/samples/driver_sample)


add_custom_target(uncrustify
    COMMAND find OpenVR/samples -name '*.cpp' -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \\\\;
    COMMAND find OpenVR/samples -name '*.h' -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \\\\;
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "beautify code"
)

include(FeatureSummary)
feature_summary(WHAT ALL FILENAME ${CMAKE_BINARY_DIR}/features.log APPEND)
