#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>
#include <QLabel>
#include "../controller/ClinicManager.h"
#include "PatientPanel.h"
#include "AppointmentPanel.h"
#include "QueuePanel.h"
#include "SearchPanel.h"
#include "ReportsPanel.h"

namespace scapms::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void captureAllScreenshots(const QString& outputDir);

private slots:
    void onNewDatabase();
    void onSaveData();
    void onLoadData();
    void onUndoAction();
    void onRefreshAllPanels();
    void onTimerTick();
    void onCaptureScreenshotsClicked();
    void onAbout();

private:
    void createMenusAndToolbars();
    void setupStatusBar();
    void setupTimers();

    controller::ClinicManager m_manager;

    QTabWidget* m_tabWidget{nullptr};
    PatientPanel* m_patientPanel{nullptr};
    AppointmentPanel* m_appointmentPanel{nullptr};
    QueuePanel* m_queuePanel{nullptr};
    SearchPanel* m_searchPanel{nullptr};
    ReportsPanel* m_reportsPanel{nullptr};

    QLabel* m_statusLabel{nullptr};
    QLabel* m_queueStatusLabel{nullptr};
    QLabel* m_timerStatusLabel{nullptr};

    QTimer* m_refreshTimer{nullptr};
};

} // namespace scapms::gui
