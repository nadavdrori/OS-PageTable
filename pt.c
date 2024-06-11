#include "os.h"
#include <stdlib.h>
#include <stdio.h>

int *PT_Levels(uint64_t vpn)
{
    int *levels = malloc(5 * sizeof(int));
    int length = 5;
    int j = 0;

    // Create the levels bit mask
    uint64_t mask = 0x1FF; // Hexadecimal representation of 9 bits set to 1
    for (int i = 0; i <= 36; i += 9, j++)
    {
        // Apply the mask and shift right
        uint64_t extracted = (vpn >> i) & mask;

        levels[length - j - 1] = (int)extracted;
    }
    return levels;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    int *levels = PT_Levels(vpn);
    int length = 5;
    uint64_t *pt_run = phys_to_virt(pt << 12);

    uint64_t mask = 1ULL;
    uint64_t invalid_mask = ~(1ULL);

    uint64_t next_pt = 0;
    int valid = 0;

    if (ppn == NO_MAPPING)
    {
        int i = 0;
        while (i < length - 1)
        {
            // Apply the mask to check validation of pte
            uint64_t extracted = pt_run[levels[i]] & mask;
            valid = (int)extracted;

            if (valid == 0)
            {
                break;
            }
            else
            {
                next_pt = (pt_run[levels[i]] >> 12) << 12;
                pt_run = phys_to_virt(next_pt);
            }
            i += 1;
        }
        if (i == length - 1)
        {
            pt_run[levels[length - 1]] = pt_run[levels[length - 1]] & invalid_mask;
        }
    }
    else
    {
        for (int i = 0; i < length - 1; i++)
        {
            // Apply the mask to check validation of pte
            uint64_t extracted = pt_run[levels[i]] & mask;
            valid = (int)extracted;
            if (valid == 1)
            {
                next_pt = (pt_run[levels[i]] >> 12) << 12;
            }
            else
            {
                uint64_t new_pt = (alloc_page_frame() << 12);
                pt_run[levels[i]] = (new_pt | mask);
                next_pt = new_pt;
            }
            pt_run = phys_to_virt(next_pt);
        }
        pt_run[levels[length - 1]] = (ppn << 12) | mask;
    }
    free(levels);
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    int *levels = PT_Levels(vpn);
    int length = 5;
    uint64_t *pt_run = phys_to_virt(pt << 12);

    // Create the valid bit mask
    uint64_t mask = 1ULL;
    uint64_t next_pt = 0;
    int valid = 0;

    for (int i = 0; i < length-1; i++)
    {
        // Apply the mask to check if the pte is valid
        uint64_t extracted = pt_run[levels[i]] & mask;
        valid = (int)extracted;

        if (valid == 0)
        {
            return NO_MAPPING;
        }
        else
        {
            next_pt = (pt_run[levels[i]] >> 12) << 12;
        }
        pt_run = phys_to_virt(next_pt);
    }
    // last level validation check
    uint64_t extracted = pt_run[levels[length-1]] & mask;
    valid = (int)extracted;
    if (valid == 0){
        return NO_MAPPING;
    }
    uint64_t ppn = pt_run[levels[length - 1]] >> 12;
    free(levels);
    return ppn;
}