#include <stdio.h>
#include "string.h"
#include "regex.h"
#include "kfw_types.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-sign"


int main() {


    // Initialize program by creating kfw_controls to control kfw.
    // Setting cli_mode to 0 which is global mode of kfw
    regex__t kfw_regex;
    kfw_controls_t kfw_controls;
    kfw_controls.current_kfw_datas=0;
    kfw_controls.current_kfw_policies=0;
    bzero(&(kfw_controls.user_command),MAX_LEN_USER_COMMAND);

    // clearing kfw_controls.AUX_data_name
    bzero(kfw_controls.AUX_data_name,MAX_LEN_DATA_NAME);

    // clearing kfw_controls.AUX_policy_name
    bzero(kfw_controls.AUX_policy_name,MAX_LEN_POLICY_NAME);


    bzero(kfw_controls.AUX_rule_type,MAX_LEN_RULE_TYPE);
    bzero(kfw_controls.AUX_rule_type,MAX_LEN_RULE_VALUE);


    setup_kfw_commands_regex(&kfw_regex);





    while(1){
          printf("kfw> ");
          fgets(kfw_controls.user_command,MAX_LEN_USER_COMMAND,stdin);


        // data definition
        if(regexec(&(kfw_regex.regex_data_definition), kfw_controls.user_command, 0, NULL, 0) ==0) {


            /* When user issues ( data DATA_NAME ) , first we should check
             * whether user has already defined a data with type DATA_NAME or not.
             *
             * So first we split data command to find the type and type of data specified by user.
             * We store the type and type in AUX variables.
            */

            // clear kfw_controls.AUX_data_name for new name
            // TODO‌ it is not necessary > bzero(kfw_controls.AUX_data_name,strlen(kfw_controls.AUX_data_name));

            split_data_definition_command(kfw_controls.user_command, kfw_controls.AUX_data_name, &kfw_controls.AUX_data_type,1,0);

            // check existence of a data with type specified by command which is stored in AUX_data_name.
            // If it does exist we get index of it , else we get -1.
            kfw_controls.res=get_index_of_data_in_datas(&kfw_controls,kfw_controls.AUX_data_name);


            // A data with type AUX_data_name does not exist so we allocate new one in
            // kfw_controls.datas
            if(kfw_controls.res==-1){
                kfw_controls.AUX_data_st_ptr=&(kfw_controls.datas[kfw_controls.current_kfw_datas]);
                // zero all that structure ( initialize )
                bzero(kfw_controls.AUX_data_st_ptr,sizeof(data_t));

                kfw_controls.AUX_data_st_ptr->type=kfw_controls.AUX_data_type;

                strcpy(kfw_controls.AUX_data_st_ptr->name, kfw_controls.AUX_data_name);

                // update total number of datas in kfw datas
                kfw_controls.current_kfw_datas++;
            }
            else {
                /* We should check whether data_type specified by user_command matches
                 * data_type of the data we have found.
                 *
                 * Users Cannot change the type whenever they enter data definition mode.
                */
                if(kfw_controls.datas[kfw_controls.res].type!=kfw_controls.AUX_data_type){
                    printe("Cannot change type for data %s\nTo change the type first delete the type and recreate it"); //TODO‌
                    continue;
                }
                else
                    kfw_controls.AUX_data_st_ptr = &(kfw_controls.datas[kfw_controls.res]);
            }


            while(1) {
                printf("kfw-data> ");
                // Clear the user command.Make all the written bytes zero
                bzero(kfw_controls.user_command,strlen(kfw_controls.user_command));
                fgets(kfw_controls.user_command, MAX_LEN_USER_COMMAND, stdin);

                //rule_definition(with overwrite ability)
                if (regexec(&kfw_regex.regex_rule_definition, kfw_controls.user_command, 0, NULL, 0) == 0) {


                    /* split rule command and put the type and value of the rule to
                     *  rules array of (founded/created) data which are :
                     *         kfw_controls.AUX_rule_type
                     *         kfw_controls.AUX_rule_value
                    */
                    bzero(kfw_controls.AUX_rule_type,strlen(kfw_controls.AUX_rule_type));
                    split_rule_definition_command(kfw_controls.user_command, kfw_controls.AUX_rule_type, kfw_controls.AUX_rule_value, 0, 1);


                    kfw_controls.res=get_index_of_rule_in_rules(kfw_controls.AUX_data_st_ptr,kfw_controls.AUX_rule_type);


                    if(kfw_controls.res==-1){
                        kfw_controls.AUX_rule_st_ptr = &(kfw_controls.AUX_data_st_ptr->rules[kfw_controls.AUX_data_st_ptr->current_rules]);
                        //zero the data_with_action
                        bzero(kfw_controls.AUX_rule_st_ptr,sizeof(rule_t));

                        strcpy(kfw_controls.AUX_rule_st_ptr->type,kfw_controls.AUX_rule_type);
                        strcpy(kfw_controls.AUX_rule_st_ptr->value,kfw_controls.AUX_rule_value);

                        //update total number of rules in data
                        kfw_controls.AUX_data_st_ptr->current_rules++;


                    }
                    else{

                        kfw_controls.AUX_rule_st_ptr = &(kfw_controls.AUX_data_st_ptr->rules[kfw_controls.res]);
                        bzero(kfw_controls.AUX_rule_st_ptr->value,strlen(kfw_controls.AUX_rule_st_ptr->value));
                        strcpy((kfw_controls.AUX_rule_st_ptr->value),kfw_controls.AUX_rule_value);

                    }


                }

                // rule deletion
                else if (regexec(&kfw_regex.regex_rule_deletion, kfw_controls.user_command, 0, NULL, 0) == 0) {
                    /*
                     * Deletion policy:
                     *  First we find the type and value of the rule based on splitting
                     *  rule command.Then we search these values in the rules array of the
                     *  (founded/created) data.After finding the index of that rule , we start
                     *  shifting the right side rules to the left.For efficiency purposes the logic is
                     *  as below‌:
                     *          if ( the rule was the last rule in the array){
                     *
                     *              // checking this is because an array with one element
                     *              // can serves its first element as its last element , so
                     *              // we have to check if it has only 1 element in itself
                     *              // decrementing is not necessary and the reason is
                     *              // whenever we want to add new element to any array in this program
                     *              // we use bzero() to clear its previous data , so we are not worried about
                     *              // previous data of first element.
                     *
                     *              if(decrementing of total number does not lead to -1)
                     *                  just decrement one from total number of rules in the array
                     *          }
                     *          else{
                     *             go to the next rule;
                     *             start copying each rules bytes to the previous index
                     *         }

                     * */


                    onebyte_p_t rule_type[MAX_LEN_RULE_TYPE];
                    onebyte_p_t rule_value[MAX_LEN_RULE_VALUE];
                    bzero(rule_type, MAX_LEN_RULE_TYPE);
                    bzero(rule_value, MAX_LEN_RULE_VALUE);

                    split_rule_definition_command(kfw_controls.user_command, rule_type, rule_value, 1, 2);

                    // Start searching for the rule based on Type  and Value
                    for (int i = 0; i < kfw_controls.AUX_data_st_ptr->current_rules; i++)
                        if (strcmp(kfw_controls.AUX_data_st_ptr->rules[i].type, rule_type) == 0 &&
                            strcmp(kfw_controls.AUX_data_st_ptr->rules[i].value, rule_value) == 0) {
                            // here we have found the rule

                            // check if the rule is the last one
                            if (i == kfw_controls.AUX_data_st_ptr->current_rules - 1) {
                                if(kfw_controls.AUX_data_st_ptr->current_rules-1!=-1)
                                    kfw_controls.AUX_data_st_ptr->current_rules--;
                            }
                            else {
                                // i++ : go to next rule
                                i++;
                                // start copying each rule to the previous index ( shifting to the left)
                                while (i <= kfw_controls.AUX_data_st_ptr->current_rules - 1) {
                                    memcpy(&kfw_controls.AUX_data_st_ptr->rules[i - 1], &kfw_controls.AUX_data_st_ptr->rules[i], sizeof(rule_t));
                                    i++;
                                }
                            }
                            break;
                        }
                    // update total number of rules
                    kfw_controls.AUX_data_st_ptr->current_rules--;
                    printf("rule removed \n");
                }

                // quick clear
                else if (regexec(&kfw_regex.regex_quick_clear, kfw_controls.user_command, 0, NULL, 0) == 0) {
                    // For quick clear , we just make total number of rules to zero.
                    // The first implementation was to make all the bytes of rules array zero ,
                    // but for efficiency purposes it was ignored.
                    kfw_controls.AUX_data_st_ptr->current_rules = 0;

//                    bzero(kfw_controls.AUX_data_st_ptr->rules, (kfw_controls.AUX_data_st_ptr->current_rules) * sizeof(rule_t));
                }

                // quick  show
                else if (regexec(&kfw_regex.regex_quick_show, kfw_controls.user_command, 0, NULL, 0) == 0) {
                    for (int i = 0; i < kfw_controls.AUX_data_st_ptr->current_rules; i++) {
                        printf("%s %s\n", kfw_controls.AUX_data_st_ptr->rules[i].type, kfw_controls.AUX_data_st_ptr->rules[i].value);
                    }
                }

                // issuing back
                else if (regexec(&kfw_regex.regex_back_to_previous_mode, kfw_controls.user_command, 0, NULL, 0) == 0) {
                        break;
                }
                }
        }

        // policy definition
        else if(regexec(&kfw_regex.regex_policy_definition, kfw_controls.user_command, 0, NULL, 0) ==0){

            // clear kfw_controls.AUX_policy_name for new name
            bzero(kfw_controls.AUX_policy_name,strlen(kfw_controls.AUX_policy_name));
            split_policy_definition_command(kfw_controls.user_command,kfw_controls.AUX_policy_name,1,0);


            kfw_controls.res=get_index_of_policy_in_policies(&kfw_controls,kfw_controls.AUX_policy_name);


            // A policy with name AUX_policy_name does not exist so we allocate new one in
            // kfw_controls.policies
            if(kfw_controls.res==-1){
                kfw_controls.AUX_policy_st_ptr=&(kfw_controls.policies[kfw_controls.current_kfw_policies]);

                // zero all that structure ( initialize )
                bzero(kfw_controls.AUX_policy_st_ptr, sizeof(policy_t));

                strcpy(kfw_controls.AUX_policy_st_ptr->name, kfw_controls.AUX_policy_name);

                // update total number of policies in kfw policies
                kfw_controls.current_kfw_policies++;
            }
            else {
                printf("found one\n");
                kfw_controls.AUX_policy_st_ptr = &(kfw_controls.policies[kfw_controls.res]);
            }


            while(1) {
                printf("kfw-policy> ");
                // Clear the user command.Make all the written bytes zero
                bzero(kfw_controls.user_command,strlen(kfw_controls.user_command));

                fgets(kfw_controls.user_command, MAX_LEN_USER_COMMAND, stdin);

                // data_action definition( with overwrite ability )
                if (regexec(&kfw_regex.regex_data_action_definition, kfw_controls.user_command, 0, NULL, 0) == 0) {
                    split_data_with_action_command(kfw_controls.user_command,kfw_controls.AUX_data_name,kfw_controls.AUX_action_name,0,1);
                    kfw_controls.res=get_index_of_data_in_datas(&kfw_controls,kfw_controls.AUX_data_name);
                    if(kfw_controls.res==-1)
                        printe("This data does not exist"); //TODO‌ %s

                    else {

                        kfw_controls.res=get_index_of_datawithaction_in_policies(kfw_controls.AUX_policy_st_ptr,kfw_controls.AUX_data_name);

                        if(kfw_controls.res==-1){

                            kfw_controls.AUX_data_action_st_ptr = &(kfw_controls.AUX_policy_st_ptr->data_with_actions[kfw_controls.AUX_policy_st_ptr->current_data_actions]);

                            //zero the data_with_action
                            bzero(kfw_controls.AUX_data_action_st_ptr, sizeof(data_with_action_t));

                            strcpy(kfw_controls.AUX_data_action_st_ptr->data_name,kfw_controls.AUX_data_name);
                            strcpy(kfw_controls.AUX_data_action_st_ptr->action,kfw_controls.AUX_action_name);

                            // update total number of data_with_action entities in the policy
                            kfw_controls.AUX_policy_st_ptr->current_data_actions++;

                        }
                        else {
                            kfw_controls.AUX_data_action_st_ptr = &(kfw_controls.AUX_policy_st_ptr->data_with_actions[kfw_controls.res]);
                            bzero(&(kfw_controls.AUX_data_action_st_ptr->action), MAX_LEN_ACTION_NAME);
                            strcpy(kfw_controls.AUX_data_action_st_ptr->action,kfw_controls.AUX_action_name);
                        }
                    }
                }

                else if (regexec(&kfw_regex.regex_back_to_previous_mode, kfw_controls.user_command, 0, NULL, 0) ==0) {
                    break;
                }

                // data_action deletion
                else if (regexec(&kfw_regex.regex_data_action_deletion, kfw_controls.user_command, 0, NULL, 0) ==0) {
                    split_data_with_action_command(kfw_controls.user_command,kfw_controls.AUX_data_name,kfw_controls.AUX_action_name,1,2);
                    kfw_controls.res=get_index_of_data_in_datas(&kfw_controls,kfw_controls.AUX_data_name);

                    // delete policy is same as rule deletion in data structure
                    if(kfw_controls.res!=-1){
                        if(kfw_controls.res==kfw_controls.AUX_policy_st_ptr->current_data_actions-1) {
                            if(kfw_controls.AUX_policy_st_ptr->current_data_actions-1!=-1)
                                kfw_controls.AUX_policy_st_ptr->current_data_actions--;
                        }
                        else{
                            kfw_controls.res++;
                            while(kfw_controls.res<=kfw_controls.AUX_policy_st_ptr->current_data_actions-1){
                                memcpy(&kfw_controls.AUX_policy_st_ptr->data_with_actions[kfw_controls.res-1],&kfw_controls.AUX_policy_st_ptr->data_with_actions[kfw_controls.res],sizeof(data_with_action_t));
                                kfw_controls.res++;
                            }

                            //update total number of data_actions
                            kfw_controls.AUX_policy_st_ptr->current_data_actions--;
                        }
                    }
                }

                // quick show
                else if (regexec(&kfw_regex.regex_quick_show, kfw_controls.user_command, 0, NULL, 0) ==0) {
                    for(int i=0;i<kfw_controls.AUX_policy_st_ptr->current_data_actions;i++){
                        printf("%s %s\n",kfw_controls.AUX_policy_st_ptr->data_with_actions[i].data_name,kfw_controls.AUX_policy_st_ptr->data_with_actions[i].action);
                    }
                }
                // quick clear
                else if (regexec(&kfw_regex.regex_quick_clear, kfw_controls.user_command, 0, NULL, 0) ==0) {
                    // For quick clear , we just make total number of data_with_actions to zero.
                    // The first implementation was to make all the bytes of data_with_actions array zero ,
                    // but for efficiency purposes it was ignored.
                    kfw_controls.AUX_policy_st_ptr->current_data_actions = 0;
                    printf("quick clear issued \n");
                    // bzero for clear
                }

//                // clear user_command for next commands
//                // one copy of user_command does exist in user_command_ns
//                bzero(kfw_controls.user_command, strlen(kfw_controls.user_command));

            }

        }

        // data deletion
        else if(regexec(&kfw_regex.regex_data_deletion, kfw_controls.user_command, 0, NULL, 0) ==0) {
            /*#TODO important
             * Data deletion command acceptance:
             *          no data DATA_NAME (all/any)?   * specifying (any/all) is optional and does not effect deletion
             *
             * */

            // when remove data check policy dependency
            split_data_definition_command(kfw_controls.user_command, kfw_controls.AUX_data_name, &kfw_controls.AUX_data_type,2,2);

            kfw_controls.res=get_index_of_data_in_datas(&kfw_controls,kfw_controls.AUX_data_name);

            if(kfw_controls.res!=-1){
                //TODO check policy_dependency



                // Delete the data from datas array.
                // Deletion policy is same as before.
                if(kfw_controls.res==kfw_controls.current_kfw_datas-1) {
                    if (kfw_controls.current_kfw_datas - 1 != -1)
                        kfw_controls.current_kfw_datas--;
                }
                else{
                    kfw_controls.res++;
                    while(kfw_controls.res<=kfw_controls.current_kfw_datas-1){
                        memcpy(&kfw_controls.datas[kfw_controls.res-1],&kfw_controls.datas[kfw_controls.res],sizeof(data_t));
                        kfw_controls.res++;
                    }
                    //update total number of datas
                    kfw_controls.current_kfw_datas--;
                    printf("data deletion issued");

                }

            }






        }

        // policy deletion
        else if(regexec(&kfw_regex.regex_policy_deletion, kfw_controls.user_command, 0, NULL, 0) ==0) {
            // clear kfw_controls.AUX_policy_name for new name
            bzero(kfw_controls.AUX_policy_name,strlen(kfw_controls.AUX_policy_name));
            split_policy_definition_command(kfw_controls.user_command,kfw_controls.AUX_policy_name,2,2);

            kfw_controls.res=get_index_of_policy_in_policies(&kfw_controls,kfw_controls.AUX_policy_name);

            // deletion policy is same as before
            if(kfw_controls.res!=-1){
                if(kfw_controls.res==kfw_controls.current_kfw_policies-1) {
                    if(kfw_controls.current_kfw_policies-1!=-1)
                        kfw_controls.current_kfw_policies--;


                }
                else{
                    kfw_controls.res++;
                    while(kfw_controls.res<=kfw_controls.current_kfw_policies-1){
                        memcpy(&kfw_controls.policies[kfw_controls.res-1],&kfw_controls.policies[kfw_controls.res],sizeof(policy_t));
                        kfw_controls.res++;
                    }
                    //update total number of policies
                    kfw_controls.current_kfw_policies--;
                }

                printf("del<%s>\n",kfw_controls.AUX_policy_name);

                printf("policy deletion issued\n");
            }


        }

        // show data
        else if (regexec(&kfw_regex.regex_show_data_command, kfw_controls.user_command, 0, NULL, 0) ==0){
            split_data_definition_command(kfw_controls.user_command, kfw_controls.AUX_data_name, &kfw_controls.AUX_data_type,2,4);

            kfw_controls.res=get_index_of_data_in_datas(&kfw_controls,kfw_controls.AUX_data_name);

            if(kfw_controls.res!=-1){
                // store the address of found data structure in kfw_controls.AUX_data_st_ptr
                kfw_controls.AUX_data_st_ptr=&(kfw_controls.datas[kfw_controls.res]);

                for (int i = 0; i < kfw_controls.AUX_data_st_ptr->current_rules; i++) {
                    printf("%s %s\n", kfw_controls.AUX_data_st_ptr->rules[i].type, kfw_controls.AUX_data_st_ptr->rules[i].value);
                }

                printf("show data issued\n");

            }

        }

        // show policy
        else if (regexec(&kfw_regex.regex_show_policy_command, kfw_controls.user_command, 0, NULL, 0) ==0){
            split_policy_definition_command(kfw_controls.user_command,kfw_controls.AUX_policy_name,2,4);

            kfw_controls.res=get_index_of_policy_in_policies(&kfw_controls,kfw_controls.AUX_policy_name);

            if(kfw_controls.res!=-1){
                kfw_controls.AUX_policy_st_ptr=&(kfw_controls.policies[kfw_controls.res]);
                for(int i=0;i<kfw_controls.AUX_policy_st_ptr->current_data_actions;i++){
                    printf("%s %s\n",kfw_controls.AUX_policy_st_ptr->data_with_actions[i].data_name,kfw_controls.AUX_policy_st_ptr->data_with_actions[i].action);
                }
                printf("show policy issued\n");
            }

        }

        // quit / exit
        else if (regexec(&kfw_regex.regex_quit_exit, kfw_controls.user_command, 0, NULL, 0) ==0){
            printf("Bye!!\n");
            return 0;
        }


//        else if(strlen(kfw_controls.user_command)==0){
//            printf("inja\n"); //TODO
//            continue;
//        }
//        else{
//            printe("Ambiguous command");
//
//        }

}




    return 0;
}

