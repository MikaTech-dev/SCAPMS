#include "FileHandler.h"
#include "Exceptions.h"
#include "IDGenerator.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <fstream>
#include <iostream>

namespace scapms::utils {

void FileHandler::saveToJson(const model::ClinicDatabase& db, const std::string& filePath) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw FileIOException("Could not open file for writing: " + filePath + " (" + file.errorString().toStdString() + ")");
    }

    QJsonObject rootObj;
    QJsonArray patientArray;

    for (const auto& patient : db.getPatients()) {
        if (!patient) continue;

        QJsonObject pObj;
        pObj["id"] = QString::fromStdString(patient->getId());
        pObj["name"] = QString::fromStdString(patient->getName());
        pObj["age"] = patient->getAge();
        pObj["contact"] = QString::fromStdString(patient->getContact());
        pObj["type"] = QString::fromStdString(patient->getPatientType());

        if (auto gp = std::dynamic_pointer_cast<model::GeneralPatient>(patient)) {
            pObj["clinicFee"] = gp->getClinicFee();
            pObj["insuranceProvider"] = QString::fromStdString(gp->getInsuranceProvider());
        } else if (auto ep = std::dynamic_pointer_cast<model::EmergencyPatient>(patient)) {
            pObj["triageLevel"] = ep->getTriageLevel();
            pObj["incidentDescription"] = QString::fromStdString(ep->getIncidentDescription());
            pObj["requiresTraumaUnit"] = ep->requiresTraumaUnit();
        } else if (auto fp = std::dynamic_pointer_cast<model::FollowUpPatient>(patient)) {
            pObj["primaryDoctor"] = QString::fromStdString(fp->getPrimaryDoctor());
            pObj["previousDiagnosis"] = QString::fromStdString(fp->getPreviousDiagnosis());
            pObj["followUpIntervalDays"] = fp->getFollowUpIntervalDays();
        }

        // Appointments
        QJsonArray apptArray;
        for (const auto& appt : patient->getAppointments()) {
            QJsonObject aObj;
            aObj["appointmentId"] = QString::fromStdString(appt.getAppointmentId());
            aObj["dateTime"] = QString::fromStdString(appt.getDateTime());
            aObj["department"] = QString::fromStdString(appt.getDepartment());
            aObj["doctor"] = QString::fromStdString(appt.getDoctor());
            aObj["reason"] = QString::fromStdString(appt.getReason());
            aObj["status"] = QString::fromStdString(appt.getStatusString());
            apptArray.append(aObj);
        }
        pObj["appointments"] = apptArray;

        patientArray.append(pObj);
    }
    rootObj["patients"] = patientArray;

    // Save waiting queue
    QJsonArray queueArray;
    for (const auto& id : db.getWaitingQueueSnapshot()) {
        queueArray.append(QString::fromStdString(id));
    }
    rootObj["waitingQueue"] = queueArray;

    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void FileHandler::loadFromJson(model::ClinicDatabase& db, const std::string& filePath) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        throw FileIOException("Could not open file for reading: " + filePath + " (" + file.errorString().toStdString() + ")");
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        throw FileIOException("JSON parsing error: " + parseError.errorString().toStdString());
    }

    if (!doc.isObject()) {
        throw FileIOException("Invalid document format: expected root JSON object.");
    }

    QJsonObject rootObj = doc.object();
    QJsonArray patientArray = rootObj["patients"].toArray();

    db.clearAll();

    uint32_t maxPatId = 1000;
    uint32_t maxApptId = 2000;

    for (const auto& val : patientArray) {
        QJsonObject pObj = val.toObject();
        std::string id = pObj["id"].toString().toStdString();
        std::string name = pObj["name"].toString().toStdString();
        int age = pObj["age"].toInt();
        std::string contact = pObj["contact"].toString().toStdString();
        std::string type = pObj["type"].toString().toStdString();

        // Scan ID for counter sync
        if (id.length() > 1 && id[0] == 'P') {
            try {
                uint32_t num = std::stoul(id.substr(1));
                if (num > maxPatId) maxPatId = num;
            } catch (...) {}
        }

        std::shared_ptr<model::PatientRecord> patient;
        if (type == "Emergency") {
            int triage = pObj["triageLevel"].toInt(1);
            std::string incident = pObj["incidentDescription"].toString().toStdString();
            bool trauma = pObj["requiresTraumaUnit"].toBool(false);
            patient = std::make_shared<model::EmergencyPatient>(id, name, age, contact, triage, incident, trauma);
        } else if (type == "Follow-Up") {
            std::string docName = pObj["primaryDoctor"].toString().toStdString();
            std::string diag = pObj["previousDiagnosis"].toString().toStdString();
            int interval = pObj["followUpIntervalDays"].toInt(14);
            patient = std::make_shared<model::FollowUpPatient>(id, name, age, contact, docName, diag, interval);
        } else {
            double fee = pObj["clinicFee"].toDouble(50.0);
            std::string provider = pObj["insuranceProvider"].toString().toStdString();
            patient = std::make_shared<model::GeneralPatient>(id, name, age, contact, fee, provider);
        }

        // Appointments
        QJsonArray apptArray = pObj["appointments"].toArray();
        for (const auto& aVal : apptArray) {
            QJsonObject aObj = aVal.toObject();
            std::string apptId = aObj["appointmentId"].toString().toStdString();
            std::string dt = aObj["dateTime"].toString().toStdString();
            std::string dept = aObj["department"].toString().toStdString();
            std::string doctor = aObj["doctor"].toString().toStdString();
            std::string reason = aObj["reason"].toString().toStdString();
            std::string statusStr = aObj["status"].toString().toStdString();

            if (apptId.length() > 3 && apptId.rfind("APT", 0) == 0) {
                try {
                    uint32_t num = std::stoul(apptId.substr(3));
                    if (num > maxApptId) maxApptId = num;
                } catch (...) {}
            }

            model::Appointment appt(apptId, id, dt, dept, doctor, reason, model::Appointment::stringToStatus(statusStr));
            patient->addAppointment(appt);
        }

        db.addPatient(patient, /*recordUndo=*/false);
    }

    // Restore waiting queue
    QJsonArray queueArray = rootObj["waitingQueue"].toArray();
    for (const auto& qVal : queueArray) {
        std::string qId = qVal.toString().toStdString();
        if (db.patientExists(qId)) {
            db.enqueueWaitingPatient(qId);
        }
    }

    // Sync IDGenerator
    IDGenerator::instance().updatePatientCounterIfHigher(maxPatId);
    IDGenerator::instance().updateAppointmentCounterIfHigher(maxApptId);
}

