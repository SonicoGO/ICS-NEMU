#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE) // number of page tables to cover the vmem
#define LAST_INDEX_PG ((VMEM_ADDR + SCR_SIZE + PAGE_SIZE - 1) / PAGE_SIZE + (NR_PT - 1) * NR_PTE)

PDE *get_updir();
PTE mm_ptable[NR_PTE * NR_PT] align_to_page;

void create_video_mapping()
{
	/* TODO: create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memeory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
    PDE *updir = (PDE *)va_to_pa(get_updir());
    
    // 映射页目录项
    for (int i = 0; i < NR_PT; i++) {
        PTE *vmem_ptable = (PTE *)va_to_pa(&mm_ptable[NR_PTE * i]);
        updir[VMEM_ADDR / PT_SIZE + i].val = make_pde(vmem_ptable);   
    }
    
    // 映射页表项
    for (int i = VMEM_ADDR / PAGE_SIZE; i < LAST_INDEX_PG; i++) {
        mm_ptable[i].val = make_pte(i << 12);
    }

}

void video_mapping_write_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		buf[i] = i;
	}
}

void video_mapping_read_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		assert(buf[i] == i);
	}
}

void video_mapping_clear()
{
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
