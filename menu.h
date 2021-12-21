#ifndef MENU_H
#define MENU_H

#include <conveyor.h>

#include <conio.h>
#include <iostream>
#include <thread>
#include <chrono>

class Menu {

    public:
        Menu();

        void show();

    private:

        Conveyor c;

        int names_count = 4;
        std::string names[4] = {
            "Generate commands",
            "Show commands",
            "Launch conveyor",
            "Exit"
        };
};

#endif // MENU_H
