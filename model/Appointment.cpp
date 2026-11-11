#include "Appointment.h"
#include <algorithm>

namespace scapms::model {

Appointment::Appointment() = default;

Appointment::Appointment(std::string appointmentId,
                         std::string patientId,
                         std::string dateTime,
                         std::string department,
                         std::string doctor,
                         std::string reason,
                         AppointmentStatus status)
    : m_appointmentId(std::move(appointmentId))
    , m_patientId(std::move(patientId))
    , m_dateTime(std::move(dateTime))
    , m_department(std::move(department))
    , m_doctor(std::move(doctor))
    , m_reason(std::move(reason))
    , m_status(status) {}

std::string Appointment::getStatusString() const {
    return statusToString(m_status);
}

std::string Appointment::statusToString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::Pending:
            return "Pending";
        case AppointmentStatus::Completed:
            return "Completed";
        case AppointmentStatus::Cancelled:
            return "Cancelled";
    }
    return "Unknown";
}

AppointmentStatus Appointment::stringToStatus(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "completed") {
        return AppointmentStatus::Completed;
    } else if (lower == "cancelled" || lower == "canceled") {
        return AppointmentStatus::Cancelled;
    }
    return AppointmentStatus::Pending;
}

} // namespace scapms::model
