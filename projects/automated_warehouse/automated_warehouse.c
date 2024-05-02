#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/automated_warehouse/aw_manager.h"
#include "projects/automated_warehouse/aw_message.h"


/** message boxes from central control node to each robot */
struct message_box* boxes_from_central_control_node;
/** message boxes from robots to central control node */
struct message_box* boxes_from_robots;

struct robot* robots;

// test code for central control node thread
void test_cnt(){
        while(1){
                print_map(robots, 4);
                thread_sleep(1000);
                block_thread();
        }
}

void printMessage(struct message msg) {
    printf("Robot Status:\t");
    printf("Current Row: %d\t", msg.row);
    printf("Current Column: %d\t", msg.col);
    printf("Current Payload: %d\t", msg.current_payload);
    printf("Required Payload: %d\t", msg.required_payload);
    printf("Next Command: %d\n\n", msg.cmd);
}

// test code for robot thread
void test_thread(void* aux){
        int idx = *((int *)aux);
        int test = 0;
        // while(1){
        //         printf("thread %d : %d\n", idx, test++);
        //         thread_sleep(idx * 1000);
        // }
}

void send_message_from_robot(int robot_id, struct message msg) {
    struct message_box* box = &boxes_from_robots[robot_id];
    printMessage(msg);
    box->msg = msg;
    if (!box->dirtyBit) {
        box->dirtyBit = 1; // new message created
    }
    block_thread();

}
void send_message_from_central_control_node(int robot_id, struct message msg) {
    struct message_box* box = &boxes_from_central_control_node[robot_id];
    printMessage(msg);
    if (!box->dirtyBit) {
        box->msg = msg;
        box->dirtyBit = 1; // new message written
    }
    
}
struct message receive_message_from_robot(int robot_id) {
    struct message_box* box = &boxes_from_robots[robot_id];
    while (!box->dirtyBit) {
        // wait until new message
        printf("NO message from robot");
    }
    struct message msg = box->msg;
    printMessage(msg);
    box->dirtyBit = 0; // read msg
    for( int i = 0; i++; i < sizeof(boxes_from_robots) / sizeof(msg)){
        struct message_box* box = &boxes_from_robots[i];
        if(box -> dirtyBit == 1)
        return msg;
    }
    unblock_threads();
    return msg;
}
struct message receive_message_central_control_node(int robot_id) {
    struct message_box* box = &boxes_from_central_control_node[robot_id];
    while (!box->dirtyBit) {
        // wait until new message
        printf("NO message from central node");
    }
    struct message msg = box->msg;
    printMessage(msg);
    box->dirtyBit = 0; // read msg
    return msg;
}


// entry point of simulator
void run_automated_warehouse(char **argv)
{       
        init_automated_warehouse(argv); // do not remove this

        printf("implement automated warehouse!\n");
        

        char *ret_ptr;
        char *next_ptr;
        int robot_count = 1;
        
        // make robot
        robots = malloc(sizeof(struct robot) * atoi(argv[1])); // get number of robots
        boxes_from_central_control_node = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        boxes_from_robots = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        tid_t* threads = malloc(sizeof(tid_t) * atoi(argv[1]));  // Create thread
        int *idxs = malloc(sizeof(int)* atoi(argv[1]));
        for (int i = 1; i < atoi(argv[1]) + 1; i++) // thread id index
                idxs[i] = i;
        threads[0] = thread_create("CNT", 0, &test_cnt, NULL); // creat main thread (idx == 0)
        
        // create thread for each robot
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
        printf("dfsdfsf\n\n\n");
        
        struct message myMessage = {3, 4, 10, 15, 1};

        send_message_from_robot(1,myMessage);
 
       
        //pintos/threads/build
        //../../utils/pintos automated_warehouse 5 2A:4C:2B:2C:3A
        
}