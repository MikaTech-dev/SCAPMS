#include "AppointmentController.h"
#include "../model/Appointable.h"
#include "../utils/Exceptions.h"
#include "../utils/IDGenerator.h"

namespace scapms::controller {

AppointmentController::AppointmentController(model::ClinicDatabase& db)
    : m_db(db) {}

bool AppointmentController::scheduleAppointment(const std::string& patientId,
                                                const std::string& dateTime,
                                                const std::string& department,
                                                const std::string& doctor,
                                                const std::string& reason) {
    auto patient = m_db.getPatientById(patientId);
    if (!patient) {
        throw utils::RecordNotFoundException("Patient ID: " + patientId);
    }

    if (dateTime.empty()) {
        throw utils::InvalidAppointmentDataException("Appointment date/time cannot be blank.");
    }
    if (doctor.empty()) {
        throw utils::InvalidAppointmentDataException("Attending doctor must be specified.");
    }

    std::string apptId = utils::IDGenerator::instance().generateAppointmentId();
    model::Appointment appt(apptId, patientId, dateTime, department, doctor, reason, model::AppointmentStatus::Pending);

    // Demonstrate polymorphic interface dispatch
    if (auto* appointable = dynamic_cast<model::Appointable*>(patient.get())) {
        appointable->scheduleAppointment(appt);
    } else {
        // Direct addition for non-Appointable patient types (e.g. emergency triage)
        patient->addAppointment(appt);
    }

    // Push undo action
    model::AdminAction action;
    action.type = model::ActionType::AddAppointment;
    action.targetId = patientId;
    action.appointmentSnapshot = appt;
    action.description = "Scheduled appointment " + apptId + " for " + patient->getName();
    m_db.pushUndoAction(std::move(action));

    return true;
}

bool AppointmentController::cancelAppointment(const std::string& patientId, const std::string& apptId) {
    auto patient = m_db.getPatientById(patientId);
    if (!patient) {
        throw utils::RecordNotFoundException("Patient ID: " + patientId);
    }

    // Find appointment snapshot for undo
    model::Appointment targetSnapshot;
    bool found = false;
    for (const auto& appt : patient->getAppointments()) {
        if (appt.getAppointmentId() == apptId) {
            targetSnapshot = appt;
            found = true;
            break;
        }
    }

    if (!found) {
        throw utils::RecordNotFoundException("Appointment ID: " + apptId);
    }

    bool success = false;
    if (auto* appointable = dynamic_cast<model::Appointable*>(patient.get())) {
        success = appointable->cancelAppointment(apptId);
    } else {
        success = patient->cancelAppointment(apptId);
    }

    if (success) {
        model::AdminAction action;
        action.type = model::ActionType::RemoveAppointment;
        action.targetId = patientId;
        action.appointmentSnapshot = targetSnapshot;
        action.description = "Cancelled appointment " + apptId;
        m_db.pushUndoAction(std::move(action));
    }

    return success;
}

bool AppointmentController::completeAppointment(const std::string& patientId, const std::string& apptId) {
    auto patient = m_db.getPatientById(patientId);
    if (!patient) {
        throw utils::RecordNotFoundException("Patient ID: " + patientId);
    }

    return patient->completeAppointment(apptId);
}

std::vector<model::Appointment> AppointmentController::getAllAppointments() const {
    std::vector<model::Appointment> list;
    for (const auto& p : m_db.getPatients()) {
        if (!p) continue;
        for (const auto& a : p->getAppointments()) {
            list.push_back(a);
        }
    }
    return list;
}

std::vector<model::Appointment> AppointmentController::getAppointmentsForPatient(const std::string& patientId) const {
    auto patient = m_db.getPatientById(patientId);
    if (patient) {
        return patient->getAppointments();
    }
    return {};
}

DailyAppointmentSummary AppointmentController::getDailySummary(const std::string& datePrefix) const {
    DailyAppointmentSummary summary;
    for (const auto& p : m_db.getPatients()) {
        if (!p) continue;
        for (const auto& a : p->getAppointments()) {
            if (datePrefix.empty() || a.getDateTime().rfind(datePrefix, 0) == 0) {
                summary.total++;
                if (a.getStatus() == model::AppointmentStatus::Completed) {
                    summary.completed++;
                } else if (a.getStatus() == model::AppointmentStatus::Pending) {
                    summary.pending++;
                } else if (a.getStatus() == model::AppointmentStatus::Cancelled) {
                    summary.cancelled++;
                }
            }
        }
    }
    return summary;
}

} // namespace scapms::controller
