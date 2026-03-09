#include <iostream>
#include "LedgerController.hpp"
#include "MenuSystem.hpp"

int main() {
    // 1. Instantiate the core controller
    LedgerController controller;

    // 2. Pass the controller instance to the menu system
    MenuSystem menu(controller);

    // 3. Start system initialization (loading CSV files, etc.)
    std::cout << "Initializing system..." << std::endl;
    std::cout << controller.init() << std::endl;

    // 4. Enter the main UI loop
    menu.run();

    return 0;
}
