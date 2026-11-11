#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QPushButton>
#include "../controller/ClinicManager.h"

namespace scapms::gui {

class AppointmentPanel : public QWidget {
    Q_OBJECT
public:
    explicit AppointmentPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refreshAppointmentTable();
    void refreshPatientDropdown();

signals:
    void dataModified();

private slots:
    void onScheduleClicked();
    void onCancelClicked();
    void onCompleteClicked();

private:
    void setupUI();

    controller::ClinicManager& m_manager;

    QTableWidget* m_apptTable{nullptr};

    QComboBox* m_patientCombo{nullptr};
    QDateTimeEdit* m_dateTimeEdit{nullptr};
    QComboBox* m_deptCombo{nullptr};
    QLineEdit* m_doctorEdit{nullptr};
    QLineEdit* m_reasonEdit{nullptr};

    QPushButton* m_btnSchedule{nullptr};
    QPushButton* m_btnCancel{nullptr};
    QPushButton* m_btnComplete{nullptr};
};

} // namespace scapms::gui
