include_directories(../../headers)

set(TARGET_NAME driver_sample)

add_library(${TARGET_NAME} MODULE
  basics.cpp
  basics.h
  driverlog.cpp
  driverlog.h
  driver_sample.cpp
  csampledevicedriver.cpp
  csampledevicedriver.h
  cserverdriver_sample.cpp
  cserverdriver_sample.h
  csamplecontrollerdriver.cpp
  csamplecontrollerdriver.h
  cwatchdogdriver_sample.cpp
  cwatchdogdriver_sample.h
)

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "")

target_link_libraries(${TARGET_NAME}
  ${OPENVR_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PREFIX "win")
else()
    set(PREFIX "linux")
endif()

if(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(ARCH 64)
else()
    set(ARCH 64)
endif()

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION drivers/sample/bin/${PREFIX}${ARCH}
    LIBRARY DESTINATION drivers/sample/bin/${PREFIX}${ARCH}
)

install(FILES default.vrsettings DESTINATION drivers/sample/resources/settings)
