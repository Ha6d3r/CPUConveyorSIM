#include "menu.h"

Menu::Menu() {
    c = Conveyor();
}

void Menu::show() {

    using std::this_thread::sleep_for;
    using std::chrono::milliseconds;

    bool run = true;
    int  pos = 0;

    unsigned int seed;
    unsigned int count;

    double type_1_weight;
    double type_3_weight;
    double reg_weight;

    int memory_speed;
    int type_1_speed;
    int type_2_speed;

    while (run) {
        std::cout << save_pos;

        for (int i = 0; i < names_count; ++i) {
            if (i == pos)
                std::cout << "\033[30m\033[47m ";
            else
                std::cout << "\033[37m\033[40m ";

            std::cout << names[i] << " \033[0m   ";
        }


        switch (getch()) {
            case 77: {
                if (pos!=names_count-1) pos++;
                break;
            }
            case 75: {
                if (pos!=0) pos--;
                break;
            }
            case 72: {
                break;
            }
            case 80: {

                std::cout << "\033[2J\033[H";

                switch (pos) {
                    case 0: {

                        std::cout << "\033[30m\033[47m SEED  \033[0m                -> ";
                        std::cin >> seed;

                        std::cout << "\033[30m\033[47m COUNT \033[0m                -> ";
                        std::cin >> count;

                        std::cout << "\033[30m\033[47m P1    \033[0m (0.9,0.7,0.5)  -> ";
                        std::cin >> type_1_weight;

                        std::cout << "\033[30m\033[47m P3    \033[0m                -> ";
                        std::cin >> type_3_weight;

                        std::cout << '\n' << "P1: 0 <= " << type_1_weight << "\n" << "P2: " << type_1_weight << " < " << 1 - type_3_weight << "\n" << "P3: " << 1 - type_3_weight << " <= 1\n\n";

                        std::cout << "\033[30m\033[47m REG   \033[0m (0.85,0.7,0.5) -> ";
                        std::cin >> reg_weight;

                        std::cout << "\033[30m\033[47m MEM   \033[0m (2,5,10)       -> ";
                        std::cin >> memory_speed;

                        std::cout << "\033[30m\033[47m M1    \033[0m (1,2,3)        -> ";
                        std::cin >> type_1_speed;

                        std::cout << "\033[30m\033[47m M2    \033[0m (4,8,16)       -> ";
                        std::cin >> type_2_speed;

                        c.generate_operations(seed,count,type_1_weight,type_3_weight,reg_weight,memory_speed,type_1_speed,type_2_speed);

                        while(getch() != 72);
                        break;
                    }

                    case 1: {

                        c.show_ops_list();

                        while(getch() != 72);
                        break;
                    }

                    case 2: {

                        std::cout << "\033[30m\033[47m SPEED \033[0m -> ";
                        std::cin >> seed;

                        while(c.cycle()) {
                            std::cout << "\033[H";
                            c.show_stack();
                            sleep_for(milliseconds(seed));
                        }

                        while(getch() != 72);
                        break;
                    }

                    case 3: {
                        run = false;
                        break;
                    }
                }

                std::cout << "\033[H\033[2J";

                break;
            }
        }

        std::cout << load_pos;
    }
}
