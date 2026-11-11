#include <QApplication>
#include <QStyleFactory>
#include <iostream>
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("SCAPMS");
    app.setApplicationDisplayName("Smart Clinic Appointment & Patient Management System");

    // Set modern Fusion style
    app.setStyle(QStyleFactory::create("Fusion"));

    scapms::gui::MainWindow mainWindow;
    mainWindow.show();

    // Check for CLI argument --capture-screenshots
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--capture-screenshots") {
            std::cout << "[SCAPMS] Capturing 6 submission screenshots..." << std::endl;
            mainWindow.captureAllScreenshots("screenshots");
            std::cout << "[SCAPMS] Screenshots saved successfully to screenshots/" << std::endl;
            return 0;
        }
    }

    return app.exec();
}
