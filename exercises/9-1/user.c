#include "os.h"

#define DELAY 1000

void user_task0(void* param)
{
	uart_puts("Task 0: Created!\n");
	uart_puts((char*)param);
	while (1) {
		uart_puts("Task 0: Running...\n");
		task_delay(DELAY);
		task_yield();
	}
}

void user_task1(void* param)
{
	uart_puts("Task 1: Created!\n");
	uart_puts((char*)param);
	while (1) {
		uart_puts("Task 1: Running...\n");
		task_delay(DELAY);
		task_yield();
	}
}

void user_task2(void* param)
{
	uart_puts("Task 2: Created!\n");
	uart_puts((char*)param);
	while (1) {
		uart_puts("Task 2: Running...\n");
		task_delay(DELAY);
		task_yield();
	}
}

/* NOTICE: DON'T LOOP INFINITELY IN main() */
void os_main(void)
{
	task_create(user_task0, "task0_param");
	task_create(user_task1, "task1_param");
	task_create(user_task2, "task2_param");
}

