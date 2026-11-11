# SCAPMS: Smart Clinic Appointment & Patient Management System

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17%2F20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![GUI Framework](https://img.shields.io/badge/GUI-Qt%206.12-brightgreen.svg)](https://www.qt.io/)
[![Build System](https://img.shields.io/badge/Build-CMake%20%7C%20Ninja-orange.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%28CachyOS%2FArch%29-purple.svg)](https://cachyos.org/)

A desktop clinical workstation application developed for the university health clinic to register and manage patients, schedule appointments, handle emergency priority triage and consultation queues, execute manual searching/sorting algorithms, undo administrative actions, and compile clinic analytical reports.

Designed and implemented in C++ using Qt 6 for **COS202 Programming II: Lab Assessment Project**.

---

## Key Architectural Highlights

- **Object-Oriented Design**:
  - `PatientRecord`: Abstract base class with pure virtual polymorphism (`getPatientType()`, `displayDetails()`, `clone()`).
  - `Appointable`: Interface class with pure virtual methods for schedulable patient types (`scheduleAppointment()`, `cancelAppointment()`).
  - `GeneralPatient`, `EmergencyPatient`, `FollowUpPatient`: Concrete polymorphic subclasses implementing specialized triage, urgency levels, and follow-up tracking.
  - `Appointment`: Composed appointment record tracking date, time, service category, attending physician, and status (`Pending`, `Completed`, `Cancelled`).
  - `ClinicDatabase`: In-memory container coordinator managing persistence and collection operations.

- **Data Structures**:
  - `std::vector<std::shared_ptr<PatientRecord>>`: Dynamic master registry for fast random access and sorting.
  - `std::queue<std::string>`: First-In-First-Out (FIFO) consultation waiting line.
  - `std::stack<std::shared_ptr<AdminAction>>`: Administrative undo history stack to reverse deletes, edits, or removals.
  - `std::array<std::shared_ptr<PatientRecord>, 10>`: Fixed-size LRU/recent patient record cache for quick lookup.

- **Manual Algorithms (Zero Standard-Library Algorithm Shortcuts)**:
  - **Searching**:
    - Linear Search: Sequential ID and contact lookup on unsorted datasets.
    - Binary Search: $O(\log n)$ lookup on sorted patient datasets.
    - Recursive Search: Divide-and-conquer recursive name matching.
  - **Sorting**:
    - Selection Sort: In-place manual exchange sort.
    - Quick Sort / Insertion Sort: Efficient manual multi-field sorting (Name, ID, Age, Date).

- **Modern Qt 6 Graphical User Interface**:
  - Multi-tabbed layout (`Patient Management`, `Appointments`, `Waiting Queue`, `Search & Sort`, `Reports & Analytics`, `Admin / Audit`).
  - Strict layout management (`QVBoxLayout`, `QHBoxLayout`, `QGridLayout`, `QFormLayout`).
  - Custom table styling with badges for patient categories (`Emergency` $\rightarrow$ High Priority red badge, `Follow-Up` $\rightarrow$ Scheduled blue badge, `General` $\rightarrow$ Normal green badge).
  - Responsive keyboard shortcuts (`Ctrl+S`, `Ctrl+F`, `Ctrl+Q`), tooltips, and timers for reminders and queue alerts.

- **Persistence & Exception Handling**:
  - Local JSON / Structured file persistence without external database engines.
  - Custom exceptions (`DuplicateIDException`, `InvalidAgeException`, `RecordNotFoundException`, `FileIOException`).

---

## Directory Structure

```text
SCAPMS/
├── CMakeLists.txt            # Root CMake build specification (Qt6::Widgets)
├── README.md                 # System documentation & quickstart
├── ARCHITECTURE.md           # Detailed class hierarchy & algorithm specifications
├── docs/                     # Assessment rubric and assignment PDFs
│   ├── COS202_Lab_Assessment_Question.pdf
│   └── SCAPMS_Implementation_Guide.pdf
├── model/                    # Data models, OOP hierarchy & database container
│   ├── Appointable.h
│   ├── Appointment.h / .cpp
│   ├── PatientRecord.h / .cpp
│   ├── GeneralPatient.h / .cpp
│   ├── EmergencyPatient.h / .cpp
│   ├── FollowUpPatient.h / .cpp
│   └── ClinicDatabase.h / .cpp
├── controller/               # Business controllers & manual algorithms
│   ├── ClinicManager.h / .cpp
│   ├── AppointmentController.h / .cpp
│   └── SearchEngine.h / .cpp
├── gui/                      # Qt GUI views and components
│   ├── MainWindow.h / .cpp
│   ├── PatientPanel.h / .cpp
│   ├── AppointmentPanel.h / .cpp
│   ├── QueuePanel.h / .cpp
│   ├── SearchPanel.h / .cpp
│   └── ReportsPanel.h / .cpp
├── utils/                    # Utility generators, serializers & custom exceptions
│   ├── Exceptions.h
│   ├── IDGenerator.h / .cpp
│   └── FileHandler.h / .cpp
├── data/                     # Sample datasets and persistence store
│   └── sample_patients.json
└── main.cpp                  # Application entry point
```

---

## Building and Running

### Prerequisites (CachyOS / Arch Linux)
```bash
sudo pacman -S base-devel cmake ninja qt6-base qt6-tools qt6-svg
```

### Build with CMake & Ninja
```bash
# Configure the build directory
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build the executable
cmake --build build

# Run SCAPMS
./build/SCAPMS
```

---

## License & Academic Integrity
Prepared for COS202 Programming II Midsemester Assessment. All core algorithms are manually implemented according to assessment rules.
