#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include "../controller/ClinicManager.h"

namespace scapms::gui {

class SearchPanel : public QWidget {
    Q_OBJECT
public:
    explicit SearchPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refreshTable();

signals:
    void dataModified();

private slots:
    void onSearchClicked();
    void onSortClicked();
    void onSortAlgoChanged(int index);

private:
    void setupUI();
    void updateSortRationale();

    controller::ClinicManager& m_manager;

    // Search UI
    QLineEdit* m_searchEdit{nullptr};
    QComboBox* m_searchAlgoCombo{nullptr};
    QPushButton* m_btnSearch{nullptr};
    QLabel* m_searchResultLabel{nullptr};

    // Sort UI
    QComboBox* m_sortAlgoCombo{nullptr};
    QComboBox* m_sortFieldCombo{nullptr};
    QComboBox* m_sortOrderCombo{nullptr};
    QPushButton* m_btnSort{nullptr};
    QTextBrowser* m_rationaleBrowser{nullptr};

    // Table
    QTableWidget* m_table{nullptr};
};

} // namespace scapms::gui
