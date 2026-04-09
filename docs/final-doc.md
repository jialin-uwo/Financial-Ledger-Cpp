# Project Postmortem Report

## CS3307 Group Project – Final Documentation

**Group Members:** Jialin Li, Xinyan Cai, Junqi Zheng, Zhixian Wang, Peiyong Wang

---



## Introduction
This project is a terminal-based personal finance ledger system, designed and implemented by our CS3307 group. Our focus was on delivering a robust, user-friendly, and extensible tool for managing income and expenses, category data, budgets, CSV import, and financial analysis. The system features a modular architecture—Data Model, Ledger Controller, Data Access, Financial Analytics, and UI System—enabling clear responsibility, parallel development, and strong validation at every layer. This postmortem summarizes the project’s scope, highlights, technical achievements, and key lessons learned.

---


## Project Summary
The system enables users to manage and analyze personal financial records with features including record CRUD, category and budget management, CSV-based persistence and import, and rich analytics (summaries, trends, comparisons). Each module enforces strong validation and error handling, ensuring data integrity and a smooth user experience. The layered design separates business logic, persistence, computation, and presentation, making the system reliable, maintainable, and ready for future extension.

---


## Key Accomplishments & Solutions
- **Comprehensive Design and Robustness:** The system supports not only basic income and expense management, but also advanced features such as batch import, budget tracking, and detailed analytics. Each module enforces strict validation and error handling to ensure data security and user experience. As a result, the system passed all acceptance tests and handled real user data without issues.
- **UI Detail Optimization:** For example, when selecting an expense category, the UI automatically disables budget input, and for historical months, fields like "days left" and "daily available" are hidden, making the interface more intuitive. These optimizations reduced user confusion and improved overall satisfaction in user testing.
- **Modularity and Maintainability:** The layered architecture enables parallel development and easy maintenance. Analytics are centralized in FinancialAnalyzer, making extension and testing straightforward. This structure allowed team members to work efficiently and made future upgrades easier.
- **Problem-Solution Loop:** Issues discovered during development (such as record ID changes, file write safety, UI details, etc.) were thoroughly resolved through architectural and code improvements, making the system more robust and aligned with real-world needs. The final product demonstrated stable operation and met all project requirements.

These accomplishments reflect our strong focus on user needs, system robustness, and maintainability.


---


## Lessons Learned
- **Thorough planning and iterative refinement** are key to building robust, user-friendly products that meet real needs.
- **Details matter:** Small interface and implementation optimizations (such as validation, error handling, and UI clarity) have a big impact on user experience and system reliability.
- **Layered architecture and modularity** make parallel development, maintenance, and future upgrades much easier.
- **Clear error reporting and defensive programming** greatly improve system stability and user trust.
- **Technology choices should fit project scale,** but leave room for future upgrades (e.g., database, GUI).

---


## Conclusion
This project was a valuable software engineering experience, pushing us to think beyond implementation and focus on design, user experience, and maintainability. The final system is robust, user-friendly, and extensible, meeting all real-world requirements. In the future, replacing CSV with a database and evolving the CLI into a GUI would be natural next steps for scalability and usability—our architecture is ready for such upgrades.