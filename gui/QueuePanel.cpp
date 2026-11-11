#include "QueuePanel.h"
#include "../utils/Exceptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

namespace scapms::gui {

QueuePanel::QueuePanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager) {
    setupUI();
    refreshPatientDropdown();
    refreshQueue();
}

void QueuePanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);

    // Queue Control Bar
    auto controlGroup = new QGroupBox("Consultation Line Controls (FIFO Queue)", this);
    auto controlLayout = new QHBoxLayout(controlGroup);

    m_patientCombo = new QComboBox(this);
    m_patientCombo->setMinimumWidth(250);
    m_patientCombo->setToolTip("Select patient to add to the consultation queue");

    m_btnEnqueue = new QPushButton("Enqueue Patient", this);
    m_btnEnqueue->setStyleSheet("background-color: #0277bd; color: white; font-weight: bold; padding: 6px 12px;");
    m_btnEnqueue->setToolTip("Place patient at the end of the waiting line");

    m_btnCallNext = new QPushButton("Call Next Patient", this);
    m_btnCallNext->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px 16px; font-size: 13px;");
    m_btnCallNext->setToolTip("Dequeue the next patient in line for doctor consultation");

    m_queueStatusLabel = new QLabel("Waiting Patients: 0", this);
    m_queueStatusLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #37474f; margin-left: 20px;");

    controlLayout->addWidget(new QLabel("Select Patient:"));
    controlLayout->addWidget(m_patientCombo);
    controlLayout->addWidget(m_btnEnqueue);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_btnCallNext);
    controlLayout->addWidget(m_queueStatusLabel);
    controlLayout->addStretch();

    mainLayout->addWidget(controlGroup);

    // Queue Table
    auto tableGroup = new QGroupBox("Active Consultation Waiting Line (FIFO Order)", this);
    auto tableLayout = new QVBoxLayout(tableGroup);

    m_queueTable = new QTableWidget(this);
    m_queueTable->setColumnCount(5);
    m_queueTable->setHorizontalHeaderLabels({"Position", "Patient ID", "Category", "Name", "Contact"});
    m_queueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_queueTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueTable->setAlternatingRowColors(true);
    tableLayout->addWidget(m_queueTable);

    mainLayout->addWidget(tableGroup);

    connect(m_btnEnqueue, &QPushButton::clicked, this, &QueuePanel::onEnqueueClicked);
    connect(m_btnCallNext, &QPushButton::clicked, this, &QueuePanel::onCallNextClicked);
}

void QueuePanel::refreshPatientDropdown() {
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

void QueuePanel::refreshQueue() {
    m_queueTable->setRowCount(0);
    auto patients = m_manager.getWaitingQueuePatients();
    m_queueTable->setRowCount(static_cast<int>(patients.size()));

    for (int i = 0; i < static_cast<int>(patients.size()); ++i) {
        const auto& p = patients[i];
        if (!p) continue;

        auto posItem = new QTableWidgetItem(QString("#%1 in line").arg(i + 1));
        if (i == 0) {
            posItem->setBackground(QColor(255, 236, 179)); // Highlight next patient
            posItem->setText(">>> NEXT <<<");
        }

        m_queueTable->setItem(i, 0, posItem);
        m_queueTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getId())));

        auto catItem = new QTableWidgetItem(QString::fromStdString(p->getPatientType()));
        if (p->getPatientType() == "Emergency") {
            catItem->setBackground(QColor(255, 205, 210));
            catItem->setForeground(QColor(183, 28, 28));
        } else if (p->getPatientType() == "Follow-Up") {
            catItem->setBackground(QColor(187, 222, 251));
            catItem->setForeground(QColor(13, 71, 161));
        }
        m_queueTable->setItem(i, 2, catItem);

        m_queueTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(p->getName())));
        m_queueTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(p->getContact())));
    }

    m_queueStatusLabel->setText(QString("Waiting Patients: %1").arg(patients.size()));
}

void QueuePanel::onEnqueueClicked() {
    if (m_patientCombo->count() == 0) return;

    std::string id = m_patientCombo->currentData().toString().toStdString();
    try {
        m_manager.enqueuePatient(id);
        refreshQueue();
        emit dataModified();
    } catch (const utils::ClinicException& e) {
        QMessageBox::warning(this, "Queue Error", QString::fromStdString(e.what()));
    }
}

void QueuePanel::onCallNextClicked() {
    try {
        auto patient = m_manager.callNextPatient();
        refreshQueue();
        emit dataModified();
        if (patient) {
            QMessageBox::information(this, "Now Attending",
                                     QString("Calling for Consultation:\n\nPatient: %1\nID: %2\nType: %3")
                                         .arg(QString::fromStdString(patient->getName()))
                                         .arg(QString::fromStdString(patient->getId()))
                                         .arg(QString::fromStdString(patient->getPatientType())));
        }
    } catch (const utils::QueueEmptyException& e) {
        QMessageBox::information(this, "Queue Empty", "No patients are currently waiting in the consultation queue.");
    }
}

} // namespace scapms::gui
