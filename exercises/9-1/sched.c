#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024
uint8_t task_stack[MAX_TASKS][STACK_SIZE];
struct context ctx_tasks[MAX_TASKS];
struct Node list[MAX_TASKS];
struct Node new_list[MAX_TASKS];

/*
 * _top is used to mark the max available position of ctx_tasks
 * _current is used to point to the context of current task
 */
static int _top = 0;
// struct context* _head = NULL;
// struct context* _new = NULL; // 新加入的任务，等待添加
// struct context* _current = NULL;

struct Node* _head = NULL;
struct Node* _new = NULL;
struct Node* _current = NULL;

static void w_mscratch(reg_t x)
{
	asm volatile("csrw mscratch, %0" : : "r" (x));
}

void sched_init()
{
	w_mscratch(0);
}

/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{
	printf("schedule()\n");
	if (_top <= 0) {
		panic("Num of task should be greater than zero!");
		return;
	}
	if (!_current) {
		printf("_current null\n");
		_current = _head;
	}
	struct context* ctx = _current->data;
	if (_new && _new->data->priority < ctx->priority) {
		ctx = _new;
		printf("schedule new ctx: %d\n", ctx->priority);
		// 只需要调度一次，后面就在_head表中正常轮转
		_new->is_used = 0;
		_new = _new->next;
	} else {
		printf("schedule normal ctx: %d\n", ctx->priority);
		_current = _current->next;
		// 新添加的优先级比当前的低，会在后面的调度中被轮询
		if (_new) {
			clear_list(_new);
			_new = NULL;
		}
	}
	switch_to(ctx);
}

struct Node* find_unused_node(struct Node* list) {
	for (int i = 0; i < MAX_TASKS; i += 1) {
		if (!list[i].is_used) {
			struct Node* node = &list[i];
			node->is_used = 1;
			return node;
		}
	}
	panic("no empty node");
}
void clear_list(struct Node* node) {
	do {
		node->is_used = 0;
		node = node->next;
	} while (node);
}

void insert_list(struct context* ctx) {
	struct Node* node = find_unused_node(list);
	node->data = ctx;
	node->next = NULL;

	struct Node* head = _head;
	if (head) {
		if (node->data->priority < head->data->priority) {
			node->next = head;
			_head = node;
		} else {
			while (head->next && node->data->priority >= head->next->data->priority) {
				head = head->next;
			}
			node->next = head->next;
			head->next = node;
		}
	} else {
		_head = node;
	}
}

void insert_new_list(struct context* ctx) {
	struct Node* node = find_unused_node(new_list);
	node->data = ctx;
	node->next = NULL;

	struct Node* head = _new;
	if (head) {
		if (node->data->priority < head->data->priority) {
			node->next = head;
			_new = node;
		} else {
			while (head->next && node->data->priority >= head->next->data->priority) {
				head = head->next;
			}
			node->next = head->next;
			head->next = node;
		}
	} else {
		_new = node;
	}
}

/*
 * DESCRIPTION
 * 	Create a task.
 * 	- start_routin: task routune entry
 * RETURN VALUE
 * 	0: success
 * 	-1: if error occured
 */
int task_create(void (*start_routin)(void*), void* param, uint8_t priority)
{
	if (_top < MAX_TASKS) {
		struct context* ctx = &(ctx_tasks[_top]);
		ctx->sp = (reg_t) &task_stack[_top][STACK_SIZE - 1];
		ctx->ra = (reg_t) start_routin;
		ctx->a0 = param;
		ctx->priority = priority;
		_top++;
		insert_list(ctx);
		insert_new_list(ctx);
		return 0;
	} else {
		return -1;
	}
}

/*
 * DESCRIPTION
 * 	task_yield()  causes the calling task to relinquish the CPU and a new 
 * 	task gets to run.
 */
void task_yield()
{
	schedule();
}

/*
 * a very rough implementaion, just to consume the cpu
 */
void task_delay(volatile int count)
{
	count *= 50000;
	while (count--);
}

