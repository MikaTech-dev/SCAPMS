#pragma once

#include "../model/ClinicDatabase.h"
#include "AppointmentController.h"
#include "SearchEngine.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace scapms::controller {

struct CategoryDistribution {
    int generalCount{0};
    int emergencyCount{0};
    int followUpCount{0};
};

class ClinicManager {
public:
    ClinicManager();
    ~ClinicManager() = default;

    // Database access
    model::ClinicDatabase& getDatabase() { return m_db; }
    const model::ClinicDatabase& getDatabase() const { return m_db; }
    AppointmentController& getAppointmentController() { return m_appointmentController; }

    // Patient lifecycle operations
    void registerGeneralPatient(const std::string& name, int age, const std::string& contact,
                                double clinicFee, const std::string& insuranceProvider,
                                const std::string& customId = "");

    void registerEmergencyPatient(const std::string& name, int age, const std::string& contact,
                                  int triageLevel, const std::string& incidentDesc,
                                  bool requiresTrauma, const std::string& customId = "");

    void registerFollowUpPatient(const std::string& name, int age, const std::string& contact,
                                 const std::string& primaryDoctor, const std::string& diagnosis,
                                 int intervalDays, const std::string& customId = "");

    bool updatePatient(std::shared_ptr<model::PatientRecord> patient);
    bool deletePatient(const std::string& id);
    std::shared_ptr<model::PatientRecord> findPatientById(const std::string& id);

    // Consultation Waiting Queue operations
    void enqueuePatient(const std::string& patientId);
    std::shared_ptr<model::PatientRecord> callNextPatient();
    std::vector<std::shared_ptr<model::PatientRecord>> getWaitingQueuePatients() const;

    // Search and Sort facade
    void sortPatients(SortAlgorithm algo, SortField field, bool ascending = true);
    std::shared_ptr<model::PatientRecord> searchPatient(SearchAlgorithm algo,
                                                        const std::string& query,
                                                        SortField field = SortField::ID);

    // Analytical Reports
    std::vector<std::shared_ptr<model::PatientRecord>> getMostAttendedPatients(size_t limit = 10);
    std::vector<std::shared_ptr<model::PatientRecord>> getPatientsWithPendingAppointments();
    CategoryDistribution getCategoryDistribution() const;
    DailyAppointmentSummary getDailyAppointmentSummary(const std::string& datePrefix = "") const;

    // Persistence & Undo
    void saveClinicData(const std::string& filePath);
    void loadClinicData(const std::string& filePath);
    void exportRosterCsv(const std::string& filePath);
    std::string undoLastOperation();

    // Seed sample data for testing and demonstration
    void loadSampleDemonstrationData();

private:
    model::ClinicDatabase m_db;
    AppointmentController m_appointmentController;
};

} // namespace scapms::controller
