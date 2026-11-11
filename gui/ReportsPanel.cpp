#include "ReportsPanel.h"
#include "../utils/FileHandler.h"
#include "../utils/Exceptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

namespace scapms::gui {

ReportsPanel::ReportsPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager) {
    setupUI();
    refreshReports();
}

void ReportsPanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);

    // Header Controls
    auto headerLayout = new QHBoxLayout();
    auto titleLabel = new QLabel("Clinic Analytical Intelligence & Summary Reports", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a237e;");

    m_btnRefresh = new QPushButton("Refresh Reports", this);
    m_btnRefresh->setStyleSheet("background-color: #0277bd; color: white; font-weight: bold; padding: 6px 12px;");

    m_btnExportRoster = new QPushButton("Export Patients CSV", this);
    m_btnExportRoster->setStyleSheet("background-color: #388e3c; color: white; font-weight: bold; padding: 6px 12px;");

    m_btnExportAppts = new QPushButton("Export Appointments CSV", this);
    m_btnExportAppts->setStyleSheet("background-color: #558b2f; color: white; font-weight: bold; padding: 6px 12px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_btnRefresh);
    headerLayout->addWidget(m_btnExportRoster);
    headerLayout->addWidget(m_btnExportAppts);
    mainLayout->addLayout(headerLayout);

    // Summary Metric Cards (Category Distribution + Daily Summary)
    auto cardsLayout = new QHBoxLayout();

    // 1. Category Distribution Box
    auto distBox = new QGroupBox("Report 3: Patient Category Distribution", this);
    auto distLayout = new QGridLayout(distBox);

    m_distGeneralLabel = new QLabel("General Patients: 0", this);
    m_distEmergencyLabel = new QLabel("Emergency Patients: 0", this);
    m_distFollowUpLabel = new QLabel("Follow-Up Patients: 0", this);

    m_distGeneralLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #2e7d32;");
    m_distEmergencyLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #c62828;");
    m_distFollowUpLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #1565c0;");

    distLayout->addWidget(m_distGeneralLabel, 0, 0);
    distLayout->addWidget(m_distEmergencyLabel, 0, 1);
    distLayout->addWidget(m_distFollowUpLabel, 0, 2);
    cardsLayout->addWidget(distBox, 1);

    // 2. Daily Appointment Summary Box
    auto apptBox = new QGroupBox("Report 4: Daily Appointment Summary", this);
    auto apptLayout = new QGridLayout(apptBox);

    m_summaryTotalLabel = new QLabel("Total: 0", this);
    m_summaryCompletedLabel = new QLabel("Completed: 0", this);
    m_summaryPendingLabel = new QLabel("Pending: 0", this);
    m_summaryCancelledLabel = new QLabel("Cancelled: 0", this);

    m_summaryTotalLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #37474f;");
    m_summaryCompletedLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #2e7d32;");
    m_summaryPendingLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #f57f17;");
    m_summaryCancelledLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #c62828;");

    apptLayout->addWidget(m_summaryTotalLabel, 0, 0);
    apptLayout->addWidget(m_summaryCompletedLabel, 0, 1);
    apptLayout->addWidget(m_summaryPendingLabel, 0, 2);
    apptLayout->addWidget(m_summaryCancelledLabel, 0, 3);
    cardsLayout->addWidget(apptBox, 1);

    mainLayout->addLayout(cardsLayout);

    // Split Tables: Frequent Patients & Pending Patients
    auto tablesLayout = new QHBoxLayout();

    // Report 1: Most Frequently Attended Patients
    auto frequentBox = new QGroupBox("Report 1: Most Frequently Attended Patients (Ranked by Completed Appts)", this);
    auto frequentLayout = new QVBoxLayout(frequentBox);
    m_frequentTable = new QTableWidget(this);
    m_frequentTable->setColumnCount(4);
    m_frequentTable->setHorizontalHeaderLabels({"Rank", "Patient ID", "Name", "Completed Appts"});
    m_frequentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_frequentTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_frequentTable->setAlternatingRowColors(true);
    frequentLayout->addWidget(m_frequentTable);
    tablesLayout->addWidget(frequentBox, 1);

    // Report 2: Patients with Pending Appointments
    auto pendingBox = new QGroupBox("Report 2: Patients with Active Pending Appointments", this);
    auto pendingLayout = new QVBoxLayout(pendingBox);
    m_pendingTable = new QTableWidget(this);
    m_pendingTable->setColumnCount(4);
    m_pendingTable->setHorizontalHeaderLabels({"Patient ID", "Name", "Category", "Pending Count"});
    m_pendingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_pendingTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_pendingTable->setAlternatingRowColors(true);
    pendingLayout->addWidget(m_pendingTable);
    tablesLayout->addWidget(pendingBox, 1);

    mainLayout->addLayout(tablesLayout);

    connect(m_btnRefresh, &QPushButton::clicked, this, &ReportsPanel::refreshReports);
    connect(m_btnExportRoster, &QPushButton::clicked, this, &ReportsPanel::onExportRosterCsv);
    connect(m_btnExportAppts, &QPushButton::clicked, this, &ReportsPanel::onExportApptsCsv);
}

void ReportsPanel::refreshReports() {
    // 1. Most Attended Patients
    auto frequentList = m_manager.getMostAttendedPatients(10);
    m_frequentTable->setRowCount(0);
    m_frequentTable->setRowCount(static_cast<int>(frequentList.size()));

    for (int i = 0; i < static_cast<int>(frequentList.size()); ++i) {
        const auto& p = frequentList[i];
        if (!p) continue;
        m_frequentTable->setItem(i, 0, new QTableWidgetItem(QString("#%1").arg(i + 1)));
        m_frequentTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getId())));
        m_frequentTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(p->getName())));
        m_frequentTable->setItem(i, 3, new QTableWidgetItem(QString::number(p->getCompletedAppointmentCount())));
    }

    // 2. Pending Appointments
    auto pendingList = m_manager.getPatientsWithPendingAppointments();
    m_pendingTable->setRowCount(0);
    m_pendingTable->setRowCount(static_cast<int>(pendingList.size()));

    for (int i = 0; i < static_cast<int>(pendingList.size()); ++i) {
        const auto& p = pendingList[i];
        if (!p) continue;
        m_pendingTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p->getId())));
        m_pendingTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getName())));
        m_pendingTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(p->getPatientType())));
        m_pendingTable->setItem(i, 3, new QTableWidgetItem(QString::number(p->getPendingAppointmentCount())));
    }

    // 3. Category Distribution
    auto dist = m_manager.getCategoryDistribution();
    m_distGeneralLabel->setText(QString("General Patients: %1").arg(dist.generalCount));
    m_distEmergencyLabel->setText(QString("Emergency Patients: %1").arg(dist.emergencyCount));
    m_distFollowUpLabel->setText(QString("Follow-Up Patients: %1").arg(dist.followUpCount));

    // 4. Daily Appointment Summary
    auto summary = m_manager.getDailyAppointmentSummary();
    m_summaryTotalLabel->setText(QString("Total: %1").arg(summary.total));
    m_summaryCompletedLabel->setText(QString("Completed: %1").arg(summary.completed));
    m_summaryPendingLabel->setText(QString("Pending: %1").arg(summary.pending));
    m_summaryCancelledLabel->setText(QString("Cancelled: %1").arg(summary.cancelled));
}

void ReportsPanel::onExportRosterCsv() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Patients CSV", "patient_roster.csv", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        try {
            m_manager.exportRosterCsv(fileName.toStdString());
            QMessageBox::information(this, "Export Successful", "Patient roster exported successfully!");
        } catch (const utils::ClinicException& e) {
            QMessageBox::warning(this, "Export Error", QString::fromStdString(e.what()));
        }
    }
}

void ReportsPanel::onExportApptsCsv() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Appointments CSV", "appointments_report.csv", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        try {
            utils::FileHandler::exportAppointmentsToCsv(m_manager.getDatabase(), fileName.toStdString());
            QMessageBox::information(this, "Export Successful", "Appointments report exported successfully!");
        } catch (const utils::ClinicException& e) {
            QMessageBox::warning(this, "Export Error", QString::fromStdString(e.what()));
        }
    }
}

} // namespace scapms::gui
