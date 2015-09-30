#include "auth_comm.h"
#include "auth_ioc.h"
#include "auth_rule.h"

/*auth ip rule node*/
struct auth_ip_rule_node {
	struct list_head rule_node;
	struct auth_ip_rule ip_rule;
};

/*net interface node*/
struct if_info_node {
	struct list_head if_node;
	struct auth_if_info if_info;
};

struct auth_rule_config {
	struct list_head rule_list;
	struct auth_options auth_option;
	struct list_head if_list;
	enum AUTH_RULE_CONF_STAT_E status;
};

static struct auth_rule_config s_auth_cfg;


static void display_auth_ip_rule(struct auth_ip_rule *ip_rule)
{
	char ip_rule_type_str[IP_RULE_TYPE_NUM][IP_RULE_TYPE_STR_LEN] = {
		"NORMAL", "WHITE", "BLACK"};
	AUTH_DEBUG("--------IP_RULE BEGIN---------\n");
	AUTH_DEBUG("TYPE of ip rule: %s.\n", ip_rule_type_str[ip_rule->type]);
	AUTH_DEBUG("PRIORITY of ip rule: %d.\n", ip_rule->priority);
	AUTH_DEBUG("ENABLE of ip rule: %d.\n", ip_rule->enable);
	AUTH_DEBUG("MIN of ip rule: %d.\n", ip_rule->min);
	AUTH_DEBUG("MAX of ip rule: %d.\n", ip_rule->max);
	AUTH_DEBUG("--------IP_RULE END---------\n");
}


void display_auth_ip_rules(void)
{
	struct list_head *cur = NULL;
	struct auth_ip_rule_node *rule_node = NULL;
	list_for_each(cur, &s_auth_cfg.rule_list) {
		rule_node = list_entry(cur, struct auth_ip_rule_node, rule_node);
		display_auth_ip_rule(&rule_node->ip_rule);
	}
}


/*clean old auth rules*/
int clean_auth_rules(void)
{
	struct auth_ip_rule_node *rule_node = NULL;
	struct list_head *cur = NULL, *next = NULL;
#if DEBUG_ENABLE
	int free_cnt = 0;
#endif

	/*if don't check empty, will cause error.*/
	if (list_empty(&s_auth_cfg.rule_list)) {
	#if DEBUG_ENABLE
		AUTH_DEBUG("no rules clean.\n");
	#endif
		return 0;
	}
	/*notice: we cann't list entry directly.*/
	list_for_each_safe(cur, next, &s_auth_cfg.rule_list) {
		rule_node = list_entry(cur, struct auth_ip_rule_node, rule_node);
		list_del(cur);
	#if DEBUG_ENABLE
		display_auth_ip_rule(&rule_node->ip_rule);
		free_cnt ++;
	#endif
		kfree(rule_node);
		rule_node = NULL;
	}
	INIT_LIST_HEAD(&s_auth_cfg.rule_list);
#if DEBUG_ENABLE
	AUTH_DEBUG("Free %d rules totally.\n", free_cnt);
#endif
	return 0;
}


int add_auth_rule(struct auth_ip_rule_node *ip_rule_node)
{
	struct list_head *cur = NULL, *pre = NULL;
	struct auth_ip_rule_node *cur_node = NULL;

	if (list_empty(&s_auth_cfg.rule_list)) {
		list_add(&ip_rule_node->rule_node, &s_auth_cfg.rule_list);
		return 0;
	}
	/*notice: we cann't list entry directly.*/
	list_for_each_prev_safe(cur, pre, &s_auth_cfg.rule_list) {
		cur_node = list_entry(cur, struct auth_ip_rule_node, rule_node);
		if (cur_node->ip_rule.priority >= ip_rule_node->ip_rule.priority) {
			break;
		}
	}
	list_add(&ip_rule_node->rule_node, cur);
	return 0;
}


int copy_auth_ip_rule_to_node(struct auth_ip_rule_node *rule_node, 
										struct auth_ip_rule *ip_rule)
{
	struct auth_ip_rule *dst_ip_rule = &rule_node->ip_rule;
	dst_ip_rule->type = ip_rule->type;
	dst_ip_rule->priority = ip_rule->priority;
	dst_ip_rule->enable = ip_rule->enable;
	dst_ip_rule->min = ip_rule->min;
	dst_ip_rule->max = ip_rule->max;
	return 0;
}


