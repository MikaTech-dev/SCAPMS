#pragma once

#include "PatientRecord.h"
#include "Appointable.h"

namespace scapms::model {

class FollowUpPatient : public PatientRecord, public Appointable {
public:
    FollowUpPatient(std::string id,
                    std::string name,
                    int age,
                    std::string contact,
                    std::string primaryDoctor = "Dr. Smith",
                    std::string previousDiagnosis = "Hypertension",
                    int followUpIntervalDays = 14);

    // Overrides from PatientRecord
    std::string getPatientType() const override { return "Follow-Up"; }
    std::string displayDetails() const override;
    std::unique_ptr<PatientRecord> clone() const override;

    // Overrides from Appointable
    bool scheduleAppointment(const Appointment& appt) override;
    bool cancelAppointment(const std::string& apptId) override;

    const std::string& getPrimaryDoctor() const { return m_primaryDoctor; }
    void setPrimaryDoctor(const std::string& doc) { m_primaryDoctor = doc; }

    const std::string& getPreviousDiagnosis() const { return m_previousDiagnosis; }
    void setPreviousDiagnosis(const std::string& diag) { m_previousDiagnosis = diag; }

    int getFollowUpIntervalDays() const { return m_followUpIntervalDays; }
    void setFollowUpIntervalDays(int days) { m_followUpIntervalDays = days; }

private:
    std::string m_primaryDoctor;
    std::string m_previousDiagnosis;
    int m_followUpIntervalDays{14};
};

} // namespace scapms::model
