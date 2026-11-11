#include "ClinicDatabase.h"
#include "../utils/Exceptions.h"
#include <algorithm>

namespace scapms::model {

ClinicDatabase::ClinicDatabase() {
    m_recentPatients.fill(nullptr);
}

void ClinicDatabase::addPatient(std::shared_ptr<PatientRecord> patient, bool recordUndo) {
    if (!patient) {
        throw utils::InvalidPatientDataException("Attempted to add a null patient pointer.");
    }

    if (patientExists(patient->getId())) {
        throw utils::DuplicateIDException(patient->getId());
    }

    m_patients.push_back(patient);
    recordAccess(patient);

    if (recordUndo) {
        AdminAction action;
        action.type = ActionType::AddPatient;
        action.targetId = patient->getId();
        action.patientSnapshot = patient;
        action.description = "Registered patient " + patient->getName() + " (" + patient->getId() + ")";
        pushUndoAction(std::move(action));
    }
}

bool ClinicDatabase::updatePatient(std::shared_ptr<PatientRecord> updatedPatient, bool recordUndo) {
    if (!updatedPatient) return false;

    for (size_t i = 0; i < m_patients.size(); ++i) {
        if (m_patients[i]->getId() == updatedPatient->getId()) {
            if (recordUndo) {
                AdminAction action;
                action.type = ActionType::ModifyPatient;
                action.targetId = updatedPatient->getId();
                action.patientSnapshot = std::shared_ptr<PatientRecord>(m_patients[i]->clone().release());
                action.description = "Updated patient " + updatedPatient->getName() + " (" + updatedPatient->getId() + ")";
                pushUndoAction(std::move(action));
            }
            m_patients[i] = updatedPatient;
            recordAccess(updatedPatient);
            return true;
        }
    }
    throw utils::RecordNotFoundException("Patient ID: " + updatedPatient->getId());
}

bool ClinicDatabase::deletePatient(const std::string& id, bool recordUndo) {
    for (auto it = m_patients.begin(); it != m_patients.end(); ++it) {
        if ((*it)->getId() == id) {
            if (recordUndo) {
                AdminAction action;
                action.type = ActionType::DeletePatient;
                action.targetId = id;
                action.patientSnapshot = std::shared_ptr<PatientRecord>((*it)->clone().release());
                action.description = "Deleted patient " + (*it)->getName() + " (" + id + ")";
                pushUndoAction(std::move(action));
            }

            // Remove from cache if present
            for (auto& cached : m_recentPatients) {
                if (cached && cached->getId() == id) {
                    cached = nullptr;
                }
            }

            m_patients.erase(it);
            return true;
        }
    }
    throw utils::RecordNotFoundException("Patient ID: " + id);
}

std::shared_ptr<PatientRecord> ClinicDatabase::getPatientById(const std::string& id) {
    // Check cache first for rapid lookup
    for (const auto& cached : m_recentPatients) {
        if (cached && cached->getId() == id) {
            return cached;
        }
    }

    for (const auto& p : m_patients) {
        if (p->getId() == id) {
            recordAccess(p);
            return p;
        }
    }
    return nullptr;
}

bool ClinicDatabase::patientExists(const std::string& id) const {
    for (const auto& p : m_patients) {
        if (p->getId() == id) {
            return true;
        }
    }
    return false;
}

void ClinicDatabase::enqueueWaitingPatient(const std::string& patientId) {
    if (!patientExists(patientId)) {
        throw utils::RecordNotFoundException("Cannot enqueue non-existent patient: " + patientId);
    }
    m_waitingQueue.push(patientId);
}

std::string ClinicDatabase::dequeueWaitingPatient() {
    if (m_waitingQueue.empty()) {
        throw utils::QueueEmptyException();
    }
    std::string nextId = m_waitingQueue.front();
    m_waitingQueue.pop();
    return nextId;
}

std::vector<std::string> ClinicDatabase::getWaitingQueueSnapshot() const {
    std::vector<std::string> snapshot;
    std::queue<std::string> copy = m_waitingQueue;
    while (!copy.empty()) {
        snapshot.push_back(copy.front());
        copy.pop();
    }
    return snapshot;
}

void ClinicDatabase::pushUndoAction(AdminAction action) {
    m_undoStack.push(std::move(action));
}

std::string ClinicDatabase::undoLastAction() {
    if (m_undoStack.empty()) {
        return "No administrative operations to undo.";
    }

    AdminAction action = std::move(m_undoStack.top());
    m_undoStack.pop();

    switch (action.type) {
        case ActionType::AddPatient: {
            deletePatient(action.targetId, /*recordUndo=*/false);
            return "Reversed registration of patient " + action.targetId;
        }
        case ActionType::DeletePatient: {
            if (action.patientSnapshot) {
                m_patients.push_back(action.patientSnapshot);
                recordAccess(action.patientSnapshot);
                return "Restored deleted patient " + action.patientSnapshot->getName() + " (" + action.targetId + ")";
            }
            break;
        }
        case ActionType::ModifyPatient: {
            if (action.patientSnapshot) {
                for (auto& p : m_patients) {
                    if (p->getId() == action.targetId) {
                        p = action.patientSnapshot;
                        recordAccess(p);
                        return "Restored previous data for patient " + action.targetId;
                    }
                }
            }
            break;
        }
        case ActionType::AddAppointment: {
            auto patient = getPatientById(action.targetId);
            if (patient) {
                patient->cancelAppointment(action.appointmentSnapshot.getAppointmentId());
                return "Cancelled appointment " + action.appointmentSnapshot.getAppointmentId();
            }
            break;
        }
        case ActionType::RemoveAppointment: {
            auto patient = getPatientById(action.targetId);
            if (patient) {
                patient->addAppointment(action.appointmentSnapshot);
                return "Restored appointment " + action.appointmentSnapshot.getAppointmentId();
            }
            break;
        }
    }
    return "Undone: " + action.description;
}

void ClinicDatabase::recordAccess(std::shared_ptr<PatientRecord> patient) {
    if (!patient) return;

    // Check if patient is already in cache
    size_t foundIdx = CACHE_SIZE;
    for (size_t i = 0; i < CACHE_SIZE; ++i) {
        if (m_recentPatients[i] && m_recentPatients[i]->getId() == patient->getId()) {
            foundIdx = i;
            break;
        }
    }

    // Shift elements down
    size_t limit = (foundIdx < CACHE_SIZE) ? foundIdx : CACHE_SIZE - 1;
    for (size_t i = limit; i > 0; --i) {
        m_recentPatients[i] = m_recentPatients[i - 1];
    }
    m_recentPatients[0] = patient;
}

void ClinicDatabase::clearAll() {
    m_patients.clear();
    while (!m_waitingQueue.empty()) m_waitingQueue.pop();
    while (!m_undoStack.empty()) m_undoStack.pop();
    m_recentPatients.fill(nullptr);
}

} // namespace scapms::model
