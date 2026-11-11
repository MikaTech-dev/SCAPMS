#pragma once

#include "../model/ClinicDatabase.h"
#include <string>

namespace scapms::utils {

class FileHandler {
public:
    static void saveToJson(const model::ClinicDatabase& db, const std::string& filePath);
    static void loadFromJson(model::ClinicDatabase& db, const std::string& filePath);

    static void exportToCsv(const model::ClinicDatabase& db, const std::string& filePath);
    static void exportAppointmentsToCsv(const model::ClinicDatabase& db, const std::string& filePath);
};

} // namespace scapms::utils
