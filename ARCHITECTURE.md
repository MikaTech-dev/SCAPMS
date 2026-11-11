# SCAPMS Architecture & Design Specification

## 1. Object-Oriented Class Hierarchy

```text
               +----------------------+
               |     Appointable      | (Interface)
               +----------------------+
               | +scheduleAppt()      |
               | +cancelAppt()        |
               +----------+-----------+
                          ^
                          | implements
+-------------------------+----+
|        PatientRecord         | (Abstract Base Class)
+------------------------------+
| # id: string                 |
| # name: string               |
| # age: int                   |
| # contact: string            |
| # appointments: vector<Appt> |
+------------------------------+
| +getPatientType()*: string   | (Pure Virtual)
| +displayDetails()*: string   | (Pure Virtual)
| +clone()*: unique_ptr        | (Virtual Prototype)
+--------------+---------------+
               |
      +--------+--------+
      |                 |
+-----+------+   +------+-----+   +-----------------+
| General    |   | Emergency  |   | FollowUp        |
| Patient    |   | Patient    |   | Patient         |
+------------+   +------------+   +-----------------+
| -clinicFee |   | -severity  |   | -prevDoctor     |
|            |   | -triageCode|   | -followUpDueDate|
+------------+   +------------+   +-----------------+
```

### 1.1 Model Breakdown
- **`Appointable` (`namespace scapms::model`)**: Pure abstract interface declaring appointment lifecycle contracts.
- **`PatientRecord` (`namespace scapms::model`)**: Common state encapsulation (`id`, `name`, `age`, `contact`, `appointments`). Pure virtual polymorphic interface guarantees type-safety and dynamic dispatch.
- **`GeneralPatient`**: Regular consultation patient with consultation fee status and insurance ID.
- **`EmergencyPatient`**: Immediate medical attention patient with triage severity score (`Level 1` to `Level 5`) and trauma unit flag.
- **`FollowUpPatient`**: Post-treatment patient with primary physician attribution, previous diagnosis, and review interval.
- **`Appointment`**: Value object encapsulating `appointmentId`, `patientId`, `dateTime`, `department`, `doctor`, `reason`, and `status` (`Pending`, `Completed`, `Cancelled`).
- **`ClinicDatabase`**: Central aggregate containing the `std::vector` of patient records, `std::queue` of consultation attendees, `std::stack` of undo snapshots, and the 10-slot fixed-size cache.

---

## 2. Core Data Structures & Administrative Operations

| Data Structure | C++ Type | Operational Context |
| :--- | :--- | :--- |
| **Master Registry** | `std::vector<std::shared_ptr<PatientRecord>>` | In-memory dynamic patient table supporting random access for sorting and search indexing. |
| **Consultation Queue** | `std::queue<std::string>` (Patient IDs) | First-Come-First-Served (FIFO) consultation waiting line. |
| **Undo Stack** | `std::stack<std::unique_ptr<ICommand>>` | Command design pattern / state snapshots enabling immediate rollback of deletes, edits, or removals. |
| **Recent Cache** | `std::array<std::shared_ptr<PatientRecord>, 10>` | Fixed-size array caching the 10 most recently accessed or modified records. |

---

## 3. Manual Algorithm Specifications

### 3.1 Searching Algorithms
1. **Linear Search (`SearchEngine::linearSearchById`)**:
   - Time Complexity: $O(n)$
   - Usage: Unsorted datasets, dynamic lookup by exact ID or partial contact phone.
2. **Binary Search (`SearchEngine::binarySearch`)**:
   - Time Complexity: $O(\log n)$
   - Usage: Executed when data is sorted by the queried key (e.g., Patient ID or Name).
3. **Recursive Search (`SearchEngine::recursiveSearchByName`)**:
   - Time Complexity: $O(n)$ with $O(n)$ recursion stack
   - Usage: Demonstrates recursive divide-and-conquer search comparing string prefixes over a range $[low, high]$.

### 3.2 Sorting Algorithms
1. **Selection Sort (`SearchEngine::selectionSort`)**:
   - Time Complexity: $O(n^2)$ comparisons, $O(n)$ swaps.
   - Mechanism: Repeatedly finds the minimum element from the unsorted subarray and places it at the beginning.
2. **Quick Sort (`SearchEngine::quickSort`)**:
   - Time Complexity: $O(n \log n)$ average.
   - Mechanism: Partition-exchange sort using pivot selection, sorting by Patient Name, ID, Age, or Appointment Date.
3. **Insertion Sort (`SearchEngine::insertionSort`)**:
   - Time Complexity: $O(n^2)$ worst/average, $O(n)$ best for near-sorted data.
   - Mechanism: Incrementally builds the sorted array one item at a time.

---

## 4. Administrative Undo Mechanism
Using the Command / Snapshot pattern:
```text
[User Action: Delete Patient P1002]
         │
         ▼
[Push UndoAction{ Type::Delete, PatientSnapshot, PrevAppointments } onto std::stack]
         │
         ▼
[Admin clicks "Undo"] ──> [Pop from stack] ──> [Restore record into std::vector & UI]
```

---

## 5. Qt 6 GUI Architecture

```text
┌─────────────────────────────────────────────────────────────┐
│ MainWindow (Menu Bar: File [New, Save, Load, Exit], Help)   │
├─────────────────────────────────────────────────────────────┤
│ QTabWidget                                                  │
│ ┌──────────────┬──────────────┬─────────────┬─────────────┐ │
│ │ Patients     │ Appointments │ Queue       │ Search/Sort │ │
│ ├──────────────┴──────────────┴─────────────┴─────────────┤ │
│ │ • QFormLayout for Patient Details                       │ │
│ │ • Custom QTableWidget with Priority Color Badges        │ │
│ │ • Dynamic form fields based on Patient Type             │ │
│ │ • Action Buttons with Tooltips (Add, Edit, Delete)      │ │
│ └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│ Status Bar (Real-time Clock, Queue Count, Last Action Msg)  │
└─────────────────────────────────────────────────────────────┘
```

- **Layout Techniques**: Nested `QVBoxLayout` and `QHBoxLayout` for toolbars and table panes; `QFormLayout` for clean patient input forms; `QGridLayout` for statistical metric summary cards.
- **Custom Presentation**: Color highlights in tables for Emergency (Crimson badge), General (Mint green badge), and Follow-Up (Royal blue badge).
- **Asynchronous Timers**: `QTimer` triggers for consultation waiting alerts and periodic dashboard refreshes.
