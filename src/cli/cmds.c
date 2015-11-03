#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stm32f10x.h>

#include "cli.h"
#include "command.h"

#include "raw_api.h"

#pragma diag_suppress 870

static int do_help(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *start = ll_entry_start(cmd_tbl_t, cmd);
	const int len = ll_entry_count(cmd_tbl_t, cmd);
	return _do_help(start, len, cmdtp, flag, argc, argv);
}

static int do_reboot(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
	NVIC_SystemReset();
	
	return 0;
}

static int do_stack(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
	LIST *iter, *head, *temp;
	RAW_TASK_OBJ *task_ptr;
	
	RAW_U32 free_stack;
	RAW_U32 inx = 0;
	
	head = &raw_task_debug.task_head;
	iter = head->next;
	
	for(iter = head->next; iter != head; iter = temp, inx++) 
	{
		temp = iter->next;
		task_ptr = raw_list_entry(iter, RAW_TASK_OBJ, task_debug_list);
		
		raw_task_stack_check(task_ptr, &free_stack);
		
		raw_printf("%04d name = %s\r\t\t\t\tstack_size = %4d\tfree_stack = %4d\n", 
					inx, task_ptr->task_name, task_ptr->stack_size, free_stack);
	}
	
	return 0;
}

//static int do_date(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
//{
//	if(1 != argc)
//	{
//		return -1;
//	}
//	
//	char *w[] = {"日", "一", "二", "三", "四", "五", "六"};
//	
//	raw_printf("%4d年 %02d月 %02d日 星期%s %2d:%02d:%02d\n",
//		2014, 10, 1, w[3], 0, 0, 0);
//	
//	return 0;
//}

static int do_version(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
    raw_printf("version: %d \t build_time: %s \n",RAW_OS_VERSION,RAW_OS_BUILD_TIME);
    return 0;
}

static int do_state(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
    RAW_U8 temp_usuage          = cpu_usuage/100;
    RAW_U8 temp_usuage_tmp      = cpu_usuage_max/100;
    raw_printf("cpu_usuage: %d% \t temp_usuage_tmp: %d% \r\n",temp_usuage,temp_usuage_tmp);
    return 0;
}


/******************************************************************************/



/******************************************************************************/
static cmd_tbl_t __cmd_list[] = 
{
	U_BOOT_CMD_MKENT
	(
		help,   CONFIG_SYS_MAXARGS, 0/* 不重复 */,  do_help,
		"print command description/usage",
		"\n"
		"    - print brief description of all commands\n"
		"help command ...\n"
		"    - print detailed usage of 'command'\n"
	),
	U_BOOT_CMD_MKENT
	(
		reboot, 1, 0/* 不重复 */,  do_reboot,//需要注意CPU引脚如果接了强上拉的话会导致复位失败
		"reboot the system",
		"\n"
		"    - reboot the system\n"
	),
	U_BOOT_CMD_MKENT
	(
		stack, 1, 1,  do_stack,
		"print all task stack status",
		"\n"
		"    - print all task stack status\n"
	),
//	U_BOOT_CMD_MKENT
//	(
//		date, 1, 0,  do_date,
//		"print date and time",
//		"\n"
//		"    - print date and time\n"
//	),
	U_BOOT_CMD_MKENT
	(
		version, 1, 0,  do_version,
		"print rawos_version and rawos_build_time",
		"\n"
		"    - print raw_os_version and raw_os_build_time\n"
	), 
	U_BOOT_CMD_MKENT
	(
		state, 1, 1,  do_state,
		"print rawos_cpu_usuage and rawos_cpu_usuage_max",
		"\n"
		"    - print rawos_cpu_usuage and rawos_cpu_usuage_max\n"
	),    
    

};

int __ll_entry_count(void)
{
	return ARRAY_SIZE(__cmd_list);
}
const void *__ll_entry_start(void)
{
	return (void *)__cmd_list;
}
const void *__ll_entry_end(void)
{
	return (struct cmd_tbl_s *)__ll_entry_start + ARRAY_SIZE(__cmd_list);
}
