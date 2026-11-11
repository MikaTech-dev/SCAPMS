cmake_minimum_required(VERSION 3.22)
project(SCAPMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automatic Qt MOC, UIC, RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    # Main Entry
    main.cpp

    # Utilities
    utils/Exceptions.h
    utils/IDGenerator.h
    utils/IDGenerator.cpp
    utils/FileHandler.h
    utils/FileHandler.cpp

    # Domain Models
    model/Appointable.h
    model/Appointment.h
    model/Appointment.cpp
    model/PatientRecord.h
    model/PatientRecord.cpp
    model/GeneralPatient.h
    model/GeneralPatient.cpp
    model/EmergencyPatient.h
    model/EmergencyPatient.cpp
    model/FollowUpPatient.h
    model/FollowUpPatient.cpp
    model/ClinicDatabase.h
    model/ClinicDatabase.cpp

    # Business Controllers & Algorithms
    controller/SearchEngine.h
    controller/SearchEngine.cpp
    controller/AppointmentController.h
    controller/AppointmentController.cpp
    controller/ClinicManager.h
    controller/ClinicManager.cpp

    # GUI Presentation
    gui/PatientPanel.h
    gui/PatientPanel.cpp
    gui/AppointmentPanel.h
    gui/AppointmentPanel.cpp
    gui/QueuePanel.h
    gui/QueuePanel.cpp
    gui/SearchPanel.h
    gui/SearchPanel.cpp
    gui/ReportsPanel.h
    gui/ReportsPanel.cpp
    gui/MainWindow.h
    gui/MainWindow.cpp
)

add_executable(SCAPMS ${SOURCES})

target_include_directories(SCAPMS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SCAPMS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
