#include "FollowUpPatient.h"
#include <sstream>

namespace scapms::model {

FollowUpPatient::FollowUpPatient(std::string id,
                                 std::string name,
                                 int age,
                                 std::string contact,
                                 std::string primaryDoctor,
                                 std::string previousDiagnosis,
                                 int followUpIntervalDays)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact))
    , m_primaryDoctor(std::move(primaryDoctor))
    , m_previousDiagnosis(std::move(previousDiagnosis))
    , m_followUpIntervalDays(followUpIntervalDays) {}

std::string FollowUpPatient::displayDetails() const {
    std::ostringstream oss;
    oss << "[Follow-Up Patient] ID: " << m_id
        << " | Name: " << m_name
        << " | Age: " << m_age
        << " | Contact: " << m_contact
        << " | Primary Doctor: " << m_primaryDoctor
        << " | Diagnosis: " << m_previousDiagnosis
        << " | Interval: " << m_followUpIntervalDays << " days";
    return oss.str();
}

std::unique_ptr<PatientRecord> FollowUpPatient::clone() const {
    auto copy = std::make_unique<FollowUpPatient>(m_id, m_name, m_age, m_contact,
                                                  m_primaryDoctor, m_previousDiagnosis,
                                                  m_followUpIntervalDays);
    copy->getAppointments() = m_appointments;
    return copy;
}

bool FollowUpPatient::scheduleAppointment(const Appointment& appt) {
    addAppointment(appt);
    return true;
}

bool FollowUpPatient::cancelAppointment(const std::string& apptId) {
    return PatientRecord::cancelAppointment(apptId);
}

} // namespace scapms::model
