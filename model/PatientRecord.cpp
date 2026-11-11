#include "PatientRecord.h"
#include "../utils/Exceptions.h"

namespace scapms::model {

PatientRecord::PatientRecord(std::string id, std::string name, int age, std::string contact)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_age(age)
    , m_contact(std::move(contact)) {
    if (m_id.empty()) {
        throw utils::InvalidPatientDataException("Patient ID cannot be empty.");
    }
    if (m_name.empty()) {
        throw utils::InvalidPatientDataException("Patient Name cannot be empty.");
    }
    if (m_age < 0 || m_age > 120) {
        throw utils::InvalidAgeException(m_age);
    }
}

void PatientRecord::setId(const std::string& id) {
    if (id.empty()) {
        throw utils::InvalidPatientDataException("Patient ID cannot be empty.");
    }
    m_id = id;
}

void PatientRecord::setName(const std::string& name) {
    if (name.empty()) {
        throw utils::InvalidPatientDataException("Patient Name cannot be empty.");
    }
    m_name = name;
}

void PatientRecord::setAge(int age) {
    if (age < 0 || age > 120) {
        throw utils::InvalidAgeException(age);
    }
    m_age = age;
}

void PatientRecord::setContact(const std::string& contact) {
    m_contact = contact;
}

void PatientRecord::addAppointment(const Appointment& appt) {
    m_appointments.push_back(appt);
}

bool PatientRecord::cancelAppointment(const std::string& apptId) {
    for (auto& appt : m_appointments) {
        if (appt.getAppointmentId() == apptId) {
            appt.setStatus(AppointmentStatus::Cancelled);
            return true;
        }
    }
    return false;
}

bool PatientRecord::completeAppointment(const std::string& apptId) {
    for (auto& appt : m_appointments) {
        if (appt.getAppointmentId() == apptId) {
            appt.setStatus(AppointmentStatus::Completed);
            return true;
        }
    }
    return false;
}

size_t PatientRecord::getCompletedAppointmentCount() const {
    size_t count = 0;
    for (const auto& appt : m_appointments) {
        if (appt.getStatus() == AppointmentStatus::Completed) {
            count++;
        }
    }
    return count;
}

size_t PatientRecord::getPendingAppointmentCount() const {
    size_t count = 0;
    for (const auto& appt : m_appointments) {
        if (appt.getStatus() == AppointmentStatus::Pending) {
            count++;
        }
    }
    return count;
}

} // namespace scapms::model
