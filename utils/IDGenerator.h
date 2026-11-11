#pragma once

#include <string>
#include <atomic>
#include <cstdint>

namespace scapms::utils {

class IDGenerator {
public:
    static IDGenerator& instance();

    std::string generatePatientId(const std::string& prefix = "P");
    std::string generateAppointmentId(const std::string& prefix = "APT");

    void updatePatientCounterIfHigher(uint32_t val);
    void updateAppointmentCounterIfHigher(uint32_t val);
    void reset();

private:
    IDGenerator();
    std::atomic<uint32_t> m_patientCounter{1001};
    std::atomic<uint32_t> m_appointmentCounter{2001};
};

} // namespace scapms::utils
