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






//TODO‌ refine this function





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


void printe(char * error_message){

    /*
     * This function will print error messages with customized style 
     *
     * */

    printf("\033[1;31m");
    printf("ERR<200c>: %s\n",error_message);
    printf("\033[0m");
}





