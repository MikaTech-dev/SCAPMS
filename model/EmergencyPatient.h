#pragma once

#include "PatientRecord.h"

namespace scapms::model {

class EmergencyPatient : public PatientRecord {
public:
    EmergencyPatient(std::string id,
                     std::string name,
                     int age,
                     std::string contact,
                     int triageLevel = 1,
                     std::string incidentDescription = "Acute condition",
                     bool requiresTraumaUnit = false);

    // Overrides from PatientRecord
    std::string getPatientType() const override { return "Emergency"; }
    std::string displayDetails() const override;
    std::unique_ptr<PatientRecord> clone() const override;

    int getTriageLevel() const { return m_triageLevel; }
    void setTriageLevel(int level) { m_triageLevel = level; }

    const std::string& getIncidentDescription() const { return m_incidentDescription; }
    void setIncidentDescription(const std::string& desc) { m_incidentDescription = desc; }

    bool requiresTraumaUnit() const { return m_requiresTraumaUnit; }
    void setRequiresTraumaUnit(bool req) { m_requiresTraumaUnit = req; }

private:
    int m_triageLevel{1}; // 1 = Critical, 5 = Non-urgent
    std::string m_incidentDescription;
    bool m_requiresTraumaUnit{false};
};

} // namespace scapms::model
