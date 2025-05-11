project(UeApplication)
cmake_minimum_required(VERSION 3.12)

aux_source_directory(. SRC_LIST)
aux_source_directory(Ports SRC_LIST)
aux_source_directory(States SRC_LIST)

# Add SmsDb.cpp explicitly
set(SRC_LIST ${SRC_LIST} SmsDb.cpp)

# Add our new state files explicitly
set(SRC_LIST ${SRC_LIST} States/ReceivingCallState.cpp States/TalkingState.cpp)

add_library(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} Common)
