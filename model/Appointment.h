#pragma once

#include <string>

namespace scapms::model {

enum class AppointmentStatus {
    Pending,
    Completed,
    Cancelled
};

class Appointment {
public:
    Appointment();
    Appointment(std::string appointmentId,
                std::string patientId,
                std::string dateTime,
                std::string department,
                std::string doctor,
                std::string reason,
                AppointmentStatus status = AppointmentStatus::Pending);

    const std::string& getAppointmentId() const { return m_appointmentId; }
    void setAppointmentId(const std::string& id) { m_appointmentId = id; }

    const std::string& getPatientId() const { return m_patientId; }
    void setPatientId(const std::string& id) { m_patientId = id; }

    const std::string& getDateTime() const { return m_dateTime; }
    void setDateTime(const std::string& dt) { m_dateTime = dt; }

    const std::string& getDepartment() const { return m_department; }
    void setDepartment(const std::string& dept) { m_department = dept; }

    const std::string& getDoctor() const { return m_doctor; }
    void setDoctor(const std::string& doc) { m_doctor = doc; }

    const std::string& getReason() const { return m_reason; }
    void setReason(const std::string& reason) { m_reason = reason; }

    AppointmentStatus getStatus() const { return m_status; }
    void setStatus(AppointmentStatus status) { m_status = status; }

    std::string getStatusString() const;
    static std::string statusToString(AppointmentStatus status);
    static AppointmentStatus stringToStatus(const std::string& str);

private:
    std::string m_appointmentId;
    std::string m_patientId;
    std::string m_dateTime;
    std::string m_department;
    std::string m_doctor;
    std::string m_reason;
    AppointmentStatus m_status{AppointmentStatus::Pending};
};

} // namespace scapms::model
