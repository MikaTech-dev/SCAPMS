#include "ClinicManager.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "../utils/IDGenerator.h"
#include "../utils/FileHandler.h"
#include "../utils/Exceptions.h"

namespace scapms::controller {

ClinicManager::ClinicManager()
    : m_appointmentController(m_db) {}

void ClinicManager::registerGeneralPatient(const std::string& name, int age, const std::string& contact,
                                           double clinicFee, const std::string& insuranceProvider,
                                           const std::string& customId) {
    std::string id = customId.empty() ? utils::IDGenerator::instance().generatePatientId() : customId;
    auto patient = std::make_shared<model::GeneralPatient>(id, name, age, contact, clinicFee, insuranceProvider);
    m_db.addPatient(patient);
}

void ClinicManager::registerEmergencyPatient(const std::string& name, int age, const std::string& contact,
                                             int triageLevel, const std::string& incidentDesc,
                                             bool requiresTrauma, const std::string& customId) {
    std::string id = customId.empty() ? utils::IDGenerator::instance().generatePatientId() : customId;
    auto patient = std::make_shared<model::EmergencyPatient>(id, name, age, contact, triageLevel, incidentDesc, requiresTrauma);
    m_db.addPatient(patient);
}

void ClinicManager::registerFollowUpPatient(const std::string& name, int age, const std::string& contact,
                                            const std::string& primaryDoctor, const std::string& diagnosis,
                                            int intervalDays, const std::string& customId) {
    std::string id = customId.empty() ? utils::IDGenerator::instance().generatePatientId() : customId;
    auto patient = std::make_shared<model::FollowUpPatient>(id, name, age, contact, primaryDoctor, diagnosis, intervalDays);
    m_db.addPatient(patient);
}

bool ClinicManager::updatePatient(std::shared_ptr<model::PatientRecord> patient) {
    return m_db.updatePatient(patient);
}

bool ClinicManager::deletePatient(const std::string& id) {
    return m_db.deletePatient(id);
}

std::shared_ptr<model::PatientRecord> ClinicManager::findPatientById(const std::string& id) {
    return m_db.getPatientById(id);
}

void ClinicManager::enqueuePatient(const std::string& patientId) {
    m_db.enqueueWaitingPatient(patientId);
}

std::shared_ptr<model::PatientRecord> ClinicManager::callNextPatient() {
    std::string nextId = m_db.dequeueWaitingPatient();
    return m_db.getPatientById(nextId);
}

std::vector<std::shared_ptr<model::PatientRecord>> ClinicManager::getWaitingQueuePatients() const {
    std::vector<std::shared_ptr<model::PatientRecord>> result;
    for (const auto& id : m_db.getWaitingQueueSnapshot()) {
        for (const auto& p : m_db.getPatients()) {
            if (p && p->getId() == id) {
                result.push_back(p);
                break;
            }
        }
    }
    return result;
}

void ClinicManager::sortPatients(SortAlgorithm algo, SortField field, bool ascending) {
    auto& list = m_db.getPatients();
    switch (algo) {
        case SortAlgorithm::SelectionSort:
            SearchEngine::selectionSort(list, field, ascending);
            break;
        case SortAlgorithm::QuickSort:
            SearchEngine::quickSort(list, field, ascending);
            break;
        case SortAlgorithm::InsertionSort:
            SearchEngine::insertionSort(list, field, ascending);
            break;
    }
}

std::shared_ptr<model::PatientRecord> ClinicManager::searchPatient(SearchAlgorithm algo,
                                                                   const std::string& query,
                                                                   SortField field) {
    const auto& list = m_db.getPatients();
    if (list.empty()) return nullptr;

    int idx = -1;
    switch (algo) {
        case SearchAlgorithm::LinearSearch:
            idx = SearchEngine::linearSearchById(list, query);
            break;
        case SearchAlgorithm::BinarySearch:
            idx = SearchEngine::binarySearch(list, query, field);
            break;
        case SearchAlgorithm::RecursiveSearch:
            idx = SearchEngine::recursiveSearchByName(list, query, 0, static_cast<int>(list.size()) - 1);
            break;
    }

    if (idx >= 0 && idx < static_cast<int>(list.size())) {
        m_db.recordAccess(list[idx]);
        return list[idx];
    }
    return nullptr;
}

std::vector<std::shared_ptr<model::PatientRecord>> ClinicManager::getMostAttendedPatients(size_t limit) {
    std::vector<std::shared_ptr<model::PatientRecord>> copy = m_db.getPatients();
    // Use manual QuickSort descending by Appointment Count
    SearchEngine::quickSort(copy, SortField::AppointmentCount, /*ascending=*/false);
    if (copy.size() > limit) {
        copy.resize(limit);
    }
    return copy;
}

std::vector<std::shared_ptr<model::PatientRecord>> ClinicManager::getPatientsWithPendingAppointments() {
    std::vector<std::shared_ptr<model::PatientRecord>> result;
    for (const auto& p : m_db.getPatients()) {
        if (p && p->getPendingAppointmentCount() > 0) {
            result.push_back(p);
        }
    }
    return result;
}

CategoryDistribution ClinicManager::getCategoryDistribution() const {
    CategoryDistribution dist;
    for (const auto& p : m_db.getPatients()) {
        if (!p) continue;
        std::string type = p->getPatientType();
        if (type == "General") dist.generalCount++;
        else if (type == "Emergency") dist.emergencyCount++;
        else if (type == "Follow-Up") dist.followUpCount++;
    }
    return dist;
}

DailyAppointmentSummary ClinicManager::getDailyAppointmentSummary(const std::string& datePrefix) const {
    return m_appointmentController.getDailySummary(datePrefix);
}

void ClinicManager::saveClinicData(const std::string& filePath) {
    utils::FileHandler::saveToJson(m_db, filePath);
}

void ClinicManager::loadClinicData(const std::string& filePath) {
    utils::FileHandler::loadFromJson(m_db, filePath);
}

void ClinicManager::exportRosterCsv(const std::string& filePath) {
    utils::FileHandler::exportToCsv(m_db, filePath);
}

std::string ClinicManager::undoLastOperation() {
    return m_db.undoLastAction();
}

void ClinicManager::loadSampleDemonstrationData() {
    m_db.clearAll();
    utils::IDGenerator::instance().reset();

    registerGeneralPatient("Alice Johnson", 29, "+1-555-0101", 45.0, "CampusCare Gold", "P1001");
    registerEmergencyPatient("Brian Miller", 42, "+1-555-0102", 1, "Acute chest trauma", true, "P1002");
    registerFollowUpPatient("Catherine Davis", 67, "+1-555-0103", "Dr. Vance", "Post-op Hip Arthroscopy", 7, "P1003");
    registerGeneralPatient("David Okafor", 21, "+1-555-0104", 30.0, "Student Health Basic", "P1004");
    registerEmergencyPatient("Elena Rostova", 34, "+1-555-0105", 2, "Severe allergic anaphylaxis", false, "P1005");
    registerFollowUpPatient("Franklin Adebayo", 53, "+1-555-0106", "Dr. Adegoke", "Type II Diabetes Review", 30, "P1006");

    // Add appointments
    m_appointmentController.scheduleAppointment("P1001", "2026-11-12 09:30", "Family Medicine", "Dr. Vance", "Annual Physical Exam");
    m_appointmentController.scheduleAppointment("P1001", "2026-10-15 14:00", "Dentistry", "Dr. Lee", "Routine Prophylaxis");
    m_appointmentController.completeAppointment("P1001", "APT2002");

    m_appointmentController.scheduleAppointment("P1003", "2026-11-14 11:00", "Orthopedics", "Dr. Vance", "Suture removal & mobility check");
    m_appointmentController.scheduleAppointment("P1004", "2026-11-13 15:30", "Optometry", "Dr. Chen", "Vision blurriness check");
    m_appointmentController.scheduleAppointment("P1006", "2026-11-20 08:30", "Endocrinology", "Dr. Adegoke", "HbA1c quarterly review");

    // Enqueue in consultation queue
    enqueuePatient("P1002"); // Emergency first
    enqueuePatient("P1001");
    enqueuePatient("P1004");
}

} // namespace scapms::controller
