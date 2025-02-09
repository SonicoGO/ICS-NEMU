#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B]; //128 * 1024 * 1024 = 128MB

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
#ifdef HAS_DEVICE_VGA
    int map_NO = is_mmio(paddr);
    if (map_NO != -1) {
        ret = mmio_read(paddr, len, map_NO);
        return ret;
    }
#endif
	
#ifdef CACHE_ENABLED
	ret = cache_read(paddr, len);
#else
    ret = hw_mem_read(paddr, len);
#endif
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
#ifdef HAS_DEVICE_VGA
    int map_NO = is_mmio(paddr);
    if (map_NO != -1) {
        mmio_write(paddr, len, data, map_NO);
        return;
    }
#endif
    
#ifdef CACHE_ENABLED
    cache_write(paddr, len, data);
#else
	hw_mem_write(paddr, len, data);
#endif
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
	paddr_t paddr = laddr;
	if (cpu.cr0.pe && cpu.cr0.pg) {	
		if ((laddr & 0xFFF) + len > 0x1000) {
			uint32_t data, part1, part2;
			part1 = 0x1000 - (laddr & 0xFFF);
			part2 = (laddr & 0xFFF) + len - 0x1000;
			
			paddr = page_translate(laddr);
			data = paddr_read(paddr, part1);
			paddr = page_translate(laddr + 0x1000 - (laddr & 0xFFF));
			data += paddr_read(paddr, part2) << (part1 << 3);
			return data;
		}
		paddr = page_translate(laddr);
	}
	return paddr_read(paddr, len);
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
	paddr_t paddr = laddr;
	if (cpu.cr0.pe && cpu.cr0.pg) {
		if ((laddr & 0xFFF) + len > 0x1000) {
			uint32_t part1, part2;
			part1 = 0x1000 - (laddr & 0xFFF);
			part2 = (laddr & 0xFFF) + len - 0x1000;
			
			paddr = page_translate(laddr);
			paddr_write(paddr, part1, data & ((1 << ((part1 << 3) + 1)) - 1));
			paddr = page_translate(laddr + 0x1000 - (laddr & 0xFFF));
			paddr_write(paddr, part2, data >> (part1 << 3));
			return;
		}
		paddr = page_translate(laddr);
	}
	paddr_write(paddr, len, data);
}

// 顶层读接口
uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	if (cpu.cr0.pe == 1)
	    vaddr = segment_translate(vaddr, sreg);
	return laddr_read(vaddr, len);
}

// 顶层写接口
void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
	if (cpu.cr0.pe == 1)
	    vaddr = segment_translate(vaddr, sreg);
	laddr_write(vaddr, len, data);
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);

#ifdef CACHE_ENABLED
    init_cache();
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
