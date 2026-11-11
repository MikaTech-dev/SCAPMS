#pragma once

#include <stdexcept>
#include <string>

namespace scapms::utils {

class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const std::string& message)
        : std::runtime_error(message) {}
};

class DuplicateIDException : public ClinicException {
public:
    explicit DuplicateIDException(const std::string& id)
        : ClinicException("Error: Patient ID already exists (" + id + ").") {}
};

class InvalidAgeException : public ClinicException {
public:
    explicit InvalidAgeException(int age)
        : ClinicException("Error: Patient age must be between 0 and 120 (provided: " + std::to_string(age) + ").") {}
};

class InvalidPatientDataException : public ClinicException {
public:
    explicit InvalidPatientDataException(const std::string& detail)
        : ClinicException("Error: Invalid patient information: " + detail) {}
};

class InvalidAppointmentDataException : public ClinicException {
public:
    explicit InvalidAppointmentDataException(const std::string& detail)
        : ClinicException("Error: Invalid appointment data: " + detail) {}
};

class RecordNotFoundException : public ClinicException {
public:
    explicit RecordNotFoundException(const std::string& identifier)
        : ClinicException("Error: Record not found (" + identifier + ").") {}
};

class FileIOException : public ClinicException {
public:
    explicit FileIOException(const std::string& detail)
        : ClinicException("Error: File operation failed: " + detail) {}
};

class InvalidSearchException : public ClinicException {
public:
    explicit InvalidSearchException(const std::string& detail)
        : ClinicException("Error: Invalid search input: " + detail) {}
};

class QueueEmptyException : public ClinicException {
public:
    QueueEmptyException()
        : ClinicException("Error: Consultation waiting queue is empty.") {}
};

} // namespace scapms::utils
