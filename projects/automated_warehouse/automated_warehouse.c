#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
int path_map[100][20] = { 0, }; 

void printMessage(struct message msg) {
    printf("Robot Status - ");
    printf("Row: %d  ", msg.row);
    printf("Column: %d  ", msg.col);
    printf("Payload: %d  ", msg.current_payload);
    printf("Req Payload: %d  ", msg.required_payload);
    printf("Next Command: %d\n", msg.cmd);
}
void send_message_from_robot(int robot_id, struct message msg) {
    
    //printMessage(msg);
    //printf("67890 - %d\n", robot_id);
    boxes_from_robots[robot_id].msg = msg;
    if (!boxes_from_robots[robot_id].dirtyBit) {
        boxes_from_robots[robot_id].dirtyBit = 1; // new message created
    }
    block_thread();

}
void send_message_from_central_control_node(int robot_id, struct message msg) {
    boxes_from_central_control_node[robot_id].msg = msg;
    boxes_from_central_control_node[robot_id].dirtyBit = 1; // new message written
    //printMessage(msg);
}
struct message receive_message_from_robot(int robot_id) {
   
    while (!boxes_from_robots[robot_id].dirtyBit) {
        // wait until new message
        printf("NO message from robot -- %d \n ",robot_id);
    }
    //struct message msg = box->msg;
    //printMessage(boxes_from_robots[robot_id].msg);
    boxes_from_robots[robot_id].dirtyBit = 0; // read msg
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
    //printMessage(boxes_from_central_control_node[robot_id].msg);
    boxes_from_central_control_node[robot_id].dirtyBit = 0; // read msg
    return boxes_from_central_control_node[robot_id].msg;
}

void create_path(int robot_idx, int package, int destination){
    int count = 0;
    int package_row, package_col, enterance_to_pkg; //enterance_to_pkg-> how to get into and come back from the package area | up first(1) or down first(-1)
    //int dest_row, dest_col;
    if (package == '1'){
        package_row = 2;
        package_col = 1;
        enterance_to_pkg = 1;
    }
    else if (package == '2'){
        package_row = 2;
        package_col = 3;
        enterance_to_pkg = 1;
    }
    else if (package == '3'){
        package_row = 2;
        package_col = 4;
        enterance_to_pkg = 1;
    }
    else if (package == '4'){
        package_row = 2;
        package_col = 5;
        enterance_to_pkg = 1;
    }
    else if (package == '5'){
        package_row = 3;
        package_col = 1;
        enterance_to_pkg = -1;
    }
    else if (package == '6'){
        package_row = 3;
        package_col = 3;
        enterance_to_pkg = -1;
    }
    else if (package == '7'){
        package_row = 3;
        package_col = 4;
        enterance_to_pkg = -1;
    }
    
    //------------------------------------------
    path_map[robot_idx][count] = 12;
    count++;
    path_map[robot_idx][count] = 12;
    count++;
    for (int i = 0; i < abs(3 - package_row); i++){
        path_map[robot_idx][count] = 12;
        count++;
    }
    for (int i = 0; i < abs(5 - package_col); i++){
        path_map[robot_idx][count] = 9;
        count++;
    }

    //----------------------------------

    if (enterance_to_pkg == 1){
        path_map[robot_idx][count] = 12;
        count++;
        //current payload +1
        path_map[robot_idx][count] = 6;
        count++;
    }
    else if (enterance_to_pkg == -1){
        path_map[robot_idx][count] = 6;
        count++;
        //current payload +1
        path_map[robot_idx][count] = 12;
        count++;
    }
    //-------------------------------------

    if (package_row <= 2){
        while(package_row != 2){
            path_map[robot_idx][count] = 6;
            package_row ++;
            count++;
        }
    }
    else if (package_row > 2){
        while(package_row != 2){
            path_map[robot_idx][count] = 12;
            package_row--;
            count++;
        }
    }

    if (package_col <= 2){
        while(package_col != 2){
            path_map[robot_idx][count] = 3;
            package_col++;
            count++;
        }
    }
    else if (package_col > 2){
        while(package_col != 2){
            path_map[robot_idx][count] = 9;
            package_col--;
            count++;
        }
    }

    if (destination == 'A'){
        path_map[robot_idx][count] = 12;
        count++;
        path_map[robot_idx][count] = 12;
        count++;
    }
    else if (destination == 'B'){
        path_map[robot_idx][count] = 9;
        count++;
        path_map[robot_idx][count] = 9;
        count++;
    }
    else if (destination == 'C'){
        path_map[robot_idx][count] = 6;
        count++;
        path_map[robot_idx][count] = 6;
        count++;
        path_map[robot_idx][count] = 6;
        count++;
    }
}

