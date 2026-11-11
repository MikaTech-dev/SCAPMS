#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include "../controller/ClinicManager.h"

namespace scapms::gui {

class PatientPanel : public QWidget {
    Q_OBJECT
public:
    explicit PatientPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refreshPatientTable();
    void refreshCacheTable();

signals:
    void dataModified();

private slots:
    void onPatientTypeChanged(int index);
    void onAddPatientClicked();
    void onUpdatePatientClicked();
    void onDeletePatientClicked();
    void onTableSelectionChanged();
    void onClearFormClicked();

private:
    void setupUI();
    void applyCustomTableBadges(int row, const std::string& type);

    controller::ClinicManager& m_manager;

    // UI Widgets
    QTableWidget* m_patientTable{nullptr};
    QTableWidget* m_cacheTable{nullptr};

    // Form inputs
    QLineEdit* m_idEdit{nullptr};
    QLineEdit* m_nameEdit{nullptr};
    QSpinBox* m_ageSpin{nullptr};
    QLineEdit* m_contactEdit{nullptr};
    QComboBox* m_typeCombo{nullptr};

    // Dynamic widgets per type
    QStackedWidget* m_dynamicStack{nullptr};
    // General
    QDoubleSpinBox* m_feeSpin{nullptr};
    QLineEdit* m_insuranceEdit{nullptr};
    // Emergency
    QSpinBox* m_triageSpin{nullptr};
    QLineEdit* m_incidentEdit{nullptr};
    QCheckBox* m_traumaCheck{nullptr};
    // Follow-Up
    QLineEdit* m_doctorEdit{nullptr};
    QLineEdit* m_diagnosisEdit{nullptr};
    QSpinBox* m_intervalSpin{nullptr};

    // Buttons
    QPushButton* m_btnAdd{nullptr};
    QPushButton* m_btnUpdate{nullptr};
    QPushButton* m_btnDelete{nullptr};
    QPushButton* m_btnClear{nullptr};
};

} // namespace scapms::gui
