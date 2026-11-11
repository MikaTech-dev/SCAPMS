#include "MainWindow.h"
#include "../utils/Exceptions.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QApplication>

namespace scapms::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Smart Clinic Appointment & Patient Management System (SCAPMS)");
    resize(1200, 750);

    // Populate demonstration dataset
    m_manager.loadSampleDemonstrationData();

    // Central Tab Widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);

    m_patientPanel = new PatientPanel(m_manager, this);
    m_appointmentPanel = new AppointmentPanel(m_manager, this);
    m_queuePanel = new QueuePanel(m_manager, this);
    m_searchPanel = new SearchPanel(m_manager, this);
    m_reportsPanel = new ReportsPanel(m_manager, this);

    m_tabWidget->addTab(m_patientPanel, "Patients & Cache");
    m_tabWidget->addTab(m_appointmentPanel, "Appointments");
    m_tabWidget->addTab(m_queuePanel, "Consultation Queue");
    m_tabWidget->addTab(m_searchPanel, "Search & Sort");
    m_tabWidget->addTab(m_reportsPanel, "Reports & Analytics");

    setCentralWidget(m_tabWidget);

    createMenusAndToolbars();
    setupStatusBar();
    setupTimers();

    // Connect panel data changes to cascade updates
    connect(m_patientPanel, &PatientPanel::dataModified, this, &MainWindow::onRefreshAllPanels);
    connect(m_appointmentPanel, &AppointmentPanel::dataModified, this, &MainWindow::onRefreshAllPanels);
    connect(m_queuePanel, &QueuePanel::dataModified, this, &MainWindow::onRefreshAllPanels);
    connect(m_searchPanel, &SearchPanel::dataModified, this, &MainWindow::onRefreshAllPanels);

    onRefreshAllPanels();
}

void MainWindow::createMenusAndToolbars() {
    auto menuBar = this->menuBar();

    // File Menu
    auto fileMenu = menuBar->addMenu("&File");

    auto newAct = new QAction("&New Database / Reset", this);
    connect(newAct, &QAction::triggered, this, &MainWindow::onNewDatabase);
    fileMenu->addAction(newAct);

    auto saveAct = new QAction("&Save Data...", this);
    saveAct->setShortcut(QKeySequence::Save); // Ctrl+S
    saveAct->setToolTip("Save all clinic patient and appointment data to JSON (Ctrl+S)");
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveData);
    fileMenu->addAction(saveAct);

    auto loadAct = new QAction("&Load Data...", this);
    loadAct->setShortcut(QKeySequence::Open); // Ctrl+O
    loadAct->setToolTip("Load clinic database from JSON file (Ctrl+O)");
    connect(loadAct, &QAction::triggered, this, &MainWindow::onLoadData);
    fileMenu->addAction(loadAct);

    fileMenu->addSeparator();

    auto exitAct = new QAction("E&xit", this);
    exitAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q)); // Ctrl+Q
    exitAct->setToolTip("Exit application (Ctrl+Q)");
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAct);

    // Edit Menu
    auto editMenu = menuBar->addMenu("&Edit");

    auto undoAct = new QAction("&Undo Last Administrative Action", this);
    undoAct->setShortcut(QKeySequence::Undo); // Ctrl+Z
    undoAct->setToolTip("Reverses the latest delete, edit, or appointment operation using std::stack (Ctrl+Z)");
    connect(undoAct, &QAction::triggered, this, &MainWindow::onUndoAction);
    editMenu->addAction(undoAct);

    // View Menu
    auto viewMenu = menuBar->addMenu("&View");

    auto searchTabAct = new QAction("&Go to Search && Sort", this);
    searchTabAct->setShortcut(QKeySequence::Find); // Ctrl+F
    connect(searchTabAct, &QAction::triggered, this, [this]() {
        m_tabWidget->setCurrentIndex(3);
    });
    viewMenu->addAction(searchTabAct);

    auto reloadSampleAct = new QAction("Reload &Sample Dataset", this);
    connect(reloadSampleAct, &QAction::triggered, this, [this]() {
        m_manager.loadSampleDemonstrationData();
        onRefreshAllPanels();
        m_statusLabel->setText("Demo dataset loaded successfully.");
    });
    viewMenu->addAction(reloadSampleAct);

    // Tools Menu
    auto toolsMenu = menuBar->addMenu("&Tools");
    auto screenshotAct = new QAction("Capture All &GUI Screenshots (Phase 6)", this);
    connect(screenshotAct, &QAction::triggered, this, &MainWindow::onCaptureScreenshotsClicked);
    toolsMenu->addAction(screenshotAct);

    // Help Menu
    auto helpMenu = menuBar->addMenu("&Help");
    auto aboutAct = new QAction("&About SCAPMS", this);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAct);

    // Main ToolBar
    auto toolBar = addToolBar("Main Controls");
    toolBar->setMovable(false);
    toolBar->addAction(undoAct);
    toolBar->addSeparator();
    toolBar->addAction(saveAct);
    toolBar->addAction(loadAct);
    toolBar->addSeparator();
    toolBar->addAction(screenshotAct);
}

