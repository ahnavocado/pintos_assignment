#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"

struct robot* robots;

// test code for central control node thread
void test_cnt(){
        while(1){
                print_map(robots, 4);
                thread_sleep(1000);
                block_thread();
        }
}

// test code for robot thread
void test_thread(void* aux){
        int idx = *((int *)aux);
        int test = 0;
        while(1){
                printf("thread %d : %d\n", idx, test++);
                thread_sleep(idx * 1000);
        }
}

// entry point of simulator
void run_automated_warehouse(char **argv)
{       
        init_automated_warehouse(argv); // do not remove this

        printf("implement automated warehouse!\n");
        

        char *ret_ptr;
        char *next_ptr;
        int robot_count = 1;
        
        robots = malloc(sizeof(struct robot) * atoi(argv[1])); // get number of robots
        tid_t* threads = malloc(sizeof(tid_t) * atoi(argv[1]));
        int *idxs = malloc(sizeof(int)* atoi(argv[1]));
        for (int i = 1; i < atoi(argv[1]) + 1; i++) // thread id index
                idxs[i] = i;
        

        threads[0] = thread_create("CNT", 0, &test_cnt, NULL);
        
        ret_ptr = strtok_r(argv[2], ":", &next_ptr); //initialize token parsing
        while(ret_ptr) {
                //printf("ret_ptr = [%s]\n", ret_ptr);
                char name_buffer[4];
                snprintf(name_buffer,4,"R%d",robot_count);
                
                *idxs = robot_count;
                setRobot(&robots[robot_count-1], name_buffer, 5, 5, ret_ptr[1], 0);
                threads[robot_count] = thread_create(name_buffer, 0, &test_thread, &idxs[robot_count-1]); // thread idx starts from 1 
                ret_ptr = strtok_r(NULL, ":", &next_ptr);
                robot_count++;
        }
        

        // robot_count = 1;
        // ret_ptr = strtok_r(argv[2], ":", &next_ptr); //initialize token parsing
        // while(ret_ptr) {
        //         //printf("ret_ptr = [%s]\n", ret_ptr);
        //         char name_buffer[20];
        //         snprintf(name_buffer,20,"R%d",robot_count);
                
        //         *idxs = robot_count;
        //         // setRobot(&robots[robot_count-1], name_buffer, 5, 5, ret_ptr[1], 0);
        //         threads[robot_count-1] = thread_create(name_buffer, 0, &test_thread, &idxs[robot_count-1]);
        //         ret_ptr = strtok_r(NULL, ":", &next_ptr);
        //         robot_count++;
        // }

        // test case robots
        // robots = malloc(sizeof(struct robot) * 4);
        // setRobot(&robots[0], "R1", 5, 5, 0, 0);
        // setRobot(&robots[1], "R2", 0, 2, 0, 0);
        // setRobot(&robots[2], "R3", 1, 1, 1, 1);
        // setRobot(&robots[3], "R4", 5, 5, 0, 0);

        // example of create thread
        // tid_t* threads = malloc(sizeof(tid_t) * 4);
        // int idxss[4] = {1, 2, 3, 4};
        // threads[0] = thread_create("CNT", 0, &test_cnt, NULL);
        // threads[1] = thread_create("R1", 0, &test_thread, &idxss[1]);
        // threads[2] = thread_create("R2", 0, &test_thread, &idxss[2]);
        // threads[3] = thread_create("R3", 0, &test_thread, &idxss[3]);

        // if you want, you can use main thread as a central control node
        
}