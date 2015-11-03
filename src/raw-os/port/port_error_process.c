#include <raw_api.h>


/*system error process function, you can do your own logic here, the following is just a example*/
void port_system_error_process(RAW_OS_ERROR error_type, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{
	switch (error_type) {

		case RAW_MUTEX_RECURSION_LEVELS_EXCEEDED:
			RAW_ASSERT(0);
			break;
			
		case RAW_SYSTEM_CRITICAL_ERROR:
			RAW_ASSERT(0);
			break;
			
		case RAW_MUTEX_NESTED_OVERFLOW:
			RAW_ASSERT(0);
			break;
			
		case RAW_EXCEED_INT_NESTED_LEVEL:
			RAW_ASSERT(0);
			break;
			
		default:
			RAW_ASSERT(0);
			break;
	}
	

}


