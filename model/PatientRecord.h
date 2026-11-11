#pragma once

#include "Appointment.h"
#include <string>
#include <vector>
#include <memory>

namespace scapms::model {

class PatientRecord {
public:
    PatientRecord(std::string id, std::string name, int age, std::string contact);
    virtual ~PatientRecord() = default;

    // Pure virtual methods
    virtual std::string getPatientType() const = 0;
    virtual std::string displayDetails() const = 0;
    virtual std::unique_ptr<PatientRecord> clone() const = 0;

    // Getters and Setters
    const std::string& getId() const { return m_id; }
    void setId(const std::string& id);

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name);

    int getAge() const { return m_age; }
    void setAge(int age);

    const std::string& getContact() const { return m_contact; }
    void setContact(const std::string& contact);

    // Appointment management
    const std::vector<Appointment>& getAppointments() const { return m_appointments; }
    std::vector<Appointment>& getAppointments() { return m_appointments; }
    void addAppointment(const Appointment& appt);
    bool cancelAppointment(const std::string& apptId);
    bool completeAppointment(const std::string& apptId);
    size_t getCompletedAppointmentCount() const;
    size_t getPendingAppointmentCount() const;

protected:
    std::string m_id;
    std::string m_name;
    int m_age{0};
    std::string m_contact;
    std::vector<Appointment> m_appointments;
};

} // namespace scapms::model
