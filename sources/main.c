#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include "plgldr.h"
#include "csvc.h"
#include "common.h"
#include "title_info.h"

void install_hooks(u64 title_id);

static Handle thread;
static Handle memLayoutChanged;
static u8 stack[STACK_SIZE] ALIGN(8);

extern char *fake_heap_start;
extern char *fake_heap_end;
extern u32 __ctru_heap;
extern u32 __ctru_linear_heap;

u32 __ctru_heap_size = 0;
u32 __ctru_linear_heap_size = 0;

void __system_allocateHeaps(PluginHeader *header)
{
    __ctru_heap_size = header->heapSize;
    __ctru_heap = header->heapVA;

    // Set up newlib heap
    fake_heap_start = (char *)__ctru_heap;
    fake_heap_end = fake_heap_start + __ctru_heap_size;
}

// Entrypoint, game will start when you exit this function
void main(void)
{
    PluginHeader *header = (PluginHeader *)0x07000000;

    // Init heap
    __system_allocateHeaps(header);

    // Init services
    srvInit();
    plgLdrInit();

    // Get memory layout changed event
    svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_GET_ON_MEMORY_CHANGE_EVENT, (u32)&memLayoutChanged, 0);

    // Install hooks
    install_hooks(get_title_id());
}