static void display_auth_options(void)
{
	struct auth_options *options = &s_auth_cfg.auth_option;
	AUTH_DEBUG("--------AUTH_OPTIONS BEGIN---------\n");
	AUTH_DEBUG("USER_CHECK_INTVAL: %u.\n", options->user_check_intval);
	AUTH_DEBUG("REDIRECT_URL: %s.\n", options->redirect_url);
	AUTH_DEBUG("REDIRECT_TITLE: %s.\n", options->redirect_title);
	AUTH_DEBUG("--------AUTH_OPTIONS END---------\n");
}


static int auth_options_check(struct auth_options *options)
{
	if (options->user_check_intval <= 0) {
		return -1;
	}
	if (strlen(options->redirect_url) >= REDIRECT_URL_MAX) {
		return -1;
	}
	if (strlen(options->redirect_title) >= REDIRECT_TITLE_MAX) {
		return -1;
	}
	return 0;
}


int set_auth_options(struct auth_options *options)
{
	struct auth_options *dst_options = &s_auth_cfg.auth_option;
	if (auth_options_check(options) != 0) {
		return -1;
	}
	dst_options->user_check_intval = options->user_check_intval;
	memset(dst_options->redirect_url, 0, REDIRECT_URL_MAX);
	memcpy(dst_options->redirect_url, options->redirect_url, REDIRECT_URL_MAX - 1);
	memset(dst_options->redirect_title, 0, REDIRECT_TITLE_MAX);
	memcpy(dst_options->redirect_title, options->redirect_title, REDIRECT_TITLE_MAX - 1);
	display_auth_options();
	return 0;
}


void display_auth_if_info(struct auth_if_info *if_info)
{
	char if_type_str[NET_IF_TYPE_NUM][NET_IF_TYPE_STR_LEN] = {
		"LAN", "WAN", "LOOP"};
	AUTH_DEBUG("--------IF_INFO BEGIN---------\n");
	AUTH_DEBUG("TYPE of net interface: %s.\n", if_type_str[if_info->type]);
	AUTH_DEBUG("Name of net interface: %s.\n", if_info->if_name);
	AUTH_DEBUG("--------IF_INFO END---------\n");
}


void display_auth_if_infos(void)
{
	struct list_head *cur = NULL;
	struct if_info_node *if_node = NULL;
	list_for_each(cur, &s_auth_cfg.if_list) {
		if_node = list_entry(cur, struct if_info_node, if_node);
		display_auth_if_info(&if_node->if_info);
	}
}


int clean_auth_if_infos(void)
{
	struct if_info_node *if_node = NULL;
	struct list_head *cur = NULL, *next = NULL;
#if DEBUG_ENABLE
	int free_cnt = 0;
#endif

	/*if don't check empty, will cause error.*/
	if (list_empty(&s_auth_cfg.if_list)) {
	#if DEBUG_ENABLE
		AUTH_DEBUG("no net interface clean.\n");
	#endif
		return 0;
	}
	/*notice: we cann't list entry directly.*/
	list_for_each_safe(cur, next, &s_auth_cfg.if_list) {
		if_node = list_entry(cur, struct if_info_node, if_node);
		list_del(cur);
	#if DEBUG_ENABLE
		display_auth_if_info(&if_node->if_info);
		free_cnt ++;
	#endif
		kfree(if_node);
		if_node = NULL;
	}
	INIT_LIST_HEAD(&s_auth_cfg.if_list);
#if DEBUG_ENABLE
	AUTH_DEBUG("Free %d net interface totally.\n", free_cnt);
#endif
	return 0;
}


/*LAN-->HEADER, WAN-->TAIL*/
static void add_auth_if_info(struct if_info_node *if_info_node)
{
	if (if_info_node->if_info.type == LAN_E) {
		list_add(&if_info_node->if_node, &s_auth_cfg.if_list);
	}
	else {
		list_add_tail(&if_info_node->if_node, &s_auth_cfg.if_list);
	}
}


