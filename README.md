# CS 3307 Group27 Ledger System

## Project Overview
This project is a C++-based personal finance ledger system. It supports adding, deleting, searching, and updating financial records, batch CSV import, category and period-based statistics, and summary features. Data is persisted in CSV files, making it suitable for command-line bookkeeping.

## Main Features
- Add/Delete/Update income and expense records
- Query records by date range, category, amount, etc.
- Batch import from CSV files
- Financial summary (total income, total expense, net balance)
- Simple period total statistics
- Error messages and input validation

## File Structure
- `main.cpp`              Program entry, UI initialization
- `MenuSystem.cpp/hpp`    Menu and user interaction logic
- `LedgerController.cpp/hpp`  Business logic controller
- `DataAccess.cpp/h`      Data persistence and CSV I/O
- `Record.cpp/h`          Record data structure and validation
- `FinancialAnalyzer.cpp/h`  Financial statistics and analysis
- `records.csv`           Ledger data file
- `runtime.csv`           Runtime data (if any)

## Build & Run
1. Enter the project directory:
   ```sh
   cd group27
   ```
2. Compile:
   ```sh
   g++ -std=c++17 *.cpp -o ledger
   ```
3. Run:
   ```sh
   ./ledger
   ```

## Usage
- Follow the menu prompts to select features by number.
- When adding/updating records, enter date, amount, type (income/expense), and category (leave blank for auto-assignment).
- For queries/summaries, you can filter by date, category, etc. Leave blank for all.
- Batch import supports standard CSV format.

## Notes
- Date format: `YYYY-MM-DD`.
- If category is left blank, expenses default to "Other Expense", income to "Other Income".
- All inputs are validated; errors will be shown with messages.

## Contributors
- All members of Group27

## Contact
For questions, contact the course staff or project members.