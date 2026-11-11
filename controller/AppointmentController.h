#pragma once

#include "../model/ClinicDatabase.h"
#include "../model/Appointment.h"
#include <string>
#include <vector>
#include <memory>

namespace scapms::controller {

struct DailyAppointmentSummary {
    int total{0};
    int completed{0};
    int pending{0};
    int cancelled{0};
};

class AppointmentController {
public:
    explicit AppointmentController(model::ClinicDatabase& db);

    // Polymorphic appointment operations
    bool scheduleAppointment(const std::string& patientId,
                             const std::string& dateTime,
                             const std::string& department,
                             const std::string& doctor,
                             const std::string& reason);

    bool cancelAppointment(const std::string& patientId, const std::string& apptId);
    bool completeAppointment(const std::string& patientId, const std::string& apptId);

    std::vector<model::Appointment> getAllAppointments() const;
    std::vector<model::Appointment> getAppointmentsForPatient(const std::string& patientId) const;
    DailyAppointmentSummary getDailySummary(const std::string& datePrefix = "") const;

private:
    model::ClinicDatabase& m_db;
};

} // namespace scapms::controller
