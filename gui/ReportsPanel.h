#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include "../controller/ClinicManager.h"

namespace scapms::gui {

class ReportsPanel : public QWidget {
    Q_OBJECT
public:
    explicit ReportsPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refreshReports();

private slots:
    void onExportRosterCsv();
    void onExportApptsCsv();

private:
    void setupUI();

    controller::ClinicManager& m_manager;

    QTableWidget* m_frequentTable{nullptr};
    QTableWidget* m_pendingTable{nullptr};

    QLabel* m_distGeneralLabel{nullptr};
    QLabel* m_distEmergencyLabel{nullptr};
    QLabel* m_distFollowUpLabel{nullptr};

    QLabel* m_summaryTotalLabel{nullptr};
    QLabel* m_summaryCompletedLabel{nullptr};
    QLabel* m_summaryPendingLabel{nullptr};
    QLabel* m_summaryCancelledLabel{nullptr};

    QPushButton* m_btnExportRoster{nullptr};
    QPushButton* m_btnExportAppts{nullptr};
    QPushButton* m_btnRefresh{nullptr};
};

} // namespace scapms::gui
