#include "PatientPanel.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "../utils/Exceptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

namespace scapms::gui {

PatientPanel::PatientPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager) {
    setupUI();
    refreshPatientTable();
    refreshCacheTable();
}

void PatientPanel::setupUI() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);

    // Left Side: Registration & Edit Form
    auto formGroupBox = new QGroupBox("Patient Registration & Management", this);
    auto formGroupLayout = new QVBoxLayout(formGroupBox);

    auto baseFormLayout = new QFormLayout();
    baseFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("Auto-generated (e.g. P1007) or custom");
    m_idEdit->setToolTip("Leave blank for auto-generation, or enter custom unique Patient ID");

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Full Legal Name");
    m_nameEdit->setToolTip("Enter patient's full name");

    m_ageSpin = new QSpinBox(this);
    m_ageSpin->setRange(0, 120);
    m_ageSpin->setValue(25);
    m_ageSpin->setToolTip("Age in years (0 - 120)");

    m_contactEdit = new QLineEdit(this);
    m_contactEdit->setPlaceholderText("+1-555-0199 or email");
    m_contactEdit->setToolTip("Phone number or primary email address");

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems({"General", "Emergency", "Follow-Up"});
    m_typeCombo->setToolTip("Select patient category for specific triage and fee details");

    baseFormLayout->addRow("Patient ID:", m_idEdit);
    baseFormLayout->addRow("Patient Name*:", m_nameEdit);
    baseFormLayout->addRow("Age (0-120)*:", m_ageSpin);
    baseFormLayout->addRow("Contact Phone*:", m_contactEdit);
    baseFormLayout->addRow("Patient Category*:", m_typeCombo);
    formGroupLayout->addLayout(baseFormLayout);

    // Dynamic Stacked Widget for Category-specific Fields
    auto dynamicGroupBox = new QGroupBox("Category-Specific Parameters", this);
    auto dynamicGroupLayout = new QVBoxLayout(dynamicGroupBox);
    m_dynamicStack = new QStackedWidget(this);

    // 1. General Patient Page
    auto generalWidget = new QWidget(this);
    auto generalLayout = new QFormLayout(generalWidget);
    m_feeSpin = new QDoubleSpinBox(this);
    m_feeSpin->setRange(0.0, 10000.0);
    m_feeSpin->setValue(50.0);
    m_feeSpin->setPrefix("$");
    m_insuranceEdit = new QLineEdit(this);
    m_insuranceEdit->setPlaceholderText("e.g. CampusHealth Gold / Self-pay");
    generalLayout->addRow("Consultation Fee:", m_feeSpin);
    generalLayout->addRow("Insurance Provider:", m_insuranceEdit);
    m_dynamicStack->addWidget(generalWidget);

    // 2. Emergency Patient Page
    auto emergencyWidget = new QWidget(this);
    auto emergencyLayout = new QFormLayout(emergencyWidget);
    m_triageSpin = new QSpinBox(this);
    m_triageSpin->setRange(1, 5);
    m_triageSpin->setValue(1);
    m_triageSpin->setToolTip("1 = Resuscitation/Immediate, 5 = Non-urgent");
    m_incidentEdit = new QLineEdit(this);
    m_incidentEdit->setPlaceholderText("Trauma or acute condition description");
    m_traumaCheck = new QCheckBox("Requires Immediate Trauma Unit", this);
    emergencyLayout->addRow("Triage Level (1-5):", m_triageSpin);
    emergencyLayout->addRow("Incident Details:", m_incidentEdit);
    emergencyLayout->addRow("", m_traumaCheck);
    m_dynamicStack->addWidget(emergencyWidget);

    // 3. Follow-Up Patient Page
    auto followUpWidget = new QWidget(this);
    auto followUpLayout = new QFormLayout(followUpWidget);
    m_doctorEdit = new QLineEdit(this);
    m_doctorEdit->setPlaceholderText("Assigned Attending Physician");
    m_diagnosisEdit = new QLineEdit(this);
    m_diagnosisEdit->setPlaceholderText("Prior diagnosis under review");
    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(1, 365);
    m_intervalSpin->setValue(14);
    m_intervalSpin->setSuffix(" days");
    followUpLayout->addRow("Primary Doctor:", m_doctorEdit);
    followUpLayout->addRow("Prior Diagnosis:", m_diagnosisEdit);
    followUpLayout->addRow("Follow-Up Interval:", m_intervalSpin);
    m_dynamicStack->addWidget(followUpWidget);

    dynamicGroupLayout->addWidget(m_dynamicStack);
    formGroupLayout->addWidget(dynamicGroupBox);

    // Action Buttons
    auto btnLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton("Register Patient", this);
    m_btnAdd->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px;");
    m_btnAdd->setToolTip("Add new patient to the master registry");

    m_btnUpdate = new QPushButton("Update Record", this);
    m_btnUpdate->setStyleSheet("background-color: #1565c0; color: white; font-weight: bold; padding: 6px;");
    m_btnUpdate->setToolTip("Save modifications to the selected patient");

    m_btnDelete = new QPushButton("Delete Patient", this);
    m_btnDelete->setStyleSheet("background-color: #c62828; color: white; font-weight: bold; padding: 6px;");
    m_btnDelete->setToolTip("Delete selected patient (can be reversed via Undo)");

    m_btnClear = new QPushButton("Clear Form", this);
    m_btnClear->setToolTip("Reset form fields");

    btnLayout->addWidget(m_btnAdd);
    btnLayout->addWidget(m_btnUpdate);
    btnLayout->addWidget(m_btnDelete);
    btnLayout->addWidget(m_btnClear);
    formGroupLayout->addLayout(btnLayout);

    // Frequently Accessed Patients Cache (Fixed 10-slot Array)
    auto cacheGroup = new QGroupBox("Frequently Accessed Cache (Fixed 10-Slot Array)", this);
    auto cacheLayout = new QVBoxLayout(cacheGroup);
    m_cacheTable = new QTableWidget(this);
    m_cacheTable->setColumnCount(4);
    m_cacheTable->setHorizontalHeaderLabels({"Slot", "ID", "Name", "Category"});
    m_cacheTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cacheTable->setMaximumHeight(140);
    cacheLayout->addWidget(m_cacheTable);
    formGroupLayout->addWidget(cacheGroup);

    mainLayout->addWidget(formGroupBox, 2);

    // Right Side: Master Patient Table
    auto tableGroupBox = new QGroupBox("Master Patient Registry", this);
    auto tableLayout = new QVBoxLayout(tableGroupBox);

    m_patientTable = new QTableWidget(this);
    m_patientTable->setColumnCount(6);
    m_patientTable->setHorizontalHeaderLabels({"ID", "Category", "Name", "Age", "Contact", "Specialized Information"});
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_patientTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientTable->setAlternatingRowColors(true);
    tableLayout->addWidget(m_patientTable);

    mainLayout->addWidget(tableGroupBox, 3);

    // Connect signals
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PatientPanel::onPatientTypeChanged);
    connect(m_btnAdd, &QPushButton::clicked, this, &PatientPanel::onAddPatientClicked);
    connect(m_btnUpdate, &QPushButton::clicked, this, &PatientPanel::onUpdatePatientClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &PatientPanel::onDeletePatientClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &PatientPanel::onClearFormClicked);
    connect(m_patientTable, &QTableWidget::itemSelectionChanged, this, &PatientPanel::onTableSelectionChanged);
}

