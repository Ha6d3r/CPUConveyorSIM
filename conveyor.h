#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <malloc.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <ansii.h>

class Conveyor {
    public:
        Conveyor();
        ~Conveyor();

        bool cycle();

        void generate_operations(unsigned int seed, int count,
                                 double type_1_weight, double type_3_weight, double register_weight,
                                 int memory_speed, int type_1_speed, int type_2_speed);

        void show_ops_list();

        void show_stack();

    private:

        unsigned int  gen_op(int type, bool op1_register, bool op2_register, int command_speed, int memory_speed);

        void mov_stack_left(int pos);

        void clear();

        unsigned int * ops_list = nullptr;
        unsigned int * op_stack = nullptr;

        int ops_count       = 0;

        int ops_pos         = 0;
        int stack_pos       = 0;

        // paralel restrictions

        int cur_reg_users   = 0;
        int cur_mem_users   = 0;

        // stats

        int cycles_count    = 0;

        int max_wait_time   = 0;
        int min_wait_time   = 100000;

        int commands_done   = 0;
        unsigned int sum    = 0;

        int type_1_count    = 0;
        int type_2_count    = 0;
        int type_3_count    = 0;

        int op1_reg         = 0;
        int op2_reg         = 0;

        int max_stack_size  = 0;

        int reg_user_pos    = -1;
        int mem_user_1_pos  = -1;
        int mem_user_2_pos  = -1;

        bool flag_move;
};

#endif // CONVEYOR_H
