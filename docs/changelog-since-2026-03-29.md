# Changelog Since 2026-03-29

1. UI and Interaction Improvements:
   - More detailed operation flows, prompts, and error feedback. All user input and batch imports now provide clear error messages.
   - Enhanced user prompts and feedback for search, filtering, and reporting features.
   - Improved batch import and category management experience, with stronger input validation and rollback mechanisms.

2. Business Logic and Fault Tolerance:
   - All multi-data synchronization operations (such as batch import, category auto-completion, record modification) now support rollback on failure to ensure data consistency.
   - Clear error propagation chain for easier troubleshooting.
   - All operation results are returned in a unified structured format, including status code, message, and detailed error info for consistent handling and display.

3. Test Data and Data Standardization:
   - Data file formats and contents are more standardized.
   - Data content has been reorganized and updated to better support functional testing and status validation.
   - Added dirty data samples to verify import validation and error reporting capabilities.
   - Adjusted test data to cover different budget status scenarios for status display validation.

4. Budget and Category Logic Optimization:
   - Budget prompts are no longer shown for income categories; budget logic is now smarter.
   - Category and budget logic is more reasonable and extensible.

5. Validation Mechanism Optimization:
   - All record and category additions, imports, and modifications now have strict format, type, and validity checks, with detailed error feedback.

6. Data Persistence Safety:
   - All data saves use a temp file + atomic rename strategy to prevent data corruption from interrupted writes.
   - Combined with in-memory rollback, batch/multi-file operations now have transactional and consistency guarantees.

7. Documentation and Maintainability:
   - Project documentation rewritten to highlight features and facilitate collaboration and onboarding.
   - Removed global 'using namespace std'; explicit namespaces are now used throughout to reduce naming conflicts and improve maintainability.
