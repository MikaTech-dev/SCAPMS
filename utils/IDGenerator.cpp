#include "IDGenerator.h"
#include <sstream>
#include <iomanip>

namespace scapms::utils {

IDGenerator& IDGenerator::instance() {
    static IDGenerator instance;
    return instance;
}

IDGenerator::IDGenerator() = default;

std::string IDGenerator::generatePatientId(const std::string& prefix) {
    uint32_t val = m_patientCounter.fetch_add(1);
    std::ostringstream oss;
    oss << prefix << std::setw(4) << std::setfill('0') << val;
    return oss.str();
}

std::string IDGenerator::generateAppointmentId(const std::string& prefix) {
    uint32_t val = m_appointmentCounter.fetch_add(1);
    std::ostringstream oss;
    oss << prefix << std::setw(4) << std::setfill('0') << val;
    return oss.str();
}

void IDGenerator::updatePatientCounterIfHigher(uint32_t val) {
    uint32_t current = m_patientCounter.load();
    while (val >= current && !m_patientCounter.compare_exchange_weak(current, val + 1)) {
        // loop until updated or val < current
    }
}

void IDGenerator::updateAppointmentCounterIfHigher(uint32_t val) {
    uint32_t current = m_appointmentCounter.load();
    while (val >= current && !m_appointmentCounter.compare_exchange_weak(current, val + 1)) {
        // loop until updated or val < current
    }
}

void IDGenerator::reset() {
    m_patientCounter.store(1001);
    m_appointmentCounter.store(2001);
}

} // namespace scapms::utils
