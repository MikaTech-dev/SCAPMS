#pragma once

#include "../model/PatientRecord.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace scapms::controller {

enum class SortField {
    Name,
    ID,
    Age,
    AppointmentCount
};

enum class SortAlgorithm {
    SelectionSort,
    QuickSort,
    InsertionSort
};

enum class SearchAlgorithm {
    LinearSearch,
    BinarySearch,
    RecursiveSearch
};

class SearchEngine {
public:
    // Manual Sorting Algorithms (NO std::sort)
    static void selectionSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                              SortField field, bool ascending = true);

    static void quickSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                          SortField field, bool ascending = true);

    static void insertionSort(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                              SortField field, bool ascending = true);

    // Manual Searching Algorithms (NO std::find, std::binary_search)
    static int linearSearchById(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                const std::string& targetId);

    static int binarySearch(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                            const std::string& targetValue,
                            SortField field);

    static int recursiveSearchByName(const std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                     const std::string& targetName,
                                     int low, int high);

    // Helpers
    static bool compare(const model::PatientRecord& a, const model::PatientRecord& b,
                        SortField field, bool ascending);

    static std::string getFieldValue(const model::PatientRecord& record, SortField field);

private:
    static void quickSortInternal(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                                  int low, int high, SortField field, bool ascending);
    static int partition(std::vector<std::shared_ptr<model::PatientRecord>>& records,
                         int low, int high, SortField field, bool ascending);
};

} // namespace scapms::controller
