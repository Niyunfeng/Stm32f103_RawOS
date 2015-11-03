#include <raw_api.h>



/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/

#define  CPU_REG_DEM_CR                       (*(volatile RAW_U32 *)0xE000EDFC)
#define  CPU_REG_DWT_CR                       (*(volatile RAW_U32 *)0xE0001000)
#define  CPU_REG_DWT_CYCCNT                   (*(volatile RAW_U32 *)0xE0001004)
#define  CPU_REG_DBGMCU_CR                    (*(volatile RAW_U32 *)0xE0042004)

/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/

#define  DEF_BIT_00                                     0x01u
#define  DEF_BIT_24                                     0x01000000u


#define  CPU_DBGMCU_CR_TRACE_IOEN_MASK                   0x10
#define  CPU_DBGMCU_CR_TRACE_MODE_ASYNC                  0x00
#define  CPU_DBGMCU_CR_TRACE_MODE_SYNC_01                0x40
#define  CPU_DBGMCU_CR_TRACE_MODE_SYNC_02                0x80
#define  CPU_DBGMCU_CR_TRACE_MODE_SYNC_04                0xC0
#define  CPU_DBGMCU_CR_TRACE_MODE_MASK                   0xC0

#define  CPU_BIT_DEM_CR_TRCENA                           DEF_BIT_24

#define  CPU_BIT_DWT_CR_CYCCNTENA                        DEF_BIT_00



void  cpu_timer_init (void)
{
    CPU_REG_DEM_CR     |= (RAW_U32)CPU_BIT_DEM_CR_TRCENA;    /* Enable Cortex-M4's DWT CYCCNT reg.                   */
    CPU_REG_DWT_CYCCNT  = (RAW_U32)0u;
    CPU_REG_DWT_CR     |= (RAW_U32)CPU_BIT_DWT_CR_CYCCNTENA;
}


PORT_TIMER_TYPE user_cpu_time_get(void)
{

	PORT_TIMER_TYPE  ts_tmr_cnts;

    ts_tmr_cnts = (PORT_TIMER_TYPE)CPU_REG_DWT_CYCCNT;

    return (ts_tmr_cnts);

}


RAW_HARD_TIME_TYPE user_sys_time_get(void)
{
	PORT_TIMER_TYPE  ts_tmr_cnts;

    ts_tmr_cnts = (PORT_TIMER_TYPE)CPU_REG_DWT_CYCCNT;

    return (RAW_HARD_TIME_TYPE)ts_tmr_cnts;
}