// code for central control node thread
void cnt_thread(){
        int flag = 0;
        
        for(int current_robot = 0; current_robot <= num_robot; current_robot++){
            for(int command = 0;;command++){
                if(path_map[current_robot][command] == 0){
                    break;
                }
                
                struct message msg = {0,0,0,0,0};
                for(int i = 0; i < num_robot ; i++){
                    //printf("sending msg\n");
                    send_message_from_central_control_node(i,msg);
                }
                
                struct message mod_msg = {0,0,0,0,path_map[current_robot][command]};
                send_message_from_central_control_node(current_robot,mod_msg);
                
                

                unblock_threads(); // unblock all threads

                // wait for reply
                while(flag == 0){
                    for (int j = 0; j < num_robot ; j++){
                        if (boxes_from_robots[j].dirtyBit == 0){
                            flag = 0;
                            break;
                        }
                        flag = 1;
                    }
                    //printf("waiting for reply\n");
                    thread_sleep(50);
                    
                }
                flag = 0;

                // read reply msg
                for(int i = 0; i < num_robot; i++){
                    //printf("reading msg\n");
                    receive_message_from_robot(i);
                    
                }
                print_map(robots, num_robot);
                thread_sleep(1000);
                // block_thread();

            }
        }
        
    
        
}



// code for robot thread 물류 로봇은 중앙 관제 노드가 정해준 행동을 취한 후 자신의 상태 정보를 관제 노드에게 전달 후 Block
void robot_thread(void* aux){
        int idx = *((int *)aux);
        while(1){
            struct message receive_msg = receive_message_central_control_node(idx-1);
            
            //printMessage(receive_msg);

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
            send_message_from_robot(idx-1, send_msg);
            
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
        char name_buffer[100][4];
        
        
        // make robot
        
        robots = malloc(sizeof(struct robot) * atoi(argv[1])); // get number of robots
        boxes_from_central_control_node = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        boxes_from_robots = malloc(sizeof(struct message) * atoi(argv[1])); // Dynamic allocation msg box
        tid_t* threads = malloc(sizeof(tid_t) * atoi(argv[1]));  // Create thread
        int *idxs = malloc(sizeof(int)* atoi(argv[1]));
        // for (int i = 1; i < atoi(argv[1]) + 1; i++) // thread id index
        //         idxs[i] = i;
        tid_t* main_thread;
        main_thread = thread_create("CNT", 0, &cnt_thread, NULL); // creat main thread 
        
        // create thread for each robot
        ret_ptr = strtok_r(argv[2], ":", &next_ptr); //initialize token parsing
        for(int k = 0; k < num_robot; k++){
                printf("ret_ptr = [%s]\n", ret_ptr);
                snprintf(name_buffer[robot_count-1],4,"R%d",robot_count);
                
                idxs[robot_count-1] = robot_count;
                
                setRobot(&robots[robot_count], name_buffer[robot_count - 1], 5, 5, ret_ptr[0], 0);




                threads[robot_count] = thread_create(name_buffer, 0, &robot_thread, &idxs[robot_count-1]); 
                create_path(robot_count-1, ret_ptr[0],ret_ptr[1]);
                ret_ptr = strtok_r(NULL, ":", &next_ptr);
                robot_count++;
                //printf("%d___%s____%s___%s_________\n", robot_count-1 ,robots[robot_count-1].name,robots[0].name,robots[1].name);
        }
        
        //print path_map
        // for (int i = 0; i < 7; i++)    
        // {
        //     for (int j = 0; j < 20; j++)    
        //     {
        //         printf("%d ", path_map[i][j]); 
        //     }
        //     printf("\n");                
        // }
        
        
}