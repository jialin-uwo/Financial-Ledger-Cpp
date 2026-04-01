# CS 3307 Group 27 Ledger System

A terminal-based personal finance ledger system supporting income/expense management, data persistence, batch import, and statistical analysis. Designed to help users efficiently track and analyze their daily finances.

## 🚀 Quick Start

**Environment Requirements:**
- Recommended: Western University CS Virtual Machine (GAUL)
- Or any Linux environment with g++ 9+ and full C++17 support

1. Enter the project directory
	```bash
	cd group27
	```
2. Compile the project
	```bash
	g++ -std=c++17 *.cpp -o ledger
	```
3. Run the application
	```bash
	./ledger
	```

---

## 🌟 Features & Highlights

- **Comprehensive Record Management:** Add, query, update, and delete any income or expense record with automatic unique ID assignment.
- **Customizable Categories:** Create and manage custom categories for both income and expenses, set budgets and warning thresholds, and auto-complete missing categories.
- **Data Persistence:** All data is saved in CSV files for reliable, continuous usage.
- **Batch Import:** Import multiple records at once from a CSV file, with automatic merging and validation.
- **Statistical Analysis:**
  - Query total income, total expense, and balance for any time period
  - Category-based statistics and budget status analysis
  - Monthly trend and income/expense distribution analysis
- **Error & Exception Handling:** All user input is validated, with clear error messages for invalid formats or operations.
- **User-Friendly Terminal Interface:** Menu-driven CLI, intuitive for all users.
- **Well-Documented Code:** All source code is Doxygen-commented for easy maintenance and documentation generation.

---

## 📁 Project Structure

- `main.cpp` — Program entry and UI initialization
- `LedgerController.cpp/hpp` — Core business logic controller
- `DataAccess.cpp/h` — Data persistence and CSV I/O
- `FinancialAnalyzer.cpp/h` — Statistical analysis and summary engine
- `Record.cpp/h` — Financial record data structure and validation
- `MenuSystem.cpp/hpp` — Terminal interaction and menu logic
- `records.csv` — Main ledger data file
- `categories.csv` — Category information file
- `runtime.csv` — Temporary file for batch import

---

## 👥 Contributors

All members of Group 27.