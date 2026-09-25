cmake_minimum_required(VERSION 3.19)
project(QtChatRoomClient LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(QtChatRoomClient
    WIN32 MACOSX_BUNDLE
    main.cpp
    widget.cpp
    widget.h
    widget.ui
    protocol.h
    user.h
    qchatclient.h qchatclient.cpp
)

target_link_libraries(QtChatRoomClient
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
)

install(TARGETS QtChatRoomClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QtChatRoomClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