onebyte_np_t get_index_of_rule_in_rules(data_t *data_st ,onebyte_p_t *rule_type ){
    for(int i=0;i<data_st->current_rules;i++){
        if (strncmp(data_st->rules[i].type, rule_type, strlen(rule_type)) == 0)
            return i;
    }
    return -1;


}

onebyte_np_t get_index_of_datawithaction_in_policies(policy_t *policy , onebyte_p_t *data_name){
    for(int i=0;i<policy->current_data_actions;i++){
        if(strncmp(policy->data_with_actions[i].data_name,data_name,strlen(data_name))==0)
            return i;
    }
    return -1;
}

onebyte_np_t get_index_of_data_in_datas(kfw_controls_t *kfw_controls,onebyte_p_t *data_name){

    for(int i=0;i<kfw_controls->current_kfw_datas;i++)
        if(strncmp(kfw_controls->datas[i].name,data_name,strlen(data_name))==0){
            return i;
        }
    return -1;

}

onebyte_np_t get_index_of_policy_in_policies(kfw_controls_t *kfw_controls,onebyte_p_t *policy_name){

    for(int i=0;i<kfw_controls->current_kfw_policies;i++)
        if(strncmp(kfw_controls->policies[i].name,policy_name,strlen(policy_name))==0){
            return i;
        }
    return -1;

}

