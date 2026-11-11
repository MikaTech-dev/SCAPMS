#include "AppointmentPanel.h"
#include "../utils/Exceptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>

namespace scapms::gui {

AppointmentPanel::AppointmentPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager) {
    setupUI();
    refreshPatientDropdown();
    refreshAppointmentTable();
}

void AppointmentPanel::setupUI() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);

    // Left: Scheduling Form
    auto formGroup = new QGroupBox("Schedule Appointment", this);
    auto formLayout = new QVBoxLayout(formGroup);

    auto inputsLayout = new QFormLayout();

    m_patientCombo = new QComboBox(this);
    m_patientCombo->setToolTip("Select registered patient");

    m_dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
    m_dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_dateTimeEdit->setCalendarPopup(true);

    m_deptCombo = new QComboBox(this);
    m_deptCombo->addItems({"Family Medicine", "Internal Medicine", "Pediatrics", "Cardiology",
                           "Orthopedics", "Dentistry", "Optometry", "Trauma & Emergency"});

    m_doctorEdit = new QLineEdit(this);
    m_doctorEdit->setPlaceholderText("e.g. Dr. Vance / Dr. Okafor");

    m_reasonEdit = new QLineEdit(this);
    m_reasonEdit->setPlaceholderText("Consultation purpose / chief complaint");

    inputsLayout->addRow("Patient*:", m_patientCombo);
    inputsLayout->addRow("Date & Time*:", m_dateTimeEdit);
    inputsLayout->addRow("Department*:", m_deptCombo);
    inputsLayout->addRow("Doctor*:", m_doctorEdit);
    inputsLayout->addRow("Reason / Notes:", m_reasonEdit);
    formLayout->addLayout(inputsLayout);

    m_btnSchedule = new QPushButton("Schedule Appointment", this);
    m_btnSchedule->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px;");
    m_btnSchedule->setToolTip("Create new appointment for selected patient");
    formLayout->addWidget(m_btnSchedule);

    auto actionGroup = new QGroupBox("Appointment Actions", this);
    auto actionLayout = new QHBoxLayout(actionGroup);

    m_btnComplete = new QPushButton("Mark Completed", this);
    m_btnComplete->setStyleSheet("background-color: #0277bd; color: white; font-weight: bold; padding: 6px;");
    m_btnComplete->setToolTip("Change status of selected appointment to Completed");

    m_btnCancel = new QPushButton("Cancel Appointment", this);
    m_btnCancel->setStyleSheet("background-color: #c62828; color: white; font-weight: bold; padding: 6px;");
    m_btnCancel->setToolTip("Cancel selected appointment");

    actionLayout->addWidget(m_btnComplete);
    actionLayout->addWidget(m_btnCancel);
    formLayout->addWidget(actionGroup);

    formLayout->addStretch();
    mainLayout->addWidget(formGroup, 2);

    // Right: Appointments List
    auto tableGroup = new QGroupBox("Master Appointments Schedule", this);
    auto tableLayout = new QVBoxLayout(tableGroup);

    m_apptTable = new QTableWidget(this);
    m_apptTable->setColumnCount(7);
    m_apptTable->setHorizontalHeaderLabels({"Appt ID", "Patient ID", "Date/Time", "Department", "Doctor", "Reason", "Status"});
    m_apptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_apptTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_apptTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_apptTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_apptTable->setAlternatingRowColors(true);
    tableLayout->addWidget(m_apptTable);

    mainLayout->addWidget(tableGroup, 3);

    connect(m_btnSchedule, &QPushButton::clicked, this, &AppointmentPanel::onScheduleClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &AppointmentPanel::onCancelClicked);
    connect(m_btnComplete, &QPushButton::clicked, this, &AppointmentPanel::onCompleteClicked);
}

void AppointmentPanel::refreshPatientDropdown() {
    m_patientCombo->clear();
    for (const auto& p : m_manager.getDatabase().getPatients()) {
        if (!p) continue;
        QString text = QString("%1 - %2 (%3)")
                           .arg(QString::fromStdString(p->getId()))
                           .arg(QString::fromStdString(p->getName()))
                           .arg(QString::fromStdString(p->getPatientType()));
        m_patientCombo->addItem(text, QString::fromStdString(p->getId()));
    }
}

void AppointmentPanel::refreshAppointmentTable() {
    m_apptTable->setRowCount(0);
    auto appts = m_manager.getAppointmentController().getAllAppointments();
    m_apptTable->setRowCount(static_cast<int>(appts.size()));

    for (int i = 0; i < static_cast<int>(appts.size()); ++i) {
        const auto& a = appts[i];
        m_apptTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.getAppointmentId())));
        m_apptTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(a.getPatientId())));
        m_apptTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(a.getDateTime())));
        m_apptTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(a.getDepartment())));
        m_apptTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(a.getDoctor())));
        m_apptTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(a.getReason())));

        auto statusItem = new QTableWidgetItem(QString::fromStdString(a.getStatusString()));
        if (a.getStatus() == model::AppointmentStatus::Pending) {
            statusItem->setBackground(QColor(255, 249, 196)); // Amber
            statusItem->setForeground(QColor(245, 127, 23));
        } else if (a.getStatus() == model::AppointmentStatus::Completed) {
            statusItem->setBackground(QColor(200, 230, 201)); // Green
            statusItem->setForeground(QColor(27, 94, 32));
        } else {
            statusItem->setBackground(QColor(255, 205, 210)); // Red
            statusItem->setForeground(QColor(183, 28, 28));
        }
        m_apptTable->setItem(i, 6, statusItem);
    }
}

void AppointmentPanel::onScheduleClicked() {
    try {
        if (m_patientCombo->count() == 0) {
            throw utils::InvalidAppointmentDataException("No patients registered yet.");
        }

        std::string patientId = m_patientCombo->currentData().toString().toStdString();
        std::string dt = m_dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm").toStdString();
        std::string dept = m_deptCombo->currentText().toStdString();
        std::string doc = m_doctorEdit->text().trimmed().toStdString();
        std::string reason = m_reasonEdit->text().trimmed().toStdString();

        if (doc.empty()) {
            throw utils::InvalidAppointmentDataException("Attending doctor cannot be empty.");
        }

        m_manager.getAppointmentController().scheduleAppointment(patientId, dt, dept, doc, reason);
        refreshAppointmentTable();
        emit dataModified();
        QMessageBox::information(this, "Success", "Appointment scheduled successfully!");
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Scheduling Error", QString::fromStdString(e.what()));
    }
}

void AppointmentPanel::onCancelClicked() {
    int row = m_apptTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select an appointment from the table.");
        return;
    }

    std::string apptId = m_apptTable->item(row, 0)->text().toStdString();
    std::string patId = m_apptTable->item(row, 1)->text().toStdString();

    try {
        m_manager.getAppointmentController().cancelAppointment(patId, apptId);
        refreshAppointmentTable();
        emit dataModified();
        QMessageBox::information(this, "Cancelled", "Appointment cancelled successfully!");
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Cancellation Error", QString::fromStdString(e.what()));
    }
}

void AppointmentPanel::onCompleteClicked() {
    int row = m_apptTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select an appointment from the table.");
        return;
    }

    std::string apptId = m_apptTable->item(row, 0)->text().toStdString();
    std::string patId = m_apptTable->item(row, 1)->text().toStdString();

    try {
        m_manager.getAppointmentController().completeAppointment(patId, apptId);
        refreshAppointmentTable();
        emit dataModified();
        QMessageBox::information(this, "Completed", "Appointment marked as completed!");
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

} // namespace scapms::gui
