/*
 *
 *  THIS FILE CONTAINS KFW MAIN FUNCTIONS
 *
 *
 *
 *  Written By :  Kiarash Sedghi
 *
 *
 * */


#include "string.h"
#include "stdio.h"
#include "regex.h"
#include <unistd.h>
#include <stdlib.h>
#include "../header_files/kfw_user.h"
#include "../header_files/kfw_user_functions.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-sign"


twobyte_p_t  talk2module(consistency_flags_t *consistencyFlags, kfw_controls_t *kfw_controls, kfwp_controls_t *kfwp_controls , onebyte_p_t type, onebyte_p_t*arg1, onebyte_p_t  *arg2, onebyte_p_t *arg3, data_t *data_ptr , policy_t * policy_ptr, ingress_policies_t *ingress_policies_ptr, egress_policies_t *egress_policies_t){
    /*
     * This function talks with kernel module with KFWP protocol.
     *
     * It sends request with specific type (arg: type) and receives KFWP replies and if
     * the reply indicates that kernel is going to send data to send data,this function will
     * receive those datas and writes theme in specified destinations on the cache:
     *              data_ptr
     *              policy_ptr
     *              ingress_policies_ptr
     *              egress_policies_ptr
     *
     * or will allocate those spaces on the cache and then writes those datas.
     *
     * */


    kfwp_controls->nlh=NULL;

    kfwp_controls->sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);

    if(kfwp_controls->sock_fd<0)
        return -1;

    memset(&kfwp_controls->src_addr, 0, sizeof(kfwp_controls->src_addr));
    kfwp_controls->src_addr.nl_family = AF_NETLINK;
    kfwp_controls->src_addr.nl_pid = getpid(); /* self pid */

    bind(kfwp_controls->sock_fd, (struct sockaddr*)&kfwp_controls->src_addr, sizeof(kfwp_controls->src_addr));

    memset(&kfwp_controls->dest_addr, 0, sizeof(kfwp_controls->dest_addr));
    kfwp_controls->dest_addr.nl_family = AF_NETLINK;
    kfwp_controls->dest_addr.nl_pid = 0; /* For Linux Kernel */
    kfwp_controls->dest_addr.nl_groups = 0; /* unicast */


    kfwp_controls->nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(1024));
    memset(kfwp_controls->nlh, 0, NLMSG_SPACE(1024));
    kfwp_controls->nlh->nlmsg_len = NLMSG_SPACE(1024);
    kfwp_controls->nlh->nlmsg_pid = getpid();
    kfwp_controls->nlh->nlmsg_flags = 0;



    kfwp_controls->iov.iov_base = (void *)kfwp_controls->nlh;
    kfwp_controls->iov.iov_len = kfwp_controls->nlh->nlmsg_len;
    kfwp_controls->msg.msg_name = (void *)&kfwp_controls->dest_addr;
    kfwp_controls->msg.msg_namelen = sizeof(kfwp_controls->dest_addr);
    kfwp_controls->msg.msg_iov = &kfwp_controls->iov;
    kfwp_controls->msg.msg_iovlen = 1;





    // create new request message called kfwp_req_msg
    kfwp_controls->kfwp_req_msg=(kfwp_req_t *)malloc(sizeof(kfwp_req_t));

    // set all bits of the request message to 0
    bzero(kfwp_controls->kfwp_req_msg, sizeof(kfwp_req_t));

    // set request message type
    kfwp_controls->kfwp_req_msg->type=type;

    // arg1 should not be copied for (show datas , show policies) commands
    // in those commands , arg1 is NULL
    if(arg1!=NULL)
        strcpy(kfwp_controls->kfwp_req_msg->arg1, arg1);

    // This checking is for (show datas and show policies) that does not have arg2 like policy definition
    if(arg2!=NULL) {
        // for data definition , arg2 is holding only the type of data which is only 1 byte
        // so we are copying only 1 byte to arg2
        if (type == 0b00000000)
            memcpy(kfwp_controls->kfwp_req_msg->arg2, arg2, 1);
        else
            memcpy(kfwp_controls->kfwp_req_msg->arg2, arg2, strlen(arg2));
    }
    // This checking is for commands except service command that do not have
    // arg3
    if (arg3 != NULL)
        memcpy(kfwp_controls->kfwp_req_msg->arg3, arg3, strlen(arg3));



    // copy data from the message to nlh
    memcpy(NLMSG_DATA(kfwp_controls->nlh), kfwp_controls->kfwp_req_msg, sizeof(kfwp_req_t));

    printd("Request created\n");


    sendmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
    printd("Request sent to the kernel\n");


    free(kfwp_controls->kfwp_req_msg);


    // create new reply message called kfwp_rep_msg
    kfwp_controls->kfwp_rep_msg=(kfwp_reply_t *)malloc(4);


    recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);

    printd("reply received from kernel\n");



    // copy the reply from nlh to kfwp_rep_msg
    memcpy(kfwp_controls->kfwp_rep_msg, NLMSG_DATA(kfwp_controls->nlh), 4);


    // checking status of the reply
    if((kfwp_controls->kfwp_rep_msg->status) == 0b00000000) {
        if (type == 0b00000000) {

            // if page_cnt was not 0 means kernel told us that it is going to
            // send us data
            if (kfwp_controls->kfwp_rep_msg->page_cnt != 0) {

                if (data_ptr == NULL) {
                    kfw_controls->AUX_data_st_ptr = &kfw_controls->datas_cache[kfw_controls->current_kfw_datas];
                    data_ptr=kfw_controls->AUX_data_st_ptr;
                    kfw_controls->current_kfw_datas++;
                    printd("new data allocated on cache\n");
                }

                // receiving data from kernel
                for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++) {
                    recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
                    if (i == kfwp_controls->kfwp_rep_msg->page_cnt - 1) {
                        memcpy((void *) data_ptr + i * kfwp_controls->kfwp_rep_msg->page_size,
                               NLMSG_DATA(kfwp_controls->nlh),
                               sizeof(data_t) - i * kfwp_controls->kfwp_rep_msg->page_size + 1);
                    }else
                        memcpy((void *) data_ptr + i * kfwp_controls->kfwp_rep_msg->page_size,
                               NLMSG_DATA(kfwp_controls->nlh), kfwp_controls->kfwp_rep_msg->page_size);
                }

                // set consistency flag to 1
                data_ptr->consistency=1;


                close(kfwp_controls->sock_fd);

                // return number of bytes written to the cache
                return  sizeof(data_t);

            } else
                printd("no data was sent from kernel\n");
        }

        else if(type == 0b10000000)
            printd("data deleted successfully!\n");

        else if(type == 0b10000010)
            printd("policy deleted successfully!\n");

        else if(type==0b00001110){

            // receiving data from kernel
            // receiving only headers of each data from kernel
            for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++){
                recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
                memcpy((void *)&kfw_controls->datas_cache + i * sizeof(data_t) , NLMSG_DATA(kfwp_controls->nlh) , kfwp_controls->kfwp_rep_msg->page_size);

                // make consistency of each data in cache to 0
                kfw_controls->datas_cache[i].consistency=0;
            }

            printd("writing headers to cache completed\n");

            //update number of current datas in cache
            kfw_controls->current_kfw_datas=kfwp_controls->kfwp_rep_msg->page_cnt;

            // set data_cache consistency flag to 1
            // later show datas does not need any request to be sent
            consistencyFlags->data_cache=1;


        }


        else if(type==0b00001111){
            // receving data from kernel
            for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++){
                recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);

                memcpy((void *)&kfw_controls->policies_cache + i * sizeof(policy_t) , NLMSG_DATA(kfwp_controls->nlh) , kfwp_controls->kfwp_rep_msg->page_size);

                // make consistency of each policy in cache to 0
                kfw_controls->policies_cache[i].consistency=0;
            }
            printd("writing headers to cache completed\n");

            //update number of current policy in cache
            kfw_controls->current_kfw_policies=kfwp_controls->kfwp_rep_msg->page_cnt;

            // set policy_cache consistency flag to 1
            // later show policies does not need request
            consistencyFlags->policy_cache=1;

        }


        else if (type == 0b00000010){
            // if page_cnt was not 0 means kernel told us that it is going to
            // send us data
            if (kfwp_controls->kfwp_rep_msg->page_cnt != 0) {

                if (data_ptr == NULL) {

                    kfw_controls->AUX_policy_st_ptr= &kfw_controls->policies_cache[kfw_controls->current_kfw_policies];
                    policy_ptr=kfw_controls->AUX_policy_st_ptr;
                    kfw_controls->current_kfw_policies++;
                    printd("new policy allocated on cache\n");
                }

                // receving data from kernel
                for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++) {
                    recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
                    if (i == kfwp_controls->kfwp_rep_msg->page_cnt - 1) {
                        memcpy((void *) policy_ptr + i * kfwp_controls->kfwp_rep_msg->page_size,
                               NLMSG_DATA(kfwp_controls->nlh),
                               sizeof(policy_t) - i * kfwp_controls->kfwp_rep_msg->page_size + 1);
                    }else
                        memcpy((void *) policy_ptr + i * kfwp_controls->kfwp_rep_msg->page_size,
                               NLMSG_DATA(kfwp_controls->nlh), kfwp_controls->kfwp_rep_msg->page_size);
                }

                // set consistency flag to 1
                policy_ptr->consistency=1;


                close(kfwp_controls->sock_fd);

                // return number of bytes written to the cache
                return  sizeof(policy_t);

            } else
                printd("no policy was sent from kernel\n");

        }


            // updating ingress cache
        else if (type==0b00001000){

            // receiving data from kernel
            for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++){
                recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
                memcpy((void *)&ingress_policies_ptr->policyWithInterfaces + i * sizeof(policy_with_int_t) , NLMSG_DATA(kfwp_controls->nlh) , kfwp_controls->kfwp_rep_msg->page_size);
            }

            printd("writing ingress policies  to cache finished\n");

            //set total number of ingress policies_cache
            ingress_policies_ptr->current_ingress_policies=kfwp_controls->kfwp_rep_msg->page_cnt;

            // set consistency flag to 1
            consistencyFlags->ingress_policy_cache=1;

        }


            // updating egress cache
        else if (type==0b00001001){
            // receiving data from kernel
            for(int i=0;i<kfwp_controls->kfwp_rep_msg->page_cnt; i++){
                recvmsg(kfwp_controls->sock_fd, &kfwp_controls->msg, 0);
                memcpy((void *)&egress_policies_t->policyWithInterfaces + i * sizeof(policy_with_int_t) , NLMSG_DATA(kfwp_controls->nlh) , kfwp_controls->kfwp_rep_msg->page_size);
            }
            printd("writing egress policies  to cache finished\n");

            //set total number of egress policies_cache
            egress_policies_t->current_egress_policies=kfwp_controls->kfwp_rep_msg->page_cnt;

            // set consistency flag to 1
            consistencyFlags->egress_policy_cache=1;
        }

        else if(type==0b00000001 || type==0b10000001)
            printd("rule modified(added/deleted/changed) successfully\n");


        else if(type==0b00000011 || type==0b10000011)
            printd("data_with_action modified(added/deleted/changed) successfully\n");


        else if(type == 0b01111110)
            printd("clear command cleared all rules successfully\n");

        else if(type == 0b01111111)
            printd("clear command cleared all data actions successfully\n");

        else if(type==0b00000100){
            printd("service command issued successfully\n");

            // setting consistency flag
            // check whether arg3 was in or out because it is important to change ingress or egress
            if(strcmp(arg3, "in") == 0) {
                printd("ingress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->ingress_policy_cache = 0;
            }
            else{
                printd("egress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->egress_policy_cache = 0;
            }
        }


        else if(type==0b10000100){
            printd("(no) service command issued successfully\n");

            // setting consistency flag
            // check whether arg3 was in or out because it is important to change ingress or egress
            if(strcmp(arg3, "in") == 0) {
                printd("ingress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->ingress_policy_cache = 0;
            }
            else{
                printd("egress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->egress_policy_cache = 0;
            }
        }

    }

    else if((kfwp_controls->kfwp_rep_msg->status ) == 0b00000001) {

        if(type==0b00000000)//TODO
            printe("Cannot change type for data %s\nTo change the type first delete the type and recreate it"); //TODO‌


        else if(type==0b00000011)
            printe("data does not exist"); //TODO‌ name of data


        else if(type==0b10000010) {
            printf("policy cannot be deleted because an ingress policy depends on it\n");//TODO
            close(kfwp_controls->sock_fd);
            // return one is just to tell that the policy has not been deleted
            // and it is useful to find out whether should we delete the cache or not
            return 1;
        }
        else if(type == 0b00000100 || type==0b10000100)
            printf("policy does not exist\n"); //TODO nameof polic

    }

    else if ((kfwp_controls->kfwp_rep_msg->status) == 0b00000010){
        if(type==0b10000000) {
            printf("data does not exist for deletion\n");
            close(kfwp_controls->sock_fd);
            // return one is just to tell that the data has not been deleted
            // and it is useful to find out whether should we delete the cache or not
            return 1;
        }

        else if(type==0b10000010) {
            printf("policy cannot be deleted because an egress policy depends on it\n");
            close(kfwp_controls->sock_fd);
            // return one is just to tell that the policy has not been deleted
            // and it is useful to find out whether should we delete the cache or not
            return 1;

        }


        else if(type==0b10000100)
            printf("policy does exist on kfw but was not set on this interface in this direction\n");
    }

    else if ((kfwp_controls->kfwp_rep_msg->status) == 0b00000011){
        if(type==0b10000000) {
            printf("data cannot be deleted because a policy depends on it\n");//TODO name
            close(kfwp_controls->sock_fd);
            // return one is just to tell that the data has not been deleted
            // and it is useful to find out whether should we delete the cache or not
            return 1;
        }
        else if(type==0b10000010) {
            printf("policy does not exist for deletion\n");//TODO name
            close(kfwp_controls->sock_fd);
            // return one is just to tell that the policy has not been deleted
            // and it is useful to find out whether should we delete the cache or not
            return 1;

        }

        else if (type==0b10000100)
            printf("there has no policy set on the interface specified\n");
    }

    else if((kfwp_controls->kfwp_rep_msg->status) == 0b00000100){
        if(type==0b00000000)
            printf("data does not exist\n");//TODO name
        else if(type==0b00000010)
            printf("policy does not exist\n");//TODO name
    }

    else if((kfwp_controls->kfwp_rep_msg->status) == 0b10000000){
        if(type==0b00000100) {

            printf("policy existed and updated \n");//TODO name delete?

            // setting consistency flag
            // check whether arg3 was in or out because it is important to change ingress or egress
            if(strcmp(arg3, "in") == 0) {
                printf("ingress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->ingress_policy_cache = 0;
            }
            else{
                printf("egress consistency flag set to 0(inconsistent)\n");
                consistencyFlags->egress_policy_cache = 0;
            }

        }
    }

    close(kfwp_controls->sock_fd);

    return 0;


}



onebyte_np_t getindex_data_in_data_cache(kfw_controls_t *kfw_controls, onebyte_p_t *data_name){

    /*
     * This function searches for a data structure in data cache space which its name
     * is data_name.
     *
     * If such data structure found , this function will return the index of that structure in cache
     * else, it will return -1
     *
     * */

    for(int i=0;i<kfw_controls->current_kfw_datas;i++)
        if(strncmp(kfw_controls->datas_cache[i].name, data_name, strlen(data_name)) == 0){
            return i;
        }
    return -1;

}

onebyte_np_t getindex_policy_in_policy_cache(kfw_controls_t *kfw_controls, onebyte_p_t *policy_name){

    /*
     * This function searches for a policy structure in policy cache space which its name
     * is policy_name.
     *
     * If such policy structure found , this function will return the index of that structure in cache
     * else, it will return -1
     *
     * */

    for(int i=0;i<kfw_controls->current_kfw_policies;i++)
        if(strncmp(kfw_controls->policies_cache[i].name, policy_name, strlen(policy_name)) == 0){
            return i;
        }
    return -1;

}

onebyte_p_t is_port_in_range(int *port_ranges, twobyte_p_t port, onebyte_p_t negation){
    /*
     * This function takes an array of ports (which is holding ports or range of ports)
     * and checks if a port number is in range of that ports array or not
     *
     * */

    while(*port_ranges!=-2){
        if(*(port_ranges+1)==-1) {
            if (port >= *(port_ranges) && port <= *(port_ranges + 2)) {
                return 1|negation;
            }
            else
                port_ranges += 3;
        }
        else{

            if(*port_ranges==port){
                return 1|negation;
            }
            port_ranges++;
        }
    }

    return 0|negation;
}



void split_service_policy_def_del_cmd(onebyte_p_t *service_policy_cmd, onebyte_p_t *policy_name, onebyte_p_t *interface_name , onebyte_p_t * direction,
                                      onebyte_p_t policy_name_pos , onebyte_p_t interface_name_pos, onebyte_p_t direction_pos){


    /*
     * This function will split the service definition/deletion commands.
     *
     * Arguments that indicating the position is because this function split both the
     * positive and negative form of the ( service command‌ ).
     *
     * For negative form all position arguments will be added in the program.
     *
     *
     * */

    onebyte_p_t service_policy_command_ele=-1;
    onebyte_p_t *temp;

    bzero(policy_name,strlen(policy_name));
    bzero(interface_name,strlen(interface_name));
    bzero(direction,strlen(direction));

    while(*service_policy_cmd){
        if(*service_policy_cmd==32 || *service_policy_cmd==10 || *service_policy_cmd==9)
            service_policy_cmd++;
        else{
            service_policy_command_ele++;
            if(service_policy_command_ele==0 && policy_name_pos==2)
                service_policy_cmd+=2;  // skipping (no) in negative form

            else if((service_policy_command_ele==0 && policy_name_pos==1)||(service_policy_command_ele==1 && policy_name_pos==2))
                service_policy_cmd+=7;  // skipping (service) in positive and negative form

            else{
                temp=service_policy_cmd;
                while(*service_policy_cmd!=32 && *service_policy_cmd!=10 && *service_policy_cmd!=9)
                    service_policy_cmd++;

                if(service_policy_command_ele==policy_name_pos)
                    memcpy(policy_name,temp,service_policy_cmd-temp);

                else if(service_policy_command_ele==interface_name_pos)
                    memcpy(interface_name,temp,service_policy_cmd-temp);

                else  if(service_policy_command_ele==direction_pos)
                    memcpy(direction,temp,service_policy_cmd-temp);
            }
        }
    }
}

void split_data_action_def_del_cmd(onebyte_p_t *data_with_action_cmd, onebyte_p_t *data_name, onebyte_p_t *action , onebyte_p_t data_name_pos , onebyte_p_t action_pos){

    /*
     * This function splits (data with mapped action) definition/deletion commands.
     *
     *
     * Argument that indicates the position is because this function split both the
     * positive and negative form of the command.
     *
     * For negative form the position argument will be added in the program.
     *
     * */


    onebyte_p_t data_with_action_command_ele=-1;
    onebyte_p_t *temp;


    while(*data_with_action_cmd){
        if(*data_with_action_cmd==32 || *data_with_action_cmd==10 || *data_with_action_cmd==9)
            data_with_action_cmd++;
        else{
            data_with_action_command_ele++;
            if(data_with_action_command_ele==0 && data_name_pos==1)
                data_with_action_cmd+=2;
            else if(data_with_action_command_ele==data_name_pos){
                temp=data_with_action_cmd;
                while(*data_with_action_cmd!=32 && *data_with_action_cmd!=10 && *data_with_action_cmd!=9)
                    data_with_action_cmd++;
                memcpy(data_name,temp,data_with_action_cmd-temp);
            }
            else if(data_with_action_command_ele==action_pos){
                temp=data_with_action_cmd;
                while(*data_with_action_cmd!=32 && *data_with_action_cmd!=10 && *data_with_action_cmd!=9)
                    data_with_action_cmd++;
                memcpy(action,temp,data_with_action_cmd-temp);
                // we put break to end the function at this point because
                // we have collected all the things we need
                break;
            }
        }

    }
}

void split_policy_def_del_show_cmd(onebyte_p_t *policy_def, onebyte_p_t *policy_name , onebyte_p_t name_pos , onebyte_p_t show_or_no_len){

    /*
    * This function splits policy definition/deletion/show commands.
    *
    *
    * Argument that indicates the position is because this function split both the
    * positive and negative form of the command.
    *
    * For negative form the position argument will be added in the program.
    *
    * show_or_no_len:
    *       > This argument indicates number of bytes that we should skip at first processing.
    *
    *      > For (show command) this argument is 4 because (show) is 4 bytes.
    *      > For (no) form this argument is 2 because (no) is 2 bytes.
    *      > For policy definition this argument is 0 because there is nothing to skip at first.
    *
    *      > We use this command in ( show policy POLICY_NAME ) or ( policy POLICY)NAME )
    *
    *
    * */


    onebyte_p_t policy_command_ele=-1;
    onebyte_p_t *temp;


    bzero(policy_name,strlen(policy_name));

    while(*policy_def){
        if(*policy_def==32 || *policy_def==10 || *policy_def==9)
            policy_def++;
        else{
            policy_command_ele++;
            if(policy_command_ele==0 && name_pos==2)
                policy_def += show_or_no_len;    // skipping (show/no) which is (4/2) characters and specified by onebyte_p_t show_or_no_len

                // we have to skip (policy) in two cases :
                // 1: we have positive form which (policy) is the first element ( name_pos = 1 , policy_pos = 0 )
                //                 or
                // 2: we have negative form which‌ (policy) is the second element ( name_pos = 2 , policy_pos = 1 )
            else if((policy_command_ele==0 && name_pos==1) || (policy_command_ele==1 && name_pos==2))
                policy_def += 6;   // skipping (policy) which is 6 characters

            else if(policy_command_ele==name_pos){
                temp=policy_def;
                while(*policy_def!=32 && *policy_def!=10 && *policy_def!=9)
                    policy_def++;
                memcpy(policy_name,temp,policy_def-temp);
                // we put break to end the function at this point because
                // we have collected all the things we need
                break;
            }
        }

    }
}

void split_rule_def_del_cmd(onebyte_p_t *rule_def, onebyte_p_t *rule_name, onebyte_p_t *rule_value , onebyte_p_t name_pos , onebyte_p_t value_pos){

    /*
   * This function splits rule definition/deletion commands.
   *
   *
   * Argument that indicates the position is because this function split both the
   * positive and negative form of the command.
   *
   * For negative form the position argument will be added in the program.
   *
   */

   bzero(rule_name,strlen(rule_name));
   bzero(rule_value,strlen(rule_value));

   onebyte_p_t rule_command_ele=-1;
   onebyte_p_t *temp;
   while(*rule_def){
       if(*rule_def==32 || *rule_def==10 || *rule_def==9)
           rule_def++;
       else {
           if (strncmp(rule_def, "not", 3) == 0) {
               *rule_value = (onebyte_p_t) '!';
               rule_value++;
               rule_def+=3;
           } else {
               rule_command_ele++;
               if (rule_command_ele == 0 && name_pos == 1)
                   rule_def += 2;
               else if (rule_command_ele == name_pos) {
                   temp = rule_def;
                   while (*rule_def != 32 && *rule_def != 10 && *rule_def != 9)
                       rule_def++;
                   memcpy(rule_name, temp, rule_def - temp);
               } else if (rule_command_ele == value_pos) {
                   temp = rule_def;
                   while (*rule_def != 32 && *rule_def != 10 && *rule_def != 9)
                       rule_def++;
                   memcpy(rule_value, temp, rule_def - temp);
                   // we put break to end the function at this point because
                   // we have collected all the things we need
                   break;
               }
           }
       }

   }
}

void split_data_def_del_cmd(onebyte_p_t * data_def , onebyte_p_t *data_name , onebyte_p_t *type , onebyte_p_t data_name_pos , onebyte_p_t show_or_no_len){

    /*
    * This function splits data definition/deletion commands.
    *
    *
    * Argument that indicates the position is because this function split both the
    * positive and negative form of the command.
    *
    * For negative form the position argument will be added in the program.
    *
    * show_or_no_len:
    *       > This argument indicates number of bytes that we should skip at first processing.
    *
    *      > For (show command) this argument is 4 because (show) is 4 bytes.
    *      > For (no) from this argument is 2 because (no) is 2 bytes;
    *      > For data definition this argument is 0 because there is nothing to skip at first.
    *
    *      > We use this command in ( show data DATA_NAME ) or ( policy DATA_NAME )
    *
    *
    * */


    onebyte_p_t data_command_ele=-1;
    onebyte_p_t *temp;

    bzero(data_name,strlen(data_name));

    while(*data_def){
        if(*data_def==32 || *data_def==10 || *data_def==9)
            data_def++;
        else{
            data_command_ele++;
            // skipping (data) which is 4 bytes
            // skipping (data) :
            //    when command is in positive form > (data) is at pos 0 && data_name is at pos 1
            //    when command is in negative form > (data) is at pos 1 && data_name is at pos 2

            if((data_command_ele==0 && data_name_pos==1) || (data_command_ele==1 && data_name_pos==2))
                data_def+=4;

            else if((data_command_ele==0 && data_name_pos==2))
                data_def+=show_or_no_len;     // skipping (show/no) which is (4/2) bytes and is specified by show_or_no_len

            else if(data_command_ele==2){
                temp=data_def;
                while(*data_def!=32 && *data_def!=10 && *data_def!=9)
                    data_def++;
                memcpy(data_name,temp,data_def-temp);
            }
            else if(data_command_ele==3){
                if(strncmp(data_def,"all",3)==0) {
                    *type = 1;
                    // break to finish the function
                    break;
                }
                else if(strncmp(data_def,"any",3)==0){
                    *type=0;
                    break;
                }
            }

        }
    }
    // if the user don't specify the type , default type is 0 which is match any
    if(data_command_ele!=3)
        *type=0;

}

void compile_kfw_cmds_regexes(regex__t *kfwregex){
    /*
     * This function will compile the regexes defined for kfw at program startup.
     *
     * */


    //TODO‌ we need exit code
    if (regcomp(&kfwregex->regex_data_definition,REGEX_DATA_DEFINITION,REG_EXTENDED) != 0) {
        printe("data_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_policy_definition,REGEX_POLICY_DEFINITION,REG_EXTENDED) != 0) {
        printe("policy_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_data_deletion,REGEX_DATA_DELETION,REG_EXTENDED) != 0) {
        printe("data_deletion regex compilation error");
    }
    if (regcomp(&kfwregex->regex_policy_deletion,REGEX_POLICY_DELETION,REG_EXTENDED) != 0) {
        printe("policy_deletion regex compilation error");
    }

    if (regcomp(&kfwregex->regex_service_policy_definition,REGEX_SERVICE_POLICY_DEFINITION,REG_EXTENDED) != 0) {
        printe("service_policy_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_service_policy_deletion,REGEX_SERVICE_POLICY_DELETION,REG_EXTENDED) != 0) {
        printe("service_policy_deletion regex compilation error");
    }

    if (regcomp(&kfwregex->regex_show_polices,REGEX_SHOW_POLICIES,REG_EXTENDED) != 0) {
        printe("show_policies regex compilation error");
    }
    if (regcomp(&kfwregex->regex_show_polices_with_dir,REGEX_SHOW_POLICIES_WITH_DIRECTION,REG_EXTENDED) != 0) {
        printe("show_policies_with_dir regex compilation error");
    }

    if (regcomp(&kfwregex->regex_show_policies_with_int,REGEX_SHOW_POLICIES_WITH_INTERFACE,REG_EXTENDED) != 0) {
        printe("show_policies_with_int regex compilation error");
    }

    if (regcomp(&kfwregex->regex_show_policies_with_int_dir,REGEX_SHOW_POLICIES_WITH_INTERFACE_DIR,REG_EXTENDED) != 0) {
        printe("show_policies_with_int_dir regex compilation error");
    }

    if (regcomp(&kfwregex->regex_show_datas,REGEX_SHOW_DATAS,REG_EXTENDED) != 0) {
        printe("show_datas regex compilation error");
    }

    if (regcomp(&kfwregex->regex_data_action_definition,REGEX_DATA_ACTION_DEFINITION,REG_EXTENDED) != 0) {
        printe("data_action regex compilation error");
    }
    if (regcomp(&kfwregex->regex_policy_definition,REGEX_POLICY_DEFINITION,REG_EXTENDED) != 0) {
        printe("policy_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_data_action_definition,REGEX_DATA_ACTION_DEFINITION,REG_EXTENDED) != 0) {
        printe("data_action_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_quit_exit,REGEX_QUIT_EXIT,REG_EXTENDED) != 0) {
        printe("quit_exit regex compilation error");
    }
    if (regcomp(&kfwregex->regex_rule_definition,REGEX_RULE_DEFINITION,REG_EXTENDED) != 0) {
        printe("rule_definition regex compilation error");
    }
    if (regcomp(&kfwregex->regex_rule_deletion,REGEX_RULE_DELETION,REG_EXTENDED) != 0) {
        printe("rule_deletion regex compilation error");
    }



    if (regcomp(&kfwregex->regex_back_to_previous_mode,REGEX_BACK_TO_PREVIOUS_MODE,REG_EXTENDED) != 0) {
        printe("back_to_previous_mode regex compilation error");
    }
    //TODO
//    if (regcomp(&kfwregex->regex_nothing_entered,REGEX_WHITESPACE,REG_EXTENDED) != 0) {
//        printe("white_space regex compilation error");
//    }
    if (regcomp(&kfwregex->regex_show_data_command,REGEX_SHOW_DATA_COMMAND,REG_EXTENDED) != 0) {
        printe("show_data regex compilation error");
    }
    if (regcomp(&kfwregex->regex_show_policy_command,REGEX_SHOW_POLICY_COMMAND,REG_EXTENDED) != 0) {
        printe("show_policy regex compilation error");
    }
    if (regcomp(&kfwregex->regex_quick_show,REGEX_QUICK_SHOW,REG_EXTENDED) != 0) {
        printe("quick_show regex compilation error");
    }
    if (regcomp(&kfwregex->regex_quick_clear,REGEX_QUICK_CLEAR,REG_EXTENDED) != 0) {
        printe("quick_clear regex compilation error");
    }
    if (regcomp(&kfwregex->regex_data_action_deletion,REGEX_DATA_ACTION_DELETION,REG_EXTENDED) != 0) {
        printe("data_action_deletion regex compilation error");
    }


}




void policy_del_cache(kfw_controls_t *kfw_controls){
    /*
     * This function will delete a specific policy from cache
     * Policy name was written in kfw_controls->AUX_policy_name
     * */
    kfw_controls->AUX_functions_returns= getindex_policy_in_policy_cache(kfw_controls,
                                                                        kfw_controls->AUX_policy_name);

    // deletion policy is same as before
    if(kfw_controls->AUX_functions_returns != -1){
        if(kfw_controls->AUX_functions_returns == kfw_controls->current_kfw_policies - 1) {
            if(kfw_controls->current_kfw_policies-1!=-1)
                kfw_controls->current_kfw_policies--;


        }
        else{
            kfw_controls->AUX_functions_returns++;
            while(kfw_controls->AUX_functions_returns <= kfw_controls->current_kfw_policies - 1){
                memcpy(&kfw_controls->policies_cache[kfw_controls->AUX_functions_returns - 1], &kfw_controls->policies_cache[kfw_controls->AUX_functions_returns], sizeof(policy_t));
                kfw_controls->AUX_functions_returns++;
            }
            //update total number of policies_cache
            kfw_controls->current_kfw_policies--;
        }
    }


}

void data_del_cache(kfw_controls_t *kfw_controls){
    /*
    * This function will delete a specific data from cache
    * Data name was written in kfw_controls->AUX_data_name
    * */

    kfw_controls->AUX_functions_returns= getindex_data_in_data_cache(kfw_controls,
                                                                    kfw_controls->AUX_data_name);

    if(kfw_controls->AUX_functions_returns != -1){
        // Delete the data from the cache
        // Delete the data from datas_cache array.
        // Deletion policy is same as before.
        if(kfw_controls->AUX_functions_returns == kfw_controls->current_kfw_datas - 1) {
            if (kfw_controls->current_kfw_datas - 1 != -1)
                kfw_controls->current_kfw_datas--;
        }
        else{
            kfw_controls->AUX_functions_returns++;
            while(kfw_controls->AUX_functions_returns <= kfw_controls->current_kfw_datas - 1){
                memcpy(&kfw_controls->datas_cache[kfw_controls->AUX_functions_returns - 1], &kfw_controls->datas_cache[kfw_controls->AUX_functions_returns], sizeof(data_t));
                kfw_controls->AUX_functions_returns++;
            }
            //update total number of datas in the cache
            kfw_controls->current_kfw_datas--;
        }
    }


}



void strnsplit(onebyte_p_t *str, onebyte_p_t position , onebyte_p_t * dst){

    /*
     * This function splits a string with whitespace( newline, space, horizental tab) delimiter,
     * and writes the specified(position) part that was created because of splitting to the (dst).
     *
     * This function usage in kfw is for (show commands)
     * */


    bzero(dst,strlen(dst));
    onebyte_p_t element_pos=-1;
    onebyte_p_t *temp;
    while(*str){
        if(*str==32 || *str==10 || *str==9)
            str++;
        else{
            element_pos++;
            if(element_pos==position){
                temp=str;
                while (*str != 32 && *str != 10 && *str != 9)
                    str++;
                memcpy(dst,temp,str-temp);
                break;
            }
            else
                while (*str != 32 && *str != 10 && *str != 9)
                    str++;
        }
    }
}

void printe(char * error_message){

    /*
     * This function will print error messages with customized style
     *
     * */

    printf("\033[1;31m");
    printf("ERR<200c>: %s\n",error_message);
    printf("\033[0m");
}


void printd(char * error_message){

    /*
     * This function will print error messages with customized style
     *
     * */

    printf("\033[1;33m");
    printf("ERR<200c>: %s\n",error_message);
    printf("\033[0m");
}

int power(int x,int p){
    /*
     * power(x,p) == x^p
     * */

    int q=1;
    for(int i=0;i<p;i++)
        q*=x;
    return q;
}

