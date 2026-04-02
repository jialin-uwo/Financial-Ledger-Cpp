/**
 * @file main.cpp
 * @brief Entry point of the C-The-Cash ledger system.
 *
 * This file contains the program entry point for the C-The-Cash
 * application. It is responsible for constructing the core controller
 * and menu system objects, initializing the system state from persistent
 * storage, reporting initialization results to the user, and starting
 * the main command-line user interface loop.
 *
 * The startup sequence performed in this file includes:
 * - creating the central LedgerController instance,
 * - binding the controller to the MenuSystem,
 * - initializing system data such as CSV-backed records and categories,
 * - handling initialization failure, and
 * - launching the interactive terminal interface.
 *
 * @author Junqi Zheng
 */

#include <iostream>
#include "LedgerController.hpp"
#include "MenuSystem.hpp"

/**
 * @brief Starts the C-The-Cash application.
 *
 * This function serves as the main entry point of the program. It performs
 * the high-level startup sequence of the ledger system by:
 * - instantiating the core @c LedgerController,
 * - passing the controller into the @c MenuSystem,
 * - initializing persistent application data,
 * - printing initialization feedback to the terminal, and
 * - entering the main menu loop if initialization succeeds.
 *
 * If initialization fails, an error message is written to the standard
 * error stream and the program exits with a non-zero status code.
 *
 * @return Returns @c 0 if the application starts and exits normally;
 * returns @c 1 if initialization fails.
 *
 * @author Junqi Zheng
 */
int main()
{
    /// Instantiate the core controller that manages ledger operations.
    LedgerController controller;

    /// Pass the controller instance to the menu system.
    MenuSystem menu(controller);

    /// Start system initialization, including loading persisted CSV data.
    std::cout << "Initializing system..." << std::endl;
    Result initResult = controller.init();

    /// Display the initialization result message to the user.
    std::cout << initResult.message << std::endl;

    /// Abort startup if initialization was unsuccessful.
    if (!initResult.ok())
    {
        std::cerr << "Initialization failed: " << initResult.message << std::endl;
        return 1;
    }

    /// Enter the main command-line user interface loop.
    menu.run();

    /// Return success after normal program termination.
    return 0;
}