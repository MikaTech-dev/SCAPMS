#pragma once

#include "PatientRecord.h"
#include <vector>
#include <queue>
#include <stack>
#include <array>
#include <memory>
#include <string>
#include <optional>

namespace scapms::model {

enum class ActionType {
    AddPatient,
    DeletePatient,
    ModifyPatient,
    AddAppointment,
    RemoveAppointment
};

struct AdminAction {
    ActionType type;
    std::shared_ptr<PatientRecord> patientSnapshot;
    std::string targetId;
    Appointment appointmentSnapshot;
    std::string description;
};

class ClinicDatabase {
public:
    static constexpr size_t CACHE_SIZE = 10;

    ClinicDatabase();
    ~ClinicDatabase() = default;

    // Master Registry (std::vector)
    void addPatient(std::shared_ptr<PatientRecord> patient, bool recordUndo = true);
    bool updatePatient(std::shared_ptr<PatientRecord> updatedPatient, bool recordUndo = true);
    bool deletePatient(const std::string& id, bool recordUndo = true);
    std::shared_ptr<PatientRecord> getPatientById(const std::string& id);
    const std::vector<std::shared_ptr<PatientRecord>>& getPatients() const { return m_patients; }
    std::vector<std::shared_ptr<PatientRecord>>& getPatients() { return m_patients; }
    bool patientExists(const std::string& id) const;

    // Waiting Queue (std::queue)
    void enqueueWaitingPatient(const std::string& patientId);
    std::string dequeueWaitingPatient();
    bool isQueueEmpty() const { return m_waitingQueue.empty(); }
    size_t getQueueSize() const { return m_waitingQueue.size(); }
    std::vector<std::string> getWaitingQueueSnapshot() const;

    // Administrative Undo (std::stack)
    bool hasUndo() const { return !m_undoStack.empty(); }
    std::string undoLastAction();
    void pushUndoAction(AdminAction action);

    // Frequently Accessed Cache (fixed-size array)
    void recordAccess(std::shared_ptr<PatientRecord> patient);
    const std::array<std::shared_ptr<PatientRecord>, CACHE_SIZE>& getRecentPatients() const {
        return m_recentPatients;
    }

    void clearAll();

private:
    std::vector<std::shared_ptr<PatientRecord>> m_patients;
    std::queue<std::string> m_waitingQueue;
    std::stack<AdminAction> m_undoStack;
    std::array<std::shared_ptr<PatientRecord>, CACHE_SIZE> m_recentPatients{};
};

} // namespace scapms::model
