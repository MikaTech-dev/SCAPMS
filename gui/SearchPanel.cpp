#include "SearchPanel.h"
#include "../utils/Exceptions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

namespace scapms::gui {

SearchPanel::SearchPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager) {
    setupUI();
    updateSortRationale();
    refreshTable();
}

void SearchPanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);

    // Top: Algorithm Controls (Search & Sort Side by Side)
    auto topLayout = new QHBoxLayout();

    // 1. Searching GroupBox
    auto searchGroup = new QGroupBox("Manual Searching Engine", this);
    auto searchLayout = new QVBoxLayout(searchGroup);

    auto searchInputLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search query (e.g. P1001 or Alice Johnson)");

    m_searchAlgoCombo = new QComboBox(this);
    m_searchAlgoCombo->addItems({"Linear Search (by ID)", "Binary Search (by ID on Sorted)", "Recursive Search (by Name)"});

    m_btnSearch = new QPushButton("SEARCH", this);
    m_btnSearch->setStyleSheet("background-color: #00796b; color: white; font-weight: bold; padding: 6px 14px;");

    searchInputLayout->addWidget(m_searchEdit, 2);
    searchInputLayout->addWidget(m_searchAlgoCombo, 2);
    searchInputLayout->addWidget(m_btnSearch, 1);
    searchLayout->addLayout(searchInputLayout);

    m_searchResultLabel = new QLabel("Ready for query.", this);
    m_searchResultLabel->setStyleSheet("font-weight: bold; color: #1565c0; padding: 4px;");
    m_searchResultLabel->setWordWrap(true);
    searchLayout->addWidget(m_searchResultLabel);

    topLayout->addWidget(searchGroup, 1);

    // 2. Sorting GroupBox
    auto sortGroup = new QGroupBox("Manual Sorting Engine", this);
    auto sortLayout = new QVBoxLayout(sortGroup);

    auto sortControlLayout = new QHBoxLayout();
    m_sortAlgoCombo = new QComboBox(this);
    m_sortAlgoCombo->addItems({"Selection Sort", "Quick Sort", "Insertion Sort"});

    m_sortFieldCombo = new QComboBox(this);
    m_sortFieldCombo->addItems({"Patient Name", "Patient ID", "Age", "Appointment Count"});

    m_sortOrderCombo = new QComboBox(this);
    m_sortOrderCombo->addItems({"Ascending", "Descending"});

    m_btnSort = new QPushButton("SORT RECORDS", this);
    m_btnSort->setStyleSheet("background-color: #e65100; color: white; font-weight: bold; padding: 6px 14px;");

    sortControlLayout->addWidget(new QLabel("Algorithm:"));
    sortControlLayout->addWidget(m_sortAlgoCombo);
    sortControlLayout->addWidget(new QLabel("Sort By:"));
    sortControlLayout->addWidget(m_sortFieldCombo);
    sortControlLayout->addWidget(m_sortOrderCombo);
    sortControlLayout->addWidget(m_btnSort);
    sortLayout->addLayout(sortControlLayout);

    m_rationaleBrowser = new QTextBrowser(this);
    m_rationaleBrowser->setMaximumHeight(70);
    sortLayout->addWidget(m_rationaleBrowser);

    topLayout->addWidget(sortGroup, 1);
    mainLayout->addLayout(topLayout);

    // Bottom: Results Table
    auto tableGroup = new QGroupBox("Patient Records View (Reflects Sorting Order)", this);
    auto tableLayout = new QVBoxLayout(tableGroup);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "Name", "Category", "Age", "Contact", "Total Appts"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    tableLayout->addWidget(m_table);

    mainLayout->addWidget(tableGroup);

    connect(m_btnSearch, &QPushButton::clicked, this, &SearchPanel::onSearchClicked);
    connect(m_btnSort, &QPushButton::clicked, this, &SearchPanel::onSortClicked);
    connect(m_sortAlgoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchPanel::onSortAlgoChanged);
}

void SearchPanel::updateSortRationale() {
    int algoIdx = m_sortAlgoCombo->currentIndex();
    if (algoIdx == 0) {
        m_rationaleBrowser->setHtml("<b>Selection Sort:</b> In-place algorithm with <i>O(n²)</i> comparisons and at most <i>O(n)</i> element swaps. Ideal when write operations to memory or storage are costly.");
    } else if (algoIdx == 1) {
        m_rationaleBrowser->setHtml("<b>Quick Sort:</b> Divide-and-conquer algorithm with <i>O(n log n)</i> average time complexity. Highly optimized cache performance makes it ideal for large clinical directories.");
    } else {
        m_rationaleBrowser->setHtml("<b>Insertion Sort:</b> Efficient online algorithm with <i>O(n)</i> best-case for nearly sorted lists. Ideal when small numbers of new patients are incrementally registered.");
    }
}

void SearchPanel::onSortAlgoChanged(int) {
    updateSortRationale();
}

void SearchPanel::refreshTable() {
    m_table->setRowCount(0);
    const auto& list = m_manager.getDatabase().getPatients();
    m_table->setRowCount(static_cast<int>(list.size()));

    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        const auto& p = list[i];
        if (!p) continue;

        m_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p->getId())));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getName())));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(p->getPatientType())));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(p->getAge())));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(p->getContact())));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(p->getAppointments().size())));
    }
}

void SearchPanel::onSearchClicked() {
    std::string query = m_searchEdit->text().trimmed().toStdString();
    if (query.empty()) {
        QMessageBox::warning(this, "Empty Query", "Please enter a search term.");
        return;
    }

    int algoIdx = m_searchAlgoCombo->currentIndex();
    controller::SearchAlgorithm algo;
    controller::SortField field = controller::SortField::ID;

    if (algoIdx == 0) {
        algo = controller::SearchAlgorithm::LinearSearch;
    } else if (algoIdx == 1) {
        algo = controller::SearchAlgorithm::BinarySearch;
        // Auto-sort by ID first to guarantee binary search invariants
        m_manager.sortPatients(controller::SortAlgorithm::QuickSort, controller::SortField::ID, true);
        refreshTable();
    } else {
        algo = controller::SearchAlgorithm::RecursiveSearch;
        field = controller::SortField::Name;
    }

    auto patient = m_manager.searchPatient(algo, query, field);
    if (patient) {
        m_searchResultLabel->setText(QString("FOUND: %1 | Type: %2 | Age: %3 | Contact: %4")
                                     .arg(QString::fromStdString(patient->getName()))
                                     .arg(QString::fromStdString(patient->getPatientType()))
                                     .arg(patient->getAge())
                                     .arg(QString::fromStdString(patient->getContact())));

        // Highlight in table
        for (int r = 0; r < m_table->rowCount(); ++r) {
            if (m_table->item(r, 0)->text().toStdString() == patient->getId()) {
                m_table->selectRow(r);
                break;
            }
        }
    } else {
        m_searchResultLabel->setText(QString("No matching patient record found for: '%1'").arg(QString::fromStdString(query)));
    }
}

void SearchPanel::onSortClicked() {
    controller::SortAlgorithm algo = static_cast<controller::SortAlgorithm>(m_sortAlgoCombo->currentIndex());
    controller::SortField field = static_cast<controller::SortField>(m_sortFieldCombo->currentIndex());
    bool ascending = (m_sortOrderCombo->currentIndex() == 0);

    m_manager.sortPatients(algo, field, ascending);
    refreshTable();
    emit dataModified();
}

} // namespace scapms::gui
