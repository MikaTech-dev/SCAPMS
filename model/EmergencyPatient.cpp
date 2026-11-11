#include "EmergencyPatient.h"
#include <sstream>

namespace scapms::model {

EmergencyPatient::EmergencyPatient(std::string id,
                                   std::string name,
                                   int age,
                                   std::string contact,
                                   int triageLevel,
                                   std::string incidentDescription,
                                   bool requiresTraumaUnit)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact))
    , m_triageLevel(triageLevel)
    , m_incidentDescription(std::move(incidentDescription))
    , m_requiresTraumaUnit(requiresTraumaUnit) {}

std::string EmergencyPatient::displayDetails() const {
    std::ostringstream oss;
    oss << "[Emergency Patient] ID: " << m_id
        << " | Name: " << m_name
        << " | Age: " << m_age
        << " | Contact: " << m_contact
        << " | Triage Severity Level: " << m_triageLevel
        << " | Incident: " << m_incidentDescription
        << " | Trauma Unit: " << (m_requiresTraumaUnit ? "YES" : "NO");
    return oss.str();
}

std::unique_ptr<PatientRecord> EmergencyPatient::clone() const {
    auto copy = std::make_unique<EmergencyPatient>(m_id, m_name, m_age, m_contact,
                                                   m_triageLevel, m_incidentDescription,
                                                   m_requiresTraumaUnit);
    copy->getAppointments() = m_appointments;
    return copy;
}

} // namespace scapms::model