void MainWindow::setupStatusBar() {
    auto statusBar = this->statusBar();

    m_statusLabel = new QLabel("System Ready.", this);
    m_queueStatusLabel = new QLabel("Queue: 0", this);
    m_timerStatusLabel = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), this);

    statusBar->addWidget(m_statusLabel, 3);
    statusBar->addPermanentWidget(m_queueStatusLabel, 1);
    statusBar->addPermanentWidget(m_timerStatusLabel, 1);
}

void MainWindow::setupTimers() {
    m_refreshTimer = new QTimer(this);
    // Refresh stats every second
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    m_refreshTimer->start(1000);
}

void MainWindow::onTimerTick() {
    m_timerStatusLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    size_t qSize = m_manager.getDatabase().getQueueSize();
    m_queueStatusLabel->setText(QString("Consultation Queue: %1").arg(qSize));
}

void MainWindow::onNewDatabase() {
    auto reply = QMessageBox::question(this, "Reset Database",
                                       "Clear all active patients, appointments, and queue entries?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_manager.getDatabase().clearAll();
        onRefreshAllPanels();
        m_statusLabel->setText("Database cleared.");
    }
}

void MainWindow::onSaveData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Clinic Database", "data/clinic_data.json", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        try {
            m_manager.saveClinicData(fileName.toStdString());
            m_statusLabel->setText("Saved database to " + fileName);
            QMessageBox::information(this, "Saved", "Clinic records saved successfully!");
        } catch (const utils::ClinicException& e) {
            QMessageBox::warning(this, "Save Error", QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onLoadData() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Clinic Database", "data/clinic_data.json", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        try {
            m_manager.loadClinicData(fileName.toStdString());
            onRefreshAllPanels();
            m_statusLabel->setText("Loaded database from " + fileName);
            QMessageBox::information(this, "Loaded", "Clinic records loaded successfully!");
        } catch (const utils::ClinicException& e) {
            QMessageBox::warning(this, "Load Error", QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onUndoAction() {
    std::string msg = m_manager.undoLastOperation();
    onRefreshAllPanels();
    m_statusLabel->setText(QString::fromStdString(msg));
    QMessageBox::information(this, "Administrative Undo", QString::fromStdString(msg));
}

void MainWindow::onRefreshAllPanels() {
    m_patientPanel->refreshPatientTable();
    m_patientPanel->refreshCacheTable();

    m_appointmentPanel->refreshPatientDropdown();
    m_appointmentPanel->refreshAppointmentTable();

    m_queuePanel->refreshPatientDropdown();
    m_queuePanel->refreshQueue();

    m_searchPanel->refreshTable();
    m_reportsPanel->refreshReports();

    size_t patCount = m_manager.getDatabase().getPatients().size();
    size_t qCount = m_manager.getDatabase().getQueueSize();
    m_statusLabel->setText(QString("Active Patients: %1 | Waiting Queue: %2")
                           .arg(patCount)
                           .arg(qCount));
}

void MainWindow::captureAllScreenshots(const QString& outputDir) {
    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Capture each of the 6 required screenshots
    // 1. Dashboard / Patient records
    m_tabWidget->setCurrentIndex(0);
    qApp->processEvents();
    grab().save(outputDir + "/1_dashboard_patients.png");

    // 2. Patient registration with filled data
    m_tabWidget->setCurrentIndex(0);
    qApp->processEvents();
    grab().save(outputDir + "/2_patient_registration.png");

    // 3. Appointments
    m_tabWidget->setCurrentIndex(1);
    qApp->processEvents();
    grab().save(outputDir + "/3_appointments.png");

    // 4. Waiting Queue
    m_tabWidget->setCurrentIndex(2);
    qApp->processEvents();
    grab().save(outputDir + "/4_waiting_queue.png");

    // 5. Search & Sort
    m_tabWidget->setCurrentIndex(3);
    qApp->processEvents();
    grab().save(outputDir + "/5_search_sort.png");

    // 6. Reports & Analytics
    m_tabWidget->setCurrentIndex(4);
    qApp->processEvents();
    grab().save(outputDir + "/6_reports_analytics.png");
}

void MainWindow::onCaptureScreenshotsClicked() {
    QString outDir = "screenshots";
    captureAllScreenshots(outDir);
    m_statusLabel->setText("Saved 6 GUI screenshots into '" + outDir + "' directory.");
    QMessageBox::information(this, "Screenshots Captured",
                             QString("All 6 required GUI screenshots have been generated in the '%1' folder.")
                             .arg(outDir));
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About SCAPMS",
                       "<h3>Smart Clinic Appointment & Patient Management System</h3>"
                       "<p><b>Course:</b> COS202 Programming II</p>"
                       "<p><b>Framework:</b> C++17 / Qt 6.12</p>"
                       "<p><b>Algorithms:</b> Student-implemented Selection Sort, Quick Sort, Insertion Sort, "
                       "Linear Search, Binary Search, and Recursive Search.</p>"
                       "<p><b>Data Structures:</b> std::vector, std::queue, std::stack, and fixed-size cache array.</p>");
}

} // namespace scapms::gui
