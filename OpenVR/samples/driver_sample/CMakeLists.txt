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

add_definitions(-DDRIVER_SAMPLE_EXPORTS)
SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "")

target_link_libraries(${TARGET_NAME}
  ${OPENVR_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

