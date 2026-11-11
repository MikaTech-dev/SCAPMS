#pragma once

#include "PatientRecord.h"
#include "Appointable.h"

namespace scapms::model {

class GeneralPatient : public PatientRecord, public Appointable {
public:
    GeneralPatient(std::string id,
                   std::string name,
                   int age,
                   std::string contact,
                   double clinicFee = 50.0,
                   std::string insuranceProvider = "Standard");

    // Overrides from PatientRecord
    std::string getPatientType() const override { return "General"; }
    std::string displayDetails() const override;
    std::unique_ptr<PatientRecord> clone() const override;

    // Overrides from Appointable
    bool scheduleAppointment(const Appointment& appt) override;
    bool cancelAppointment(const std::string& apptId) override;

    double getClinicFee() const { return m_clinicFee; }
    void setClinicFee(double fee) { m_clinicFee = fee; }

    const std::string& getInsuranceProvider() const { return m_insuranceProvider; }
    void setInsuranceProvider(const std::string& provider) { m_insuranceProvider = provider; }

private:
    double m_clinicFee{50.0};
    std::string m_insuranceProvider;
};

} // namespace scapms::model