void split_data_with_action_command(onebyte_p_t *data_with_action_cmd,onebyte_p_t *data_name,onebyte_p_t *action , onebyte_p_t data_name_pos ,onebyte_p_t action_pos){
    onebyte_p_t data_with_action_command_ele=-1;
    onebyte_p_t *temp;

    // TODO‌ do this on other functions of splitting
    bzero(data_name,strlen(data_name));
    bzero(action,strlen(action));

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

void split_policy_definition_command(onebyte_p_t *policy_def,onebyte_p_t *policy_name , onebyte_p_t name_pos ,onebyte_p_t show_or_no_len){

    onebyte_p_t policy_command_ele=-1;
    onebyte_p_t *temp;
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

void split_rule_definition_command(onebyte_p_t *rule_def,onebyte_p_t *rule_name,onebyte_p_t *rule_value , onebyte_p_t name_pos ,onebyte_p_t value_pos){
    onebyte_p_t rule_command_ele=-1;
    onebyte_p_t *temp;
    while(*rule_def){
        if(*rule_def==32 || *rule_def==10 || *rule_def==9)
            rule_def++;
        else{
            rule_command_ele++;
            if(rule_command_ele==0 && name_pos==1)
                rule_def+=2;
            else if(rule_command_ele==name_pos){
                temp=rule_def;
                while(*rule_def!=32 && *rule_def!=10 && *rule_def!=9)
                    rule_def++;
                memcpy(rule_name,temp,rule_def-temp);
            }
            else if(rule_command_ele==value_pos){
                temp=rule_def;
                while(*rule_def!=32 && *rule_def!=10 && *rule_def!=9)
                    rule_def++;
                memcpy(rule_value,temp,rule_def-temp);
                // we put break to end the function at this point because
                // we have collected all the things we need
                break;
            }
        }

    }
}

void split_data_definition_command(onebyte_p_t * data_def , onebyte_p_t *data_name ,onebyte_p_t *type , onebyte_p_t data_name_pos ,onebyte_p_t show_or_no_len){
    /*
     * show_or_no_len==0 means don't care
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

void setup_kfw_commands_regex(regex__t *kfwregex){
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
    if (regcomp(&kfwregex->regex_nothing_entered,REGEX_WHITESPACE,REG_EXTENDED) != 0) {
        printe("white_space regex compilation error");
    }
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

void strip_space(onebyte_p_t * str , onebyte_p_t * dst){
    /*
     * This function is designed to strip spaces around any input‌(usuall user command)
     *
     * */

    char *beg_no_space=str;
    char *end_no_space=str;

    bzero(dst,strlen(dst));

    while(*end_no_space)
        end_no_space++;
    end_no_space--;

    while(*beg_no_space==32 ||*beg_no_space==10|| *beg_no_space==9 )
        beg_no_space++;

    while(*end_no_space==32  ||*end_no_space==10 || *end_no_space==9)
        end_no_space--;

    memcpy(dst,beg_no_space,end_no_space-beg_no_space+1);

}

void printe(char * error_message){
    /*
     * This function will print error message with red color
     * */
    printf("\033[1;31m");
    printf("ERR<200c>: %s\n",error_message);
    printf("\033[0m");
}