static int copy_auth_if_info_to_node(struct if_info_node *if_info_node, struct auth_if_info *if_info)
{
	INIT_LIST_HEAD(&if_info_node->if_node);
	if_info_node->if_info.type = if_info->type;
	memcpy(if_info_node->if_info.if_name, if_info->if_name, IF_NAME_MAX - 1);
	return 0;
}

/*
*Firstly, allocating memory for new rules.
*Then, cleaning up old rules and freeing its memory.
*Lastly, adding new rules to rule_list.
*Notice: smp is also need considered.
*/
int update_auth_rules(struct auth_ip_rule *ip_rules, uint32_t n_rule)
{
	int i = 0, no_mem = 0;
	struct auth_ip_rule_node **ip_rule_nodes = NULL;

	auth_cfg_disable();
	if (n_rule == 0) {
		/*no rule, so, clear old rules*/
		clean_auth_rules();
		goto OUT;
	}
	/*allocating n rule_node*/
	ip_rule_nodes = AUTH_NEW_N(struct auth_ip_rule_node *, n_rule);
	if (ip_rule_nodes == NULL) {
		AUTH_ERROR("No memory.");
		no_mem = 1;
		goto OUT;
	}
	memset(ip_rule_nodes, 0, n_rule * sizeof(struct auth_ip_rule_node*));
	for (i = 0; i < n_rule; i++) {
		ip_rule_nodes[i] = AUTH_NEW(struct auth_ip_rule_node);
		if (ip_rule_nodes[i] == NULL) {
			AUTH_ERROR("No memory.");
			no_mem = 1;
			goto OUT;
		}
		INIT_LIST_HEAD(&ip_rule_nodes[i]->rule_node);
	}

	/*free old rule_list*/
	clean_auth_rules();

	/*insert new rule*/
	for (i = 0; i < n_rule; i++) {
		copy_auth_ip_rule_to_node(ip_rule_nodes[i], &ip_rules[i]);
		add_auth_rule(ip_rule_nodes[i]);
	}

#if DEBUG_ENABLE
	display_auth_ip_rules();
#endif

OUT:
	if (no_mem) {
		if (ip_rule_nodes) {
			for (i = 0; i < n_rule; i++) {
				if (ip_rule_nodes[i]) {
					kfree(ip_rule_nodes[i]);
					ip_rule_nodes[i] = NULL;
				}
			}
			kfree(ip_rule_nodes);
			ip_rule_nodes = NULL;
		}
	}
	auth_cfg_enable();
	if (no_mem) {
		return -1;
	}
	return 0;
}


int update_auth_options(struct auth_options *options)
{
	int ret = 0;	
	auth_cfg_disable();
	ret = set_auth_options(options);
	auth_cfg_enable();
	return ret;
}


int update_auth_if_info(struct auth_if_info* if_info, uint16_t n_if)
{
	int i = 0, no_mem = 0;
	struct if_info_node **if_info_nodes = NULL;

	auth_cfg_disable();
	if (n_if == 0) {
		clean_auth_if_infos();
		goto OUT;
	}
	/*allocating n if_info node*/
	if_info_nodes = AUTH_NEW_N(struct if_info_node *, n_if);
	if (if_info_nodes == NULL) {
		AUTH_ERROR("No memory.");
		no_mem = 1;
		goto OUT;
	}
	memset(if_info_nodes, 0, n_if * sizeof(struct if_info_node*));
	for (i = 0; i < n_if; i++) {
		if_info_nodes[i] = AUTH_NEW(struct if_info_node);
		if (if_info_nodes[i] == NULL) {
			AUTH_ERROR("No memory.");
			no_mem = 1;
			goto OUT;
		}
		INIT_LIST_HEAD(&if_info_nodes[i]->if_node);
	}

	/*free old rule_list*/
	clean_auth_if_infos();

	/*insert new rule*/
	for (i = 0; i < n_if; i++) {
		copy_auth_if_info_to_node(if_info_nodes[i], &if_info[i]);
		add_auth_if_info(if_info_nodes[i]);
	}

#if DEBUG_ENABLE
	display_auth_if_infos();
#endif

OUT:
	if (no_mem) {
		if (if_info_nodes) {
			for (i = 0; i < n_if; i++) {
				if (if_info_nodes[i]) {
					kfree(if_info_nodes[i]);
					if_info_nodes[i] = NULL;
				}
			}
			kfree(if_info_nodes);
			if_info_nodes = NULL;
		}
	}
	auth_cfg_enable();
	if (no_mem) {
		return -1;
	}
	return 0;

}


