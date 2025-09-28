/**
******************************************************************************
// Executes a task scheduler.
// In this case, the tasks are toggling LEDs on the board.
******************************************************************************
*/

#include "main.h"
#include "led.h"

/* Global Variables */

// Keeps track of the current task being executed on the CPU
uint8_t current_task = 1; // Start with Task 1

// Global tick count
uint32_t g_tick_count = 0;

// Array keeping track of user tasks
TCB_t user_tasks[MAX_TASKS];

int main(void)
{
	printf("This is a task scheduler demo.\n");
	initialise_monitor_handles(); // semi hosting init function
	enable_processor_faults();
	init_scheduler_stack(SCHED_STACK_START);
	init_tasks_stack(); // store dummy values for tasks
	led_init_all();
	init_systick_timer(TICK_HZ);
	switch_sp_to_psp(); // we start thread mode with MSP; need to switch to PSP for user tasks
	task1_handler();
	while(1);
}

void idle_task(void)
{
	while(1);
}

void task1_handler(void)
{
	while(1)
	{
		printf("This is Task 1\n");
		led_on(LED_GREEN);
		task_delay(DELAY_COUNT_1S);
		led_off(LED_GREEN);
		task_delay(DELAY_COUNT_1S);
	}
	
}

void task2_handler(void)
{
	while(1)
	{
		led_on(LED_ORANGE);
		task_delay(DELAY_COUNT_500MS);
		led_off(LED_ORANGE);
		task_delay(DELAY_COUNT_500MS);
	}
	
}

void task3_handler(void)
{
	while(1)
	{
		led_on(LED_BLUE);
		task_delay(DELAY_COUNT_250MS);
		led_off(LED_BLUE);
		task_delay(DELAY_COUNT_250MS);
	}
	
}

void task4_handler(void)
{
	while(1)
	{
		led_on(LED_RED);
		task_delay(DELAY_COUNT_125MS);
		led_off(LED_RED);
		task_delay(DELAY_COUNT_125MS);
	}
}

void init_systick_timer(uint32_t tick_hz)
{
	// ptrs for registers
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;
	
	// calculate the reload value: counts down for each clock cycle
	uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1;
	
	// set up reload value
	*pSRVR &= ~(0x00FFFFFFFF);
	*pSRVR |= count_value;
	
	// enable the systick timer
	*pSCSR |= ( 1 << 1); // TICKINT
	*pSCSR |= ( 1 << 2);  // CLKSOURCE
	*pSCSR |= ( 1 << 0); // ENABLE
}

__attribute__((naked)) void init_scheduler_stack(uint32_t top_of_sched_stack)
{
	// set MSP = top of scheduler stack
	__asm volatile("MSR MSP,%0": :  "r" (top_of_sched_stack)  :   );
	// branching back to main (naked funcs don't have prologue/epilogue)
	__asm volatile("BX LR");
}

void init_tasks_stack(void)
{
	user_tasks[0].current_state = TASK_READY_STATE;
	user_tasks[1].current_state = TASK_READY_STATE;
	user_tasks[2].current_state = TASK_READY_STATE;
	user_tasks[3].current_state = TASK_READY_STATE;
	user_tasks[4].current_state = TASK_READY_STATE;
	
	user_tasks[0].psp_addr = IDLE_STACK_START;
	user_tasks[1].psp_addr = T1_STACK_START;
	user_tasks[2].psp_addr = T2_STACK_START;
	user_tasks[3].psp_addr = T3_STACK_START;
	user_tasks[4].psp_addr = T4_STACK_START;
	
	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;
	
	uint32_t *pPSP;
	for (int i = 0; i < MAX_TASKS; i++)
	{
		pPSP = (uint32_t*) user_tasks[i].psp_addr;
		
		pPSP--;
		*pPSP = DUMMY_XPSR; // account for T bit
		
		pPSP--; // PC of the task = the task handler's addr
		*pPSP = (uint32_t) user_tasks[i].task_handler;
		
		
		pPSP--; // LR: return to thread mode & use PSP
		*pPSP = 0xFFFFFFFD;
		
		for (int j = 0; j < 13; j++)
		{
			// fill up other registers with 0
			pPSP--;
			*pPSP = 0;
		}
		
		// ensure that the PSP is accurate
		user_tasks[i].psp_addr = (uint32_t)pPSP;
	}
}

void enable_processor_faults(void)
{
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;
	*pSHCSR |= ( 1 << 16); // mem manage
	*pSHCSR |= ( 1 << 17); // bus fault
	*pSHCSR |= ( 1 << 18); // usage fault
}

uint32_t get_psp_addr(void)
{
	return user_tasks[current_task].psp_addr;
}


void save_psp_addr(uint32_t current_psp_addr)
{
	user_tasks[current_task].psp_addr = current_psp_addr;
}


void update_next_task(void)
{
	int state = TASK_BLOCKED_STATE;
	
	for (int i= 0; i < (MAX_TASKS); i++)
	{
		current_task++;
		current_task %= MAX_TASKS;
		state = user_tasks[current_task].current_state;
		if((state == TASK_READY_STATE) && (current_task != 0))
		// if there's a user task ready to be scheduled, schedule it
		break;
	}
	
	// all tasks are in blocked state: schedule the idle task
	if (state != TASK_READY_STATE)
	current_task = 0;
}

