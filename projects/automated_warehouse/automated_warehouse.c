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
int num_robot ;

void printMessage(struct message msg) {
    printf("Robot Status - ");
    printf("Row: %d  ", msg.row);
    printf("Column: %d  ", msg.col);
    printf("Payload: %d  ", msg.current_payload);
    printf("Req Payload: %d  ", msg.required_payload);
    printf("Next Command: %d\n\n", msg.cmd);
}
void send_message_from_robot(int robot_id, struct message msg) {
    //struct message_box* box = &boxes_from_robots[robot_id];
    printMessage(msg);
    printf("67890 - %d\n", robot_id);
    boxes_from_robots[robot_id].msg = msg;
    if (!boxes_from_robots[robot_id].dirtyBit) {
        boxes_from_robots[robot_id].dirtyBit = 1; // new message created
    }
    block_thread();

}
void send_message_from_central_control_node(int robot_id, struct message msg) {
    //struct message_box* box = &boxes_from_central_control_node[robot_id];
    
    printMessage(msg);
    printf("12345 - %d\n", robot_id);
    if (!boxes_from_central_control_node[robot_id].dirtyBit) {
        boxes_from_central_control_node[robot_id].msg = msg;
        boxes_from_central_control_node[robot_id].dirtyBit = 1; // new message written
    }
    
}
struct message receive_message_from_robot(int robot_id) {
    //struct message_box* box = &boxes_from_robots[robot_id];
    while (!boxes_from_robots[robot_id].dirtyBit) {
        // wait until new message
        printf("NO message from robot -- %d \n ",robot_id);
    }
    //struct message msg = box->msg;
    printMessage(boxes_from_robots[robot_id].msg);
    boxes_from_robots[robot_id].dirtyBit = 0; // read msg
    // for( int i = 0; i++; i < sizeof(boxes_from_robots) / sizeof(msg)){
    //     struct message_box* box = &boxes_from_robots[i];
    //     if(box -> dirtyBit == 1)
    //     return msg;
    // }
    // unblock_threads();
    return boxes_from_robots[robot_id].msg;
}
struct message receive_message_central_control_node(int robot_id) {
    //struct message_box* box = &boxes_from_central_control_node[robot_id];
    while (!boxes_from_central_control_node[robot_id].dirtyBit) {
        // wait until new message
        printf("NO message from central node -- %d\n",robot_id);
        thread_sleep(100);
    }
    //struct message msg = box->msg;
    printMessage(boxes_from_central_control_node[robot_id].msg);
    boxes_from_central_control_node[robot_id].dirtyBit = 0; // read msg
    return boxes_from_central_control_node[robot_id].msg;
}

// test code for central control node thread
void test_cnt(){
        while(1){
                print_map(robots, 5);
                thread_sleep(10);
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

// code for central control node thread
void cnt_thread(){
        int flag = 0;
        while(1){
            print_map(robots, 5);
            for(int i = 1; i <= num_robot; i++){
                printf("sending msg\n");
                struct message msg = {0,0,0,0,12};
                send_message_from_central_control_node(i,msg);
            }
            unblock_threads(); // unblock all threads

            while(flag == 0){
                for (int j = 1; j <= num_robot ; j++){
                    if (boxes_from_robots[j].dirtyBit == 0){
                        flag = 0;
                        break;
                    }
                    flag = 1;
                }
                printf("waiting for reply\n");
                thread_sleep(1000);
            }
            flag = 0;
            for(int i = 1; i <= num_robot; i++){
                printf("reading msg\n");
                receive_message_from_robot(i);
            }
            print_map(robots, 5);
            thread_sleep(1000);
            // block_thread();
        }
        
}


// code for robot thread 물류 로봇은 중앙 관제 노드가 정해준 행동을 취한 후 자신의 상태 정보를 관제 노 드에게 전달 후 Block
void robot_thread(void* aux){
        int idx = *((int *)aux);
        while(1){
            struct message receive_msg = receive_message_central_control_node(idx);
            
            // struct robot* __robot = &robots[idx];
            
            if(receive_msg.cmd == 3){
                robots[idx].col += 1;
            }
            else if(receive_msg.cmd == 6){
                robots[idx].row += 1;
            }
            else if(receive_msg.cmd == 9){
                robots[idx].col -= 1;
            }
            else if(receive_msg.cmd == 12){
                robots[idx].row -= 1;
            }
            struct message send_msg =  {robots[idx].row,robots[idx].col,robots[idx].current_payload,robots[idx].required_payload};
            send_message_from_robot(idx, send_msg);
            //block_thread();
        }

}




// entry point of simulator
void run_automated_warehouse(char **argv)
{       
        //pintos/threads/build
        //../../utils/pintos automated_warehouse 5 2A:4C:2B:2C:3A
        init_automated_warehouse(argv); // do not remove this

        printf("implement automated warehouse!\n");
        

        char *ret_ptr;
        char *next_ptr;
        int robot_count = 1;
        num_robot = sizeof(struct robot) * atoi(argv[1])/20;
        
        
        // make robot
        robots = malloc(sizeof(struct robot) * atoi(argv[1])); // get number of robots
        boxes_from_central_control_node = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        boxes_from_robots = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        tid_t* threads = malloc(sizeof(tid_t) * atoi(argv[1]));  // Create thread
        int *idxs = malloc(sizeof(int)* atoi(argv[1]));
        for (int i = 1; i < atoi(argv[1]) + 1; i++) // thread id index
                idxs[i] = i;
        threads[0] = thread_create("CNT", 0, &cnt_thread, NULL); // creat main thread (idx == 0)
        
        // create thread for each robot
        ret_ptr = strtok_r(argv[2], ":", &next_ptr); //initialize token parsing
        while(ret_ptr) {
                printf("ret_ptr = [%s]\n", ret_ptr);
                
                char name_buffer[4];
                snprintf(name_buffer,4,"R%d",robot_count);
                *idxs = robot_count;
                setRobot(&robots[robot_count-1], name_buffer, 5, 5, ret_ptr[1], 0);
                threads[robot_count] = thread_create(name_buffer, 0, &robot_thread, &idxs[robot_count-1]); // thread idx starts from 1 
                ret_ptr = strtok_r(NULL, ":", &next_ptr);
                robot_count++;
                
        }
        
        
}