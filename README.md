# CS 3307 Group 27 Ledger System

A terminal-based personal finance ledger system supporting income/expense management, data persistence, batch import, and statistical analysis. Designed to help users efficiently track and analyze their daily finances.

## 🚀 Quick Start

### Environment

- Recommended platform: Western University CS Virtual Machine (GAUL)
- Supported build environment: Linux with `g++` 9+ and full C++17 support
- Optional tool (for diagrams only): Graphviz (`dot`)

### Build And Run

1. Enter the project directory:
	```bash
	cd group27
	```
2. Compile:
	```bash
	g++ -std=c++17 *.cpp -o ledger
	```
3. Run:
	```bash
	./ledger
	```

---

## 🌟 Features & Highlights

- **Comprehensive Record Management:** Add, query, update, and delete any income or expense record with automatic unique ID assignment.
- **Customizable Categories:** Manage custom categories, set budgets, and warning thresholds with intelligent auto-completion.
- **Data Persistence:** Reliable CSV-based storage for continuous usage.
- **Batch Import:** High-performance import from CSV with automatic merging and validation logic.
- **Statistical Analysis:** Real-time calculation of totals, balances, and category-based budget health.
- **Robustness:** Strict input validation and clear exception handling across all modules.

---

## 🛠 Engineering Upgrades & Refactoring

As a core contributor, I have spearheaded a major architectural refactoring of the system since **2026-03-29** to transition it into a robust, industry-standard application.

**Key Contributions:**
- **Transactional Integrity:** Implemented **rollback mechanisms** and **atomic write strategies** (temp file + atomic rename) to ensure zero data corruption during crashes.
- **UX Intelligence:** Developed **context-aware UI logic**, such as dynamically hiding irrelevant metrics for historical data and implementing conditional input constraints for different category types.
- **Standardized Communication:** Unified backend response structures with professional error propagation chains and status codes.
- **Code Standards:** Conducted a system-wide cleanup of `using namespace std;` and improved class extensibility through explicit namespace management.

👉 [**Detailed Refactoring Documentation & Changelog**](./docs/Refactoring%20&%20System%20Upgrade%20Contributions.md)

---

## 📁 Project Structure

- `main.cpp` — Program entry and UI initialization
- `LedgerController.cpp/hpp` — Core business logic controller
- `DataAccess.cpp/h` — Data persistence and CSV I/O
- `FinancialAnalyzer.cpp/h` — Statistical analysis and summary engine
- `Record.cpp/h` — Financial record data structure and validation
- `Category.cpp/h` — Category management and financial logic
- `MenuSystem.cpp/hpp` — Terminal interaction and menu logic
- `BudgetStatus.h` / `BudgetHealth.h` — Data structures for budget analysis
- `Result.h` — Unified operation result and status code definitions
- `docs/` — Comprehensive project documentation and refactoring logs

---

## 👥 Contributors

All members of Group 27.