void PatientPanel::onPatientTypeChanged(int index) {
    m_dynamicStack->setCurrentIndex(index);
}

void PatientPanel::applyCustomTableBadges(int row, const std::string& type) {
    auto item = m_patientTable->item(row, 1);
    if (!item) return;

    if (type == "Emergency") {
        item->setBackground(QColor(255, 205, 210)); // Soft red
        item->setForeground(QColor(183, 28, 28));    // Dark red
        item->setText("EMERGENCY (High Priority)");
    } else if (type == "Follow-Up") {
        item->setBackground(QColor(187, 222, 251)); // Soft blue
        item->setForeground(QColor(13, 71, 161));    // Dark blue
        item->setText("FOLLOW-UP (Scheduled)");
    } else {
        item->setBackground(QColor(200, 230, 201)); // Soft green
        item->setForeground(QColor(27, 94, 32));     // Dark green
        item->setText("GENERAL (Normal)");
    }
}

void PatientPanel::refreshPatientTable() {
    m_patientTable->setRowCount(0);
    const auto& list = m_manager.getDatabase().getPatients();
    m_patientTable->setRowCount(static_cast<int>(list.size()));

    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        const auto& p = list[i];
        if (!p) continue;

        m_patientTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p->getId())));
        m_patientTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getPatientType())));
        m_patientTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(p->getName())));
        m_patientTable->setItem(i, 3, new QTableWidgetItem(QString::number(p->getAge())));
        m_patientTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(p->getContact())));

        std::string specialDetails;
        if (auto gp = std::dynamic_pointer_cast<model::GeneralPatient>(p)) {
            specialDetails = "Fee: $" + QString::number(gp->getClinicFee(), 'f', 2).toStdString() +
                             " | Ins: " + gp->getInsuranceProvider();
        } else if (auto ep = std::dynamic_pointer_cast<model::EmergencyPatient>(p)) {
            specialDetails = "Triage: Level " + std::to_string(ep->getTriageLevel()) +
                             " | " + ep->getIncidentDescription() +
                             (ep->requiresTraumaUnit() ? " [TRAUMA]" : "");
        } else if (auto fp = std::dynamic_pointer_cast<model::FollowUpPatient>(p)) {
            specialDetails = "Dr: " + fp->getPrimaryDoctor() +
                             " | Diag: " + fp->getPreviousDiagnosis() +
                             " | Every " + std::to_string(fp->getFollowUpIntervalDays()) + "d";
        }
        m_patientTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(specialDetails)));

        applyCustomTableBadges(i, p->getPatientType());
    }
}

void PatientPanel::refreshCacheTable() {
    m_cacheTable->setRowCount(0);
    const auto& cache = m_manager.getDatabase().getRecentPatients();
    int count = 0;
    for (size_t i = 0; i < cache.size(); ++i) {
        if (cache[i]) {
            m_cacheTable->insertRow(count);
            m_cacheTable->setItem(count, 0, new QTableWidgetItem(QString("#%1").arg(i + 1)));
            m_cacheTable->setItem(count, 1, new QTableWidgetItem(QString::fromStdString(cache[i]->getId())));
            m_cacheTable->setItem(count, 2, new QTableWidgetItem(QString::fromStdString(cache[i]->getName())));
            m_cacheTable->setItem(count, 3, new QTableWidgetItem(QString::fromStdString(cache[i]->getPatientType())));
            count++;
        }
    }
}

void PatientPanel::onAddPatientClicked() {
    try {
        std::string id = m_idEdit->text().trimmed().toStdString();
        std::string name = m_nameEdit->text().trimmed().toStdString();
        int age = m_ageSpin->value();
        std::string contact = m_contactEdit->text().trimmed().toStdString();
        int typeIdx = m_typeCombo->currentIndex();

        if (name.empty()) {
            throw utils::InvalidPatientDataException("Patient name cannot be empty.");
        }

        if (typeIdx == 0) { // General
            m_manager.registerGeneralPatient(name, age, contact, m_feeSpin->value(),
                                             m_insuranceEdit->text().trimmed().toStdString(), id);
        } else if (typeIdx == 1) { // Emergency
            m_manager.registerEmergencyPatient(name, age, contact, m_triageSpin->value(),
                                               m_incidentEdit->text().trimmed().toStdString(),
                                               m_traumaCheck->isChecked(), id);
        } else if (typeIdx == 2) { // Follow-Up
            m_manager.registerFollowUpPatient(name, age, contact,
                                              m_doctorEdit->text().trimmed().toStdString(),
                                              m_diagnosisEdit->text().trimmed().toStdString(),
                                              m_intervalSpin->value(), id);
        }

        refreshPatientTable();
        refreshCacheTable();
        onClearFormClicked();
        emit dataModified();
        QMessageBox::information(this, "Success", "Patient successfully registered!");
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Validation Error", QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "System Error", QString("Unexpected error: %1").arg(e.what()));
    }
}

