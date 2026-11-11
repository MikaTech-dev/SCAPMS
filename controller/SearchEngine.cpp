#include "SearchEngine.h"
#include <algorithm>
#include <iostream>

namespace scapms::controller {

// Compares two PatientRecord objects based on the selected field and sort direction
bool SearchEngine::compare(const model::PatientRecord& a, const model::PatientRecord& b,
                           SortField field, bool ascending) {
    bool less = false;
    switch (field) {
        case SortField::Name:
            less = (a.getName() < b.getName());
            break;
        case SortField::ID:
            less = (a.getId() < b.getId());
            break;
        case SortField::Age:
            less = (a.getAge() < b.getAge());
            break;
        case SortField::AppointmentCount:
            less = (a.getAppointments().size() < b.getAppointments().size());
            break;
    }
    return ascending ? less : !less;
}

std::string SearchEngine::getFieldValue(const model::PatientRecord& record, SortField field) {
    switch (field) {
        case SortField::Name:
            return record.getName();
        case SortField::ID:
            return record.getId();
        case SortField::Age:
            return std::to_string(record.getAge());
        case SortField::AppointmentCount:
            return std::to_string(record.getAppointments().size());
    }
    return "";
}

// ==========================================
// 1. Selection Sort (Manual Implementation)
// Time Complexity: O(n^2) comparisons, O(n) swaps
// Best suited for small arrays where minimizing memory write operations is paramount.
// ==========================================
void SearchEngine::selectionSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                 SortField field, bool ascending) {
    int n = static_cast<int>(records.size());
    for (int i = 0; i < n - 1; ++i) {
        int targetIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (compare(*records[j], *records[targetIdx], field, ascending)) {
                targetIdx = j;
            }
        }
        if (targetIdx != i) {
            std::swap(records[i], records[targetIdx]);
        }
    }
}

// ==========================================
// 2. Insertion Sort (Manual Implementation)
// Time Complexity: O(n^2) worst/average, O(n) best for nearly sorted data
// Best suited for incremental updates and already partially sorted datasets.
// ==========================================
void SearchEngine::insertionSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                 SortField field, bool ascending) {
    int n = static_cast<int>(records.size());
    for (int i = 1; i < n; ++i) {
        auto key = records[i];
        int j = i - 1;
        while (j >= 0 && compare(*key, *records[j], field, ascending)) {
            records[j + 1] = records[j];
            --j;
        }
        records[j + 1] = key;
    }
}

// ==========================================
// 3. Quick Sort (Manual Implementation)
// Time Complexity: O(n log n) average, divide-and-conquer
// Best suited for large patient directories requiring high-performance re-indexing.
// ==========================================
int SearchEngine::partition(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                            int low, int high, SortField field, bool ascending) {
    auto pivot = records[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (compare(*records[j], *pivot, field, ascending)) {
            ++i;
            std::swap(records[i], records[j]);
        }
    }
    std::swap(records[i + 1], records[high]);
    return i + 1;
}

void SearchEngine::quickSortInternal(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                     int low, int high, SortField field, bool ascending) {
    if (low < high) {
        int pi = partition(records, low, high, field, ascending);
        quickSortInternal(records, low, pi - 1, field, ascending);
        quickSortInternal(records, pi + 1, high, field, ascending);
    }
}

void SearchEngine::quickSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                             SortField field, bool ascending) {
    if (records.empty()) return;
    quickSortInternal(records, 0, static_cast<int>(records.size()) - 1, field, ascending);
}

// ==========================================
// 4. Linear Search (Manual Implementation)
// Time Complexity: O(n)
// Works on unsorted patient records.
// ==========================================
int SearchEngine::linearSearchById(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                   const std::string& targetId) {
    for (int i = 0; i < static_cast<int>(records.size()); ++i) {
        if (records[i] && records[i]->getId() == targetId) {
            return i;
        }
    }
    return -1;
}

// ==========================================
// 5. Binary Search (Manual Implementation)
// Time Complexity: O(log n)
// Assumes records are already sorted ascending by the specified field.
// ==========================================
int SearchEngine::binarySearch(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                               const std::string& targetValue,
                               SortField field) {
    int low = 0;
    int high = static_cast<int>(records.size()) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        std::string currentVal = getFieldValue(*records[mid], field);

        if (currentVal == targetValue) {
            return mid;
        }
        if (currentVal < targetValue) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

// ==========================================
// 6. Recursive Search (Manual Implementation)
// Time Complexity: O(n), Space Complexity: O(n) stack frame depth
// Recursively traverses the records array searching for targetName.
// ==========================================
int SearchEngine::recursiveSearchByName(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                        const std::string& targetName,
                                        int low, int high) {
    // Base Case 1: Range exhausted without finding target
    if (low > high) {
        return -1;
    }

    // Base Case 2: Match found at current head
    if (records[low] && records[low]->getName() == targetName) {
        return low;
    }

    // Recursive Step: Check remaining subarray
    return recursiveSearchByName(records, targetName, low + 1, high);
}

} // namespace scapms::controller
