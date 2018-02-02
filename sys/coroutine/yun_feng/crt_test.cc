#include <stdio.h>
#include <ucontext.h> // 协程相关api所在头文件
#include <list>

using namespace std;

// 定义3个协程，类似于3个线程
static void start();
static void func2();
static void func1();

static ucontext_t ctx1; // 协程func1的上下文，由makecontext调用构造
static ucontext_t ctx2; // 协程func2的上下文，由makecontext调用构造
static ucontext_t ctx3; // 协程start的上下文，由swapcontext自动构造

// stack为new/malloc出来的也可以的
static char stack1[4096]; // 协程func1的栈，得合适大小，否则一样会出现栈溢出
static char stack2[8192]; // 协程func2的栈，得合适大小，否则一样会出现栈溢出

int main()
{
	printf("main 1\n");

	// 构造协程func1的上下文
	getcontext(&ctx1);
	ctx1.uc_stack.ss_sp = stack1;
	ctx1.uc_stack.ss_size = sizeof(stack1);
	ctx1.uc_link = &ctx3; // 在ctx1之后的上下文
	makecontext(&ctx1, func1, 0); // 在运行完func1之后，运行ctx3

	// 构造协程func2的上下文
	getcontext(&ctx2);
	ctx2.uc_stack.ss_sp = stack2;
	ctx2.uc_stack.ss_size = sizeof(stack2);
	ctx2.uc_link = &ctx1;
	makecontext(&ctx2, func2, 0); // 在运行完func1之后，运行ctx1

	start();
	printf("main 2\n");

	return 0;
}

// 可把start当成一个线程，不过它是微线程
void start()
{
	printf("%s 1\n", __func__);
	// 切换到ctx2执行，也就是执行func2，当前的保存在ctx3
	swapcontext(&ctx3, &ctx2);
	// 当切回到ctx3时，会执行以下代码段
	printf("%s 2\n", __func__);
}

// 也可把func2当成一个线程，不过它是微线程
void func2()
{
	printf("%s\n", __func__);
}

// 也可把func1当成一个线程，不过它是微线程
void func1()
{
	printf("%s\n", __func__);
}

