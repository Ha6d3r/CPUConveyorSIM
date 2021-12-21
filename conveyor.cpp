#include "conveyor.h"

Conveyor::Conveyor() {
}

Conveyor::~Conveyor() {
    if (ops_list) free(ops_list);
    if (op_stack) free(op_stack);
}

inline int get_bit(unsigned int op, int pos) {
    return (op & ( 1 << pos )) >> pos;
}


// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                                          =========
inline int get_type(unsigned int op) {
    return (op & 0x1FFFFF) >> 19;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                                                  ======
inline int get_op1r(unsigned int op) {
    return (op & 0x200000) >> 21;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                                                       ======
inline int get_op2r(unsigned int op) {
    return (op & 0x400000) >> 22;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//       ===========
inline int get_op1(unsigned int op) {
    return (op >> 2) & 0xF;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                 ===========
inline int get_op2(unsigned int op) {
    return (op >> 6) & 0xF;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//       ===========
inline void set_op1(unsigned int * op, int oper) {
    *op &= 0xFFFFFFC3;
    *op |= (oper << 2);
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                 ===========
inline void set_op2(unsigned int * op, int oper) {
    *op &= 0xFFFFFC3F;
    *op |= (oper << 6);
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                           ==================
inline int get_ex(unsigned int op) {
    return (op >> 10) & 0x1F;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                           ==================
inline void set_ex(unsigned int * op, int ex) {
    *op &= 0xFFFF83FF;
    *op |= (ex << 10);
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                            ===============
inline int get_w(unsigned int op) {
    return (op >> 15) & 0xF;
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                            ===============
inline void set_w(unsigned int * op, int w) {
    *op &= 0xFFF87FFF;
    *op |= (w << 15);
}

// Used to check if operation is complete - this part will be 0
// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
// ==========================================================
inline int get_cycles(unsigned int op) {
    return (op & 0x7FFFF);
}

//                                                                             Left over bits used as wait
// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                                                            ============================
inline int get_wait(unsigned int op) {
    return (op >> 23);
}

// 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
// F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................
//                                                                            ============================
inline void set_wait(unsigned int * op, int wait) {
    //*op &= ~0xFF800000;
    *op &= 0x7FFFFF;
    *op |= (wait << 23);
}

void Conveyor::clear() {
    if (ops_list) free(ops_list);

    ops_count       = 0;

    ops_pos         = 0;
    stack_pos       = 0;

    cur_reg_users   = 0;
    cur_mem_users   = 0;

    cycles_count    = 0;

    max_wait_time   = 0;

    type_1_count    = 0;
    type_2_count    = 0;
    type_3_count    = 0;

    op1_reg         = 0;
    op2_reg         = 0;

    max_stack_size  = 0;

    reg_user_pos    = -1;
    mem_user_1_pos  = -1;
    mem_user_2_pos  = -1;
}

unsigned int Conveyor::gen_op(int type, bool op1_register, bool op2_register, int command_speed, int memory_speed) {
    // 0 | 1 | 2 3 4 5 | 6 7 8 9 | 10 11 12 13 14 | 15 16 17 18 | 19 20 | 21 | 22 | 23 24 25 26 27 28 29 30 31
    // F | D | OP1.... | OP2.... | EX............ | W.......... | T.... | R1 | R2 | ..........................

    unsigned int op = 3;

    op |= (op1_register) ? 4      : (memory_speed << 2 );
    op |= (op2_register) ? 64     : (memory_speed << 6 );
    op |= (command_speed << 10);
    op |= (op2_register) ? 32768  : (memory_speed << 15);

    op |= (type << 19);
    op |= (op1_register << 21);
    op |= (op2_register << 22);

    return op;
}

void Conveyor::generate_operations(unsigned int seed, int count, double type_1_weight, double type_3_weight, double register_weight, int memory_speed, int type_1_speed, int type_2_speed) {

    clear();

    ops_list = static_cast<unsigned int*>(malloc(count * sizeof(unsigned int)));

    int type;

    double number;
    bool op1_register;
    bool op2_register;

    type_3_weight = 1 - type_3_weight;

    srand(seed);

    for (int i = 0; i < count; ++i) {
        number = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

        if (number <= type_1_weight) type = 0;
        if (number > type_1_weight && number < type_3_weight) type = 1;
        if (number >= type_3_weight) type = 2;


        op1_register = false;
        op2_register = false;

        number = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        if (number <= register_weight) { op1_register = true; op1_reg++; }

        number = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        if (number <= register_weight) { op2_register = true; op2_reg++; }

        ops_count++;

        if (type == 0) {
            ops_list[i] = gen_op(type,op1_register,op2_register,type_1_speed,memory_speed);
            type_1_count++;
        }
        if (type == 1) {
            ops_list[i] = gen_op(type,op1_register,op2_register,type_2_speed,memory_speed);
            type_2_count++;
        }
        if (type == 2) {
            ops_list[i] = gen_op(type,op1_register,op2_register,1,memory_speed);
            type_3_count++;
            break;
        }

    }

    if (op_stack) free(op_stack);

    op_stack = static_cast<unsigned int*>(malloc((ops_count) * sizeof(unsigned int)));

    op_stack[0] = ops_list[0];
}

void Conveyor::mov_stack_left(int pos) {
    for (int i = pos; i < stack_pos-1; ++i) {
        op_stack[i] = op_stack[i+1];
    }
    stack_pos--;
}

bool Conveyor::cycle() {
    if (get_cycles(op_stack[0]) == 0 && stack_pos == 0 && ops_pos == ops_count) return false;

    cycles_count++;

    flag_move = false;

    if (get_cycles(op_stack[0]) == 0) { flag_move = true; commands_done++; sum += get_wait(op_stack[0]); mov_stack_left(0); }

    cur_reg_users = 0;
    cur_mem_users = 0;

    reg_user_pos    = -1;
    mem_user_1_pos  = -1;
    mem_user_2_pos  = -1;

    unsigned int op;
    int cycles;
    for (int i = 0; i < stack_pos; ++i) {
        op = op_stack[i];
        // лучше сделать через __builtin_clz()

        if (op&1) { op_stack[i] &= ~(1UL); continue; }
        if (op&2) { op_stack[i] &= ~(2UL); continue; }


        // тут можно было сделать код меньше, так как OP1 OP2 и W операции
        // похожи, но так нагляднее

        cycles = get_op1(op);
        if (cycles) {
            if (get_op1r(op)) {
                if (cur_reg_users == 0) {
                    set_op1(&op_stack[i],--cycles);
                    cur_reg_users = 1;
                    reg_user_pos  = i;
                    continue;
                }
                cycles = get_wait(op);
                set_wait(&op_stack[i],++cycles);
                if (cycles > max_wait_time) max_wait_time = cycles;
                if (cycles < min_wait_time) min_wait_time = cycles;
                continue;
            }
            if (cur_mem_users < 2 ) {
                set_op1(&op_stack[i],--cycles);
                cur_mem_users++;
                if (mem_user_1_pos < 0) mem_user_1_pos = i; else mem_user_2_pos = i;
                continue;
            }
            cycles = get_wait(op);
            set_wait(&op_stack[i],++cycles);
            if (cycles > max_wait_time) max_wait_time = cycles;
            if (cycles < min_wait_time) min_wait_time = cycles;
            continue;
        }

        cycles = get_op2(op);
        if (cycles) {
            if (get_op2r(op)) {
                if (cur_reg_users == 0) {
                    set_op2(&op_stack[i],--cycles);
                    cur_reg_users = 1;
                    reg_user_pos  = i;
                    continue;
                }
                cycles = get_wait(op);
                set_wait(&op_stack[i],++cycles);
                if (cycles > max_wait_time) max_wait_time = cycles;
                if (cycles < min_wait_time) min_wait_time = cycles;
                continue;
            }
            if (cur_mem_users < 2 ) {
                set_op2(&op_stack[i],--cycles);
                cur_mem_users++;
                if (mem_user_1_pos < 0) mem_user_1_pos = i; else mem_user_2_pos = i;
                continue;
            }
            cycles = get_wait(op);
            set_wait(&op_stack[i],++cycles);
            if (cycles > max_wait_time) max_wait_time = cycles;
            if (cycles < min_wait_time) min_wait_time = cycles;
            continue;
        }

        cycles = get_ex(op);
        if (cycles) {
            set_ex(&op_stack[i],--cycles);
            continue;
        }

        cycles = get_w(op);
        if (cycles) {
            if (get_op2r(op)) {
                if (cur_reg_users == 0) {
                    set_w(&op_stack[i],--cycles);
                    cur_reg_users = 1;
                    reg_user_pos  = i;
                    continue;
                }
                cycles = get_wait(op);
                set_wait(&op_stack[i],++cycles);
                if (cycles > max_wait_time) max_wait_time = cycles;
                if (cycles < min_wait_time) min_wait_time = cycles;
                continue;
            }
            if (cur_mem_users < 2 ) {
                set_w(&op_stack[i],--cycles);
                cur_mem_users++;
                if (mem_user_1_pos < 0) mem_user_1_pos = i; else mem_user_2_pos = i;
                continue;
            }
            cycles = get_wait(op);
            set_wait(&op_stack[i],++cycles);
            if (cycles > max_wait_time) max_wait_time = cycles;
            if (cycles < min_wait_time) min_wait_time = cycles;
            continue;
        }

        if (i > 1) {
            cycles = get_wait(op);
            set_wait(&op_stack[i],++cycles);

            if (cycles > max_wait_time) max_wait_time = cycles;
            if (cycles < min_wait_time) min_wait_time = cycles;
        }
    }

    if (ops_pos < ops_count) {
        op_stack[stack_pos++] = ops_list[ops_pos++];
        if (stack_pos > max_stack_size) max_stack_size = stack_pos;
    }


    return true;
}

void Conveyor::show_stack() {
    std::cout << "   POS  | Fetch  | Decode |   OP1  |   OP2  |  Exec  | Write  |  Wait  |\n";
    std::cout << "-----------------------------------------------------------------------\n";

    unsigned int op;
    bool state_shown;
    char flag;

    for (int i = 0; i < stack_pos; ++i) {
        std::cout << " " <<std::setw(6) << i;
        std::cout << " | ";
        op = op_stack[i];

        state_shown = true;

        if (op&1) {
            if (state_shown) {
                std::cout << ansii_b_green << ansii_f_white;
                state_shown = false;
            }
            std::cout << std::setw(6) << '1' << ansii_reset;
        } else {
            std::cout << "      ";
        }
        std::cout << " | ";

        if (op&2) {
            if (state_shown) {
                std::cout << ansii_b_magenta << ansii_f_white;
                state_shown = false;
            }
            std::cout << std::setw(6) << '1' << ansii_reset;
        } else {
            std::cout << "      ";
        }
        std::cout << " |";

        if (get_op1(op)) {

            if (get_op1r(op)) {
                std::cout << "R";
            } else {
                std::cout << "M";
            }

            if (state_shown) {
                std::cout << ansii_b_cyan << ansii_f_white;
            }

            std::cout << std::setw(6) << get_op1(op) << ansii_reset;

            flag = ' ';

            if (state_shown) {
                if (get_op1r(op)) {
                    if (i == reg_user_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = 'R'; }
                } else {
                    if (i == mem_user_1_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '1'; }
                    if (i == mem_user_2_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '2'; }
                }
                state_shown = false;
            }
            std::cout << flag << ansii_reset;

        } else {
            std::cout << "        ";
        }
        std::cout << "|";

        if (get_op2(op)) {

            if (get_op2r(op)) {
                std::cout << "R";
            } else {
                std::cout << "M";
            }

            if (state_shown) {
                std::cout << ansii_b_yellow << ansii_f_white;
            }
            std::cout << std::setw(6) << get_op2(op) << ansii_reset;

            flag = ' ';

            if (state_shown) {
                if (get_op2r(op)) {
                    if (i == reg_user_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = 'R'; }
                } else {
                    if (i == mem_user_1_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '1'; }
                    if (i == mem_user_2_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '2'; }
                }
                state_shown = false;
            }
            std::cout << flag << ansii_reset;

        } else {
            std::cout << "        ";
        }
        std::cout << "| ";

        if (get_ex(op)) {
            if (state_shown) {
                std::cout << ansii_b_blue << ansii_f_white;
                state_shown = false;
            }
            std::cout << std::setw(6) << get_ex(op) << ansii_reset;
        } else {
            std::cout << "      ";
        }
        std::cout << " |";

        if (get_w(op)) {

            if (get_op2r(op)) {
                std::cout << "R";
            } else {
                std::cout << "M";
            }

            if (state_shown) {
                std::cout << ansii_b_red << ansii_f_white;
            }
            std::cout << std::setw(6) << get_w(op) << ansii_reset;

            flag = ' ';

            if (state_shown) {
                if (get_op2r(op)) {
                    if (i == reg_user_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = 'R'; }
                } else {
                    if (i == mem_user_1_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '1'; }
                    if (i == mem_user_2_pos) { std::cout << ansii_b_red << ansii_f_yellow; flag = '2'; }
                }
                state_shown = false;
            }
            std::cout << flag << ansii_reset;

        } else {
            std::cout << "        ";
        }
        std::cout << "| ";

        if (get_wait(op)) {
            if (state_shown) {
                std::cout << ansii_b_red << ansii_f_white;
            }
            std::cout << std::setw(6) << get_wait(op) << ansii_reset;
        } else {
            std::cout << "      ";
        }
        std::cout << " |\n";
    }
    std::cout << "-----------------------------------------------------------------------\n";

    //                                                           это ужасно.................................
    std::cout << "Commands left:   " << ops_count-ops_pos << "                                                             \n";
    std::cout << "Commands Done:   " << commands_done << "                                                             \n";
    std::cout << "Max stack size:  " << max_stack_size << "                                                             \n";
    std::cout << "Worst wait time: " << max_wait_time << "                                                             \n";
    std::cout << "Best wait time:  " << min_wait_time << "                                                             \n";

    if (commands_done > 0)     std::cout << "Avg time:        " << (double)sum/(double)commands_done << "                                                             \n"; else
        std::cout << "                                                                                                       \n";

    if (flag_move) std::cout << "                                                                         ";
    std::cout << std::endl;
}

void Conveyor::show_ops_list() {
    int op;

    std::cout << ansii_b_green << "F" << ansii_b_magenta << "D" << ansii_b_cyan << "OP1 " << ansii_b_yellow << "OP2 " << ansii_b_blue << "EX   " << ansii_b_red << "W   " << ansii_b_white << ansii_f_black << "T RR   VOID  " << ansii_reset << "     Type  Reg1  Reg2\n";

    for (int i = 0; i < ops_count; ++i) {
        //zeroes_on_left = __builtin_clz(ops_list[i]);
        op = ops_list[i];
        for (int bit_pos = 0; bit_pos < 32; ++bit_pos) {
            if (bit_pos == 0) std::cout << ansii_b_green;
            if (bit_pos == 1) std::cout << ansii_b_magenta;
            if (bit_pos > 1  && bit_pos < 6 ) std::cout << ansii_b_cyan;
            if (bit_pos > 5  && bit_pos < 10) std::cout << ansii_b_yellow;
            if (bit_pos > 9  && bit_pos < 15) std::cout << ansii_b_blue;
            if (bit_pos > 14 && bit_pos < 19) std::cout << ansii_b_red;
            if (bit_pos > 18) std::cout << ansii_b_white << ansii_f_black;
            std::cout << get_bit(op,bit_pos);
        }
        std::cout << ansii_reset;
        std::cout << "    ";
        std::cout << std::setw(5) << get_type(op) + 1 << ' ';
        std::cout << std::setw(5) << get_op1r(op)     << ' ';
        std::cout << std::setw(5) << get_op2r(op);
        std::cout << '\n';
    }
    std::cout << std::endl;
    std::cout << "Total OP:   " << ops_count << '\n';
    std::cout << "Type 1:     " << type_1_count << '\n';
    std::cout << "Type 2:     " << type_2_count << '\n';
    std::cout << "Type 3:     " << type_3_count << '\n';
    std::cout << "OP 1 Reg:   " << op1_reg << '\n';
    std::cout << "OP 1 Mem:   " << ops_count - op1_reg << '\n';
    std::cout << "OP 2 Reg:   " << op2_reg << '\n';
    std::cout << "OP 2 Mem:   " << ops_count - op2_reg << '\n';
    std::cout << std::endl;
}
