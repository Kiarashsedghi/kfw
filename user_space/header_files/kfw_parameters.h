/*
 *  THIS FILE CONTAINS KFW MAIN PARAMETERS
 *
 *
 *  Written By :  Kiarash Sedghi
 *
 *
 * */


#ifndef KFW_KFW_PARAMETERS_H
#define KFW_KFW_PARAMETERS_H

/*
 *
 *  GLOBAL CONTROL PARAMETERS
 *
 *
 * */


// Parameters that indicate the length policies
#define MAX_LEN_RULE_TYPE 10
#define MAX_LEN_RULE_VALUE 40
#define MAX_LEN_DATA_NAME 10
#define MAX_LEN_POLICY_NAME 10
#define MAX_LEN_INTERFACE_NAME 10
#define MAX_LEN_ACTION_NAME 7
#define MAX_LEN_POLICY_DIRECTION 4
#define MAX_LEN_KFWP_ARG1 10
#define MAX_LEN_KFWP_ARG2 40
#define MAX_LEN_KFWP_ARG3 10
#define MAX_LEN_USER_COMMAND 40
// --------------------------------------------



// Parameters that control quantity policy
#define MAX_RULES_IN_DATA  10
#define MAX_DATA_IN_KFW  10
#define MAX_POLICY_IN_KFW 10
#define MAX_DATA_ACTIONS_IN_POLICY 20

#define MAX_INGRESS_POLICIES 100
#define MAX_EGRESS_POLICIES 100
// --------------------------------------------




// Parameters that control netlink
#define NETLINK_USER 31
#define MIN_SIZE_KFWP 37









/*
 *
 *
 *           REGEX PARAMETERS
 *
 *
 * */

#define REGEX_RULE_DEFINITION "^(\\s*(sip|dip)(\\s+not)?\\s+((([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5]))(\\/((([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])))?\\s*)$|^\\s*((proto)\\s+(not\\s+)?((udp|tcp|dhcp|icmp|igmp|ftp|telnet|smtp|pop3|imap|http|https|dns(\\/tcp|\\/udp)?))(,(udp|tcp|dhcp|icmp|igmp|ftp|telnet|smtp|pop3|imap|http|https|dns(\\/tcp|udp)?))*)\\s*$|^(\\s*(sudp|stcp|dtcp|dudp)(\\s+not)?\\s+(([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])(-([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?)(,(([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])(-([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?))*\\s*)$"


#define REGEX_RULE_DELETION "^(\\s*(no\\s+)(sip|dip)\\s*)$|^\\s*(no\\s+)(proto)\\s*$|^\\s*(no\\s+)(sudp|stcp|dtcp|dudp)\\s*$"


#define REGEX_DATA_DEFINITION "^\\s*data\\s+[0-9a-zA-Z_]+(\\s+(any|all))?\\s*$"
#define REGEX_DATA_DELETION "^\\s*(no\\s+)data\\s+[0-9a-zA-Z_]+(\\s+(any|all))?\\s*$"


#define REGEX_POLICY_DEFINITION "^\\s*policy\\s+[a-zA-Z_0-9]+\\s*$"
#define REGEX_POLICY_DELETION "^\\s*(no\\s+)policy\\s+[a-zA-Z_0-9]+\\s*$"


#define REGEX_DATA_ACTION_DEFINITION "^\\s*[a-zA-Z_0-9]+\\s+(permit|deny)\\s*$"
#define REGEX_DATA_ACTION_DELETION "^\\s*(no\\s+)[a-zA-Z_0-9]+(\\s+(permit|deny))?\\s*$"

#define REGEX_SERVICE_POLICY_DEFINITION "^\\s*service\\s+[a-zA-Z_0-9]+\\s+[a-zA-Z0-9_]+\\s+(in|out)\\s*$"
#define REGEX_SERVICE_POLICY_DELETION "^\\s*(no\\s+)service\\s+[a-zA-Z_0-9]+\\s+[a-zA-Z0-9_]+\\s+(in|out)\\s*$"


#define REGEX_QUICK_SHOW "^\\s*\\?\\s*$"
#define REGEX_QUICK_CLEAR "^\\s*clear\\s*$"


#define REGEX_SHOW_DATAS "^\\s*show\\s+datas\\s*$"
#define REGEX_SHOW_DATA_COMMAND "^\\s*show\\s+data\\s+[a-zA-Z0-9_]+\\s*$"


#define REGEX_SHOW_POLICIES "^\\s*show\\s+policies\\s*$"
#define REGEX_SHOW_POLICY_COMMAND "^\\s*show\\s+policy\\s+[a-zA-Z0-9_]+\\s*$"
#define REGEX_SHOW_POLICIES_WITH_DIRECTION "\\s*show\\s+policies\\s+(in|out)\\s*"
#define REGEX_SHOW_POLICIES_WITH_INTERFACE "^\\s*show\\s+policies\\s+[a-zA-Z_0-9]+\\s*$"
#define REGEX_SHOW_POLICIES_WITH_INTERFACE_DIR "^\\s*show\\s+policies\\s+[a-zA-Z_0-9]+\\s+(in|out)\\s*$"


#define REGEX_QUIT_EXIT "^\\s*(quit|exit)\\s*$"
#define REGEX_BACK_TO_PREVIOUS_MODE "^\\s*back\\s*$"









#endif //KFW_KFW_PARAMETERS_H



//#define REGEX_WHITESPACE "\\s+"