void PatientPanel::onUpdatePatientClicked() {
    try {
        std::string id = m_idEdit->text().trimmed().toStdString();
        if (id.empty()) {
            throw utils::InvalidPatientDataException("Select a patient from the table to update.");
        }

        std::string name = m_nameEdit->text().trimmed().toStdString();
        int age = m_ageSpin->value();
        std::string contact = m_contactEdit->text().trimmed().toStdString();
        int typeIdx = m_typeCombo->currentIndex();

        std::shared_ptr<model::PatientRecord> updated;
        if (typeIdx == 0) {
            updated = std::make_shared<model::GeneralPatient>(id, name, age, contact,
                                                              m_feeSpin->value(),
                                                              m_insuranceEdit->text().trimmed().toStdString());
        } else if (typeIdx == 1) {
            updated = std::make_shared<model::EmergencyPatient>(id, name, age, contact,
                                                                m_triageSpin->value(),
                                                                m_incidentEdit->text().trimmed().toStdString(),
                                                                m_traumaCheck->isChecked());
        } else {
            updated = std::make_shared<model::FollowUpPatient>(id, name, age, contact,
                                                               m_doctorEdit->text().trimmed().toStdString(),
                                                               m_diagnosisEdit->text().trimmed().toStdString(),
                                                               m_intervalSpin->value());
        }

        // Preserve previous appointments
        auto existing = m_manager.findPatientById(id);
        if (existing) {
            updated->getAppointments() = existing->getAppointments();
        }

        m_manager.updatePatient(updated);
        refreshPatientTable();
        refreshCacheTable();
        emit dataModified();
        QMessageBox::information(this, "Updated", "Patient record updated successfully!");
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Update Error", QString::fromStdString(e.what()));
    }
}

void PatientPanel::onDeletePatientClicked() {
    int row = m_patientTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a patient from the table to delete.");
        return;
    }

    std::string id = m_patientTable->item(row, 0)->text().toStdString();
    auto reply = QMessageBox::question(this, "Confirm Deletion",
                                       QString("Are you sure you want to delete patient %1?\n(This action can be reversed via Undo).")
                                       .arg(QString::fromStdString(id)),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        try {
            m_manager.deletePatient(id);
            refreshPatientTable();
            refreshCacheTable();
            onClearFormClicked();
            emit dataModified();
            QMessageBox::information(this, "Deleted", "Patient deleted. Use 'Undo' to restore if needed.");
        } catch (const utils::ClinicException& e) {
            QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
        }
    }
}

void PatientPanel::onTableSelectionChanged() {
    int row = m_patientTable->currentRow();
    if (row < 0) return;

    std::string id = m_patientTable->item(row, 0)->text().toStdString();
    auto patient = m_manager.findPatientById(id);
    if (!patient) return;

    m_idEdit->setText(QString::fromStdString(patient->getId()));
    m_nameEdit->setText(QString::fromStdString(patient->getName()));
    m_ageSpin->setValue(patient->getAge());
    m_contactEdit->setText(QString::fromStdString(patient->getContact()));

    if (auto gp = std::dynamic_pointer_cast<model::GeneralPatient>(patient)) {
        m_typeCombo->setCurrentIndex(0);
        m_feeSpin->setValue(gp->getClinicFee());
        m_insuranceEdit->setText(QString::fromStdString(gp->getInsuranceProvider()));
    } else if (auto ep = std::dynamic_pointer_cast<model::EmergencyPatient>(patient)) {
        m_typeCombo->setCurrentIndex(1);
        m_triageSpin->setValue(ep->getTriageLevel());
        m_incidentEdit->setText(QString::fromStdString(ep->getIncidentDescription()));
        m_traumaCheck->setChecked(ep->requiresTraumaUnit());
    } else if (auto fp = std::dynamic_pointer_cast<model::FollowUpPatient>(patient)) {
        m_typeCombo->setCurrentIndex(2);
        m_doctorEdit->setText(QString::fromStdString(fp->getPrimaryDoctor()));
        m_diagnosisEdit->setText(QString::fromStdString(fp->getPreviousDiagnosis()));
        m_intervalSpin->setValue(fp->getFollowUpIntervalDays());
    }
    refreshCacheTable();
}

void PatientPanel::onClearFormClicked() {
    m_idEdit->clear();
    m_nameEdit->clear();
    m_ageSpin->setValue(25);
    m_contactEdit->clear();
    m_insuranceEdit->clear();
    m_incidentEdit->clear();
    m_traumaCheck->setChecked(false);
    m_doctorEdit->clear();
    m_diagnosisEdit->clear();
    m_patientTable->clearSelection();
}

} // namespace scapms::gui
