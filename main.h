#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdio.h>

#define MAX_TASKS                   5
#define SIZE_TASK_STACK             1024U
#define SIZE_SCHED_STACK            1024U

#define SRAM_START                  0x20000000U
#define SIZE_SRAM                   ((128) * (1024))
#define SRAM_END                    ((SRAM_START) + (SIZE_SRAM))

#define T1_STACK_START              SRAM_END
#define T2_STACK_START              ((SRAM_END) - (1 * SIZE_TASK_STACK))
#define T3_STACK_START              ((SRAM_END) - (2 * SIZE_TASK_STACK))
#define T4_STACK_START              ((SRAM_END) - (3 * SIZE_TASK_STACK))
#define IDLE_STACK_START            ((SRAM_END) - (4 * SIZE_TASK_STACK))
#define SCHED_STACK_START           ((SRAM_END) - (5 * SIZE_SCHED_STACK))

#define TICK_HZ                     1000U
#define HSI_CLOCK         		    16000000U
#define SYSTICK_TIM_CLK   		    HSI_CLOCK

#define DUMMY_XPSR                  0x01000000U

#define TASK_READY_STATE            0x00
#define TASK_BLOCKED_STATE          0XFF

#define INTERRUPT_DISABLE()         do{__asm volatile ("MOV R0,#0x1"); asm volatile("MSR PRIMASK,R0"); } while(0)
#define INTERRUPT_ENABLE()          do{__asm volatile ("MOV R0,#0x0"); asm volatile("MSR PRIMASK,R0"); } while(0)

/*
// Task control block: information about each task
*/
typedef struct
{
	uint32_t psp_addr;
	uint32_t block_count;
	uint8_t  current_state;
	void (*task_handler)(void);
}TCB_t;

// ========= Tasks ========= //

/*
// Task 1: The green LED should be toggling every 1s.
*/
void task1_handler(void);

/*
// Task 2: The orange LED should be toggling every 500ms.
*/
void task2_handler(void);

/*
// Task 3: The blue LED should be toggling every 250 ms.
*/
void task3_handler(void);

/*
// Task 4: The red LED should be toggling every 125 ms.
*/
void task4_handler(void);

/*
// Idle task: hangs
*/
void idle_task(void);

// ========= Init Functions ========= //

/*
// Initiates the Systick Timer.
// 1. Sets STCSR: enables SysTick timer
// 2. Sets STCVR register to the desired reload value.
// Argument:
// - tick_hz: desired exception frequency
*/
void init_systick_timer(uint32_t tick_hz);

/*
// Initiates the stack for the scheduler > stores the top of the scheduler stack in MSP
// Argument:
// - top_of_sched_stack: address for the top of the scheduler's stack
*/
__attribute__((naked)) void init_scheduler_stack(uint32_t top_of_sched_stack);

/*
// Initiates the stack for the tasks. Fills in with dummy values.
// Goes to the start of each task's stack, filling in XPSR, PC, LR, and registers.
*/
void init_tasks_stack(void);

/*
// Enables processor faults by modifying the SHCSR register (usage, memory management, & bus faults)
*/
void enable_processor_faults(void);

/*
// Switches the stack pointer to use PSP for thread mode (user tasks), instead of MSP
// Must be naked function to access CONTROL register
*/
__attribute__((naked)) void switch_sp_to_psp(void);

// ========= Main Functions ========= //

/*
// Gets the current value of PSP (the address of the current task running)
// Returns:
// - uint32_t: the current value of PSP
*/
uint32_t get_psp_value(void);

/*
// Saves the current value of PSP (an address)
// Arguments:
// - current_psp_addr: the current value of PSP (this will be saved in R0 register via function input)
*/
void save_psp_addr(uint32_t current_psp_addr);

/*
// Moves onto the next task: choose the next task that is in a READY state
// If all are BLOCKED, then run the idle task
*/
void update_next_task(void);

/*
// Scheduler task: pends the PendSV exception for context switch
*/
void schedule(void);

/*
// Updates the global tick count by one, whenever the SysTick exception occurs
*/
void update_global_tick_count(void);

/*
// Updates the current state of tasks. 
// Checks if they are in a blocked state and if the block count has been complete > they can be changed to READY.
*/
void unblock_tasks(void);

/*
// Delays the current task by the desired tick count by changing its current state to BLOCKED.
// Arguments:
// - tick_count: desired delay for the task, measured in number of system ticks (1 tick = 1ms)
*/
void task_delay(uint32_t tick_count);

/*
// Function defined in syscalls.c
// For initialization of semi-hosting for printing out statements.
*/
extern void initialise_monitor_handles(void);

#endif
