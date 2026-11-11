#pragma once

#include "Appointment.h"
#include <string>

namespace scapms::model {

class Appointable {
public:
    virtual ~Appointable() = default;

    virtual bool scheduleAppointment(const Appointment& appt) = 0;
    virtual bool cancelAppointment(const std::string& apptId) = 0;
};

} // namespace scapms::model
