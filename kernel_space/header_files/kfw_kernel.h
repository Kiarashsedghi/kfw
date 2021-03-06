/*
 *
 *  THIS FILE CONTAINS KERNEL MODULE MAIN PROGRAM CONTROLLERS
 *
 *
 *
 *  Written By :  Kiarash Sedghi
 *
 *
 * */
#ifndef KFW_KFW_KERNEL_H
#define KFW_KFW_KERNEL_H

#include "linux/kfw_dstructures.h"


typedef struct kmc_controles  kmc_controles_t;


struct kmc_controles{



    onebyte_p_t user_command[MAX_LEN_USER_COMMAND];



    //---------------------------------------------------------------------------
    /*
     * kfw should maintain some global like variables.This is because of efficiency.
     * For example , whenever user wants to create new data , we had to allocate a new data_t,
     * fill the fields of that structure and then copy it to datas_cache array.This method gradually
     * increases memory usage , the solution can be maintaining some variables and pointers ,
     * and whenever we want to create for example a new data or a new rule , just by some definite
     * variables , handle all the operation.
     * */
    // AUX_data_st_ptr is useful when you create multiple datas_cache.It stores the address of the next
    // available data structure in datas_cache array so it is not necessary that for each data ,
    // you declare new variable to save its address.
    data_t *AUX_data_st_ptr;
    onebyte_p_t AUX_data_name[MAX_LEN_DATA_NAME];
    onebyte_p_t AUX_data_type;

    policy_t *AUX_policy_st_ptr;
    onebyte_p_t AUX_policy_name[MAX_LEN_POLICY_NAME];

    data_with_action_t *AUX_data_action_st_ptr;
    onebyte_p_t AUX_action_name[MAX_LEN_ACTION_NAME];

    onebyte_p_t AUX_interface_name[MAX_LEN_INTERFACE_NAME];
    onebyte_p_t AUX_policy_direction[MAX_LEN_POLICY_DIRECTION];



    // Again like above , we have decalare some pointers and variables for rules
    rule_t *AUX_rule_st_ptr;
    onebyte_p_t AUX_rule_type[MAX_LEN_RULE_TYPE];
    onebyte_p_t AUX_rule_value[MAX_LEN_RULE_VALUE];


    onebyte_p_t current_kfw_datas;
    onebyte_p_t current_kfw_policies;
    data_t datas[10];
    policy_t policies[10];

    onebyte_np_t AUX_functions_returns;
    int *port_number_array;
    int *port_number_array_start;

    unsigned int ip_byte;
    unsigned int wm_byte;
    struct mutex  thread_lock;


    kfwp_req_t *kfwpmss;
    kfwp_reply_t *kfwprepmss;

    struct udphdr *udph_t;
    struct tcphdr *tcph_t;
    char ip_addr[16];
    char wildcard_mask[16];
    char AUX_str_ptr;
    char *AUX_str_ptr_temp;

    struct sock *nl_sk ;
    struct nf_hook_ops *egress_kfwh ;
    struct nf_hook_ops *ingress_kfwh ;



    struct task_struct *talk2user_thread;
    struct task_struct *firewall_thread;


    onebyte_p_t *port_number_ptr;
    onebyte_p_t *port_number_ptr_temp;

    twobyte_p_t port_number;

    struct iphdr *iph_t;
    onebyte_p_t AUX_page[KFW_PAGE_SIZE];
    onebyte_p_t negation_flag;


    char protocol_name[8];



};




#endif //KFW_KFW_KERNEL_H
