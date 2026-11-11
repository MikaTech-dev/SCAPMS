#include "GeneralPatient.h"
#include <sstream>
#include <iomanip>

namespace scapms::model {

GeneralPatient::GeneralPatient(std::string id,
                               std::string name,
                               int age,
                               std::string contact,
                               double clinicFee,
                               std::string insuranceProvider)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact))
    , m_clinicFee(clinicFee)
    , m_insuranceProvider(std::move(insuranceProvider)) {}

std::string GeneralPatient::displayDetails() const {
    std::ostringstream oss;
    oss << "[General Patient] ID: " << m_id
        << " | Name: " << m_name
        << " | Age: " << m_age
        << " | Contact: " << m_contact
        << " | Consultation Fee: $" << std::fixed << std::setprecision(2) << m_clinicFee
        << " | Insurance: " << m_insuranceProvider
        << " | Total Appts: " << m_appointments.size();
    return oss.str();
}

std::unique_ptr<PatientRecord> GeneralPatient::clone() const {
    auto copy = std::make_unique<GeneralPatient>(m_id, m_name, m_age, m_contact, m_clinicFee, m_insuranceProvider);
    copy->getAppointments() = m_appointments;
    return copy;
}

bool GeneralPatient::scheduleAppointment(const Appointment& appt) {
    addAppointment(appt);
    return true;
}

bool GeneralPatient::cancelAppointment(const std::string& apptId) {
    return PatientRecord::cancelAppointment(apptId);
}

} // namespace scapms::model