__attribute__((naked)) void switch_sp_to_psp(void)
{
	// 1. initialize PSP with task 1's stack start addr
	__asm volatile ("PUSH {LR}"); // preserve LR back to main()
	__asm volatile ("BL get_psp_addr"); // fetch the current task's stack addr, saved in R0
	__asm volatile ("MSR PSP,R0"); // initialize PSP
	__asm volatile ("POP {LR}");  // pop back LR
	
	// 2. change SP to PSP using control register
	__asm volatile ("MOV R0,#0X02");
	__asm volatile ("MSR CONTROL,R0");
	__asm volatile ("BX LR");
}

void schedule(void)
{
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= ( 1 << 28);
}

void task_delay(uint32_t tick_count)
{
	// disable interrupt before accessing global variable in case of race condition between handler & thread mode
	INTERRUPT_DISABLE();
	
	// only block user tasks, not the idle task; idle task is always ready
	if (current_task)
	{
		user_tasks[current_task].block_count = g_tick_count + tick_count;
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
		schedule(); // allow another task to run as the current task is delayed
	}
	
	INTERRUPT_ENABLE();
}

void update_global_tick_count(void)
{
	g_tick_count++;
}

void unblock_tasks(void)
{
	for (int i = 1; i < MAX_TASKS; i++)
	{
		if (user_tasks[i].current_state != TASK_READY_STATE)
		{
			if (user_tasks[i].block_count == g_tick_count)
			{
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}

/*
// SysTick Exception Handler
// 1. Updates the global tick count, because each time exception occurs = another tick
// 2. Unblocks tasks that are done with their delays - set their current state to READY
// 3. Schedules another context switch by pending PendSV handler
*/
void  SysTick_Handler(void)
{
	update_global_tick_count();
	unblock_tasks();
	schedule();	
}

/*
// PendSV Exception Handler
// 1. Saves context of the current task
// 2. Retrieves the next task
*/
__attribute__((naked)) void PendSV_Handler(void)
{
	
	// save
	__asm volatile("MRS R0,PSP"); // save current task's PSP
	__asm volatile("STMDB R0!,{R4-R11}"); // save registers
	__asm volatile("PUSH {LR}");
	__asm volatile("BL save_psp_addr"); // r0 = PSP & will pass to this function
	
	// decide next user task to run
	__asm volatile("BL update_next_task");
	
	
	// retrieve
	__asm volatile ("BL get_psp_addr"); // get next task's PSP
	__asm volatile ("LDMIA R0!,{R4-R11}"); // load in registers
	__asm volatile("MSR PSP,R0"); // load in next task's PSP
	__asm volatile("POP {LR}");
	__asm volatile("BX LR"); // no epilogue, we need to exit ourselves
}

void HardFault_Handler(void)
{
	uint32_t *pHFSR = (uint32_t*)0xE000ED2C;

	printf("Exception: Hardfault\n");\
	printf("FORCED: %ld\n", (*pHFSR >> 30 & 0x1));
	printf("VECTTBL: %ld\n", (*pHFSR >> 1 & 0x1));
	
	while(1);
}


void MemManage_Handler(void)
{
	uint32_t *pMMSR = (uint32_t*)0xE000ED28;

	printf("Exception: MemManage\n");
	printf("MMARVALID: %ld\n", (*pMMSR >> 7 & 0x1));
	printf("MLSPERR: %ld\n", (*pMMSR >> 5 & 0x1));
	printf("MSTKERR: %ld\n", (*pMMSR >> 4 & 0x1));
	printf("MUNSTKERR: %ld\n", (*pMMSR >> 3 & 0x1));
	printf("DACCVIOL: %ld\n", (*pMMSR >> 1 & 0x1));
	printf("IACCVIOL: %ld\n", (*pMMSR >> 0 & 0x1));

	while(1);
}

void BusFault_Handler(void)
{
	uint32_t *pBFSR = (uint32_t*)0xE000ED29;

	printf("Exception: BusFault\n");
	printf("BFARVALID: %ld\n", (*pBFSR >> 7 & 0x1));
	printf("LSPERR: %ld\n", (*pBFSR >> 5 & 0x1));
	printf("STKERR: %ld\n", (*pBFSR >> 4 & 0x1));
	printf("UNSTKERR: %ld\n", (*pBFSR >> 3 & 0x1));
	printf("IMPRECISERR: %ld\n", (*pBFSR >> 2 & 0x1));
	printf("PRECISERR: %ld\n", (*pBFSR >> 1 & 0x1));
	printf("IBUSERR: %ld\n", (*pBFSR >> 0 & 0x1));
	
	while(1);
}

void UsageFault_Handler(void)
{
	uint32_t *pUFSR = (uint32_t*)0xE000ED2A;

	printf("Exception: UsageFault\n");
	printf("DIVBYZERO: %ld\n", (*pUFSR >> 9 & 0x1));
	printf("UNALIGNED: %ld\n", (*pUFSR >> 8 & 0x1));
	printf("NOCP: %ld\n", (*pUFSR >> 3 & 0x1));
	printf("INVPC: %ld\n", (*pUFSR >> 2 & 0x1));
	printf("INVSTATE: %ld\n", (*pUFSR >> 1 & 0x1));
	printf("UNDEFINSTR: %ld\n", (*pUFSR >> 0 & 0x1));
	
	while(1);
}
