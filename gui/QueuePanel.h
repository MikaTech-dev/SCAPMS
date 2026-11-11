#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "../controller/ClinicManager.h"

namespace scapms::gui {

class QueuePanel : public QWidget {
    Q_OBJECT
public:
    explicit QueuePanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refreshQueue();
    void refreshPatientDropdown();

signals:
    void dataModified();

private slots:
    void onEnqueueClicked();
    void onCallNextClicked();

private:
    void setupUI();

    controller::ClinicManager& m_manager;

    QComboBox* m_patientCombo{nullptr};
    QPushButton* m_btnEnqueue{nullptr};
    QPushButton* m_btnCallNext{nullptr};
    QLabel* m_queueStatusLabel{nullptr};
    QTableWidget* m_queueTable{nullptr};
};

} // namespace scapms::gui
