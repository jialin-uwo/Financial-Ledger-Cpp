


# CS 3307 Group 27 Ledger System

A terminal-based personal finance ledger system supporting income/expense management, data persistence, batch import, and basic statistical analysis—helping users efficiently track and analyze their daily finances.

## 🚀 Quick Start


**Recommended Environment:**
- Western University CS Virtual Machine (GAUL) (strongly recommended for consistent running and testing)
- Or any Linux environment with g++ 9+ and full C++17 support (required)
	- C++17 support is mandatory for successful compilation and execution.
	- This ensures the program can run and be tested without platform-specific issues.

1. **Enter the project directory**
	```bash
	cd group27
	```
2. **Compile the project**
	```bash
	g++ -std=c++17 *.cpp -o ledger
	```
3. **Run the application**
	```bash
	./ledger
	```

---

## 📁 Project Structure

- `main.cpp` — Program entry point and UI initialization
- `LedgerController.cpp/hpp` — Core business logic controller and coordinator
- `DataAccess.cpp/h` — Data persistence and CSV I/O operations
- `FinancialAnalyzer.cpp/h` — Statistical analysis and total calculation engine
- `Record.cpp/h` — Data structure for financial records with built-in validation
- `MenuSystem.cpp/hpp` — Terminal-based user interaction logic
- `records.csv` — Main ledger data file (persistent financial records)
- `runtime.csv` — Temporary or batch import file (for session or analytics)

---

## 📦 Batch Import & Usage Notes

- **Batch Import (Add by File):** Use the "Add by File" feature to batch import records from a CSV file. Place `runtime.csv` in the project root. The format must match `records.csv` (columns: id, date, amount, isExpense, category). Imported data merges into `records.csv`.
- **Date Format:** Inputs must follow the `YYYY-MM-DD` format.
- **Auto-Assignment:** If a category is blank, the system defaults to `Other Expense` or `Other Income` for aggregation.
- **Documentation:** All source code is Doxygen-commented for future technical documentation.

---

## 🏗 Implementation Strategy (Stage 3)

This stage marks the transition from design to functional development, focusing on a robust Data Flow and Persistence Layer (**~30% of total scope**).

### Prototype Completion (30%)
- **Record CRUD:** Full Create, Read, Update, Delete for transactions
- **Data Persistence:** Load/Save via CSV for session continuity
- **Initialization & Safety:** `init()`/`shutDown()` for re-indexing and safe file streaming
- **Core Analytics:** Calculation engines for `getTotal` and `getPeriodSummary`

### Design Decisions & Trade-offs
- **Category Logic Simplification:** Advanced Category CRUD deferred to Stage 4; current structure supports category attributes

#### Technical Reasoning
- **Logical Dependencies:** Full category system requires complex validation and cascading updates
- **Data Integrity:** Prioritized persistence and record management before high-level business rules
- **Visualization Roadmap:** Structured summaries now; graphical analysis (charts, diagrams) planned for Stage 4

---

## 👥 Contributors

All members of Group 27.