/**********************************auth_realted**********************************/
/*0-->unmatch, 1-->match*/
int flow_dir_check(const char *inname, const char *outname) 
{
	int in_match = 0, out_match = 0;
	struct list_head *cur = NULL;
	struct if_info_node *cur_node = NULL;
	struct auth_if_info *if_info = NULL;
	
	if (inname == NULL || outname == NULL) {
		return FLOW_NONEED_CHECK;
	}
	
	if (list_empty(&s_auth_cfg.if_list)) {
		return FLOW_NONEED_CHECK;
	}
	/*LAN-->LAN-->...>WAN-->WAN*/
	list_for_each(cur, &s_auth_cfg.if_list) {
		cur_node = list_entry(cur, struct if_info_node, if_node);
		if_info = &cur_node->if_info;
		if (if_info->type != LAN_E) {
			break;
		}
		if (strstr(if_info->if_name, inname)) {
			in_match = 1;
			break;
		}
	}
	if (in_match == 0) {
		return FLOW_NONEED_CHECK;
	}
	list_for_each_prev(cur, &s_auth_cfg.if_list) {
		cur_node = list_entry(cur, struct if_info_node, if_node);
		if_info = &cur_node->if_info;
		if (if_info->type != WAN_E) {
			break;
		}
		if (strstr(if_info->if_name, outname)) {
			out_match = 1;
			break;
		}
	}
	if (out_match == 0) {
		return FLOW_NONEED_CHECK;
	}
	return FLOW_NEED_CHECK;
}


/*First step,traversing auth rules until across a match rule or run over all rule.
 *Second step, i.e, last step, return the process code.*/
int auth_rule_check(uint32_t ipv4)
{
	int auth_res = AUTH_RULE_PASS;	/*default process is pass*/
	struct list_head *cur = NULL;
	struct auth_ip_rule_node *cur_node = NULL;
	struct auth_ip_rule *ip_rule = NULL;

	if (list_empty(&s_auth_cfg.rule_list)) {
		return auth_res;
	}
	list_for_each(cur, &s_auth_cfg.rule_list) {
		cur_node = list_entry(cur, struct auth_ip_rule_node, rule_node);
		if (cur_node->ip_rule.enable == 0) {
			continue;
		}
		ip_rule = &cur_node->ip_rule;
		/*ip in range*/
		if (ipv4 <= ip_rule->min || ipv4 >= ip_rule->max) {
			continue;
		}
		switch (ip_rule->type) {
			case NORMAL:
				auth_res = AUTH_RULE_REDIRECT;
				break;
			case WHITE:
				auth_res = AUTH_RULE_PASS;
				break;
			case BLACK:
				auth_res = AUTH_RULE_REJECT;
				break;
			default:
				auth_res = AUTH_RULE_PASS;
				break;
		}
		break;
	}
	AUTH_DEBUG("HOST:%pI4h, AUTH_RES:%d.\n", &ipv4, auth_res);
	return auth_res;
}


int get_auth_cfg_status(void)
{
	return s_auth_cfg.status;
}


void auth_cfg_enable(void)
{
	s_auth_cfg.status = AUTH_CONF_AVAILABLE;
}


void auth_cfg_disable(void)
{
	s_auth_cfg.status = AUTH_CONF_UNAVAILABLE;
}


int auth_rule_init()
{
	memset(&s_auth_cfg, 0, sizeof(struct auth_rule_config));
	INIT_LIST_HEAD(&s_auth_cfg.rule_list);
	INIT_LIST_HEAD(&s_auth_cfg.if_list);
	s_auth_cfg.status = AUTH_CONF_AVAILABLE;
	return 0;
}


void auth_rule_fini(void)
{
	auth_cfg_disable();
	clean_auth_rules();
	clean_auth_if_infos();
}