void FileHandler::exportToCsv(const model::ClinicDatabase& db, const std::string& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) {
        throw FileIOException("Could not open CSV file for export: " + filePath);
    }

    out << "Patient ID,Name,Age,Contact,Patient Type,Completed Appts,Pending Appts\n";
    for (const auto& p : db.getPatients()) {
        if (!p) continue;
        out << "\"" << p->getId() << "\","
            << "\"" << p->getName() << "\","
            << p->getAge() << ","
            << "\"" << p->getContact() << "\","
            << "\"" << p->getPatientType() << "\","
            << p->getCompletedAppointmentCount() << ","
            << p->getPendingAppointmentCount() << "\n";
    }
}

void FileHandler::exportAppointmentsToCsv(const model::ClinicDatabase& db, const std::string& filePath) {
    std::ofstream out(filePath);
    if (!out.is_open()) {
        throw FileIOException("Could not open CSV file for appointments export: " + filePath);
    }

    out << "Appointment ID,Patient ID,Patient Name,Date Time,Department,Doctor,Reason,Status\n";
    for (const auto& p : db.getPatients()) {
        if (!p) continue;
        for (const auto& a : p->getAppointments()) {
            out << "\"" << a.getAppointmentId() << "\","
                << "\"" << p->getId() << "\","
                << "\"" << p->getName() << "\","
                << "\"" << a.getDateTime() << "\","
                << "\"" << a.getDepartment() << "\","
                << "\"" << a.getDoctor() << "\","
                << "\"" << a.getReason() << "\","
                << "\"" << a.getStatusString() << "\"\n";
        }
    }
}

} // namespace scapms::utils
