#include "os.h"
#include <stdlib.h>

int *PT_Levels(uint16_t vpn)
{
    int *levels = malloc(5 * sizeof(int));
    for (int i = 0; i <= 36;)
    {
        // Create the levels bit mask
        uint64_t mask = (1ULL << 9) - 1;
        // Apply the mask and shift right
        uint64_t extracted = (vpn >> i) & mask;

        levels[len(levels) - i] = (int)extracted;
        i += 9;
    }
    return levels;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    int *levels = PT_Levels(vpn);
    uint64_t *pt_run = phys_to_virt(pt);

    uint64_t mask = (1ULL << 1) - 1;
    uint64_t invalid_mask = ~(1ULL);

    uint64_t *next_pt = NULL;
    int valid = 0;

    if (ppn = NO_MAPPING)
    {
        int i = 0;
        while (i < len(levels) - 1)
        {
            // Apply the mask
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
            i++;
        }
        if (i == len(levels) - 1)
        {
            pt_run[levels[len(levels) - 1]] = pt_run[levels[len(levels) - 1]] & invalid_mask;
        }
    }
    else
    {
        int creat_new = 0;
        for (int i = 0; i < len(levels) - 1; i++)
        {
            // Apply the mask
            uint64_t extracted = pt_run[levels[i]] & mask;
            valid = (int)extracted;
            if (valid == 1 && creat_new == 0)
            {
                next_pt = (pt_run[levels[i]] >> 12) << 12;
            }
            else
            {
                creat_new = 1;
                uint64_t new_pt = (alloc_page_frame() << 12);
                pt_run[levels[i]] = (new_pt | mask);
                next_pt = (pt_run[levels[i]] >> 12) << 12;
            }
            pt_run = phys_to_virt(next_pt);
        }
        pt_run[levels[len(levels) - 1]] = (ppn << 12) | mask;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{

    int *levels = PT_Levels(vpn);
    uint64_t *pt_run = phys_to_virt(pt);

    // Create the valid bit mask
    uint64_t mask = (1ULL << 1) - 1;
    uint64_t *next_pt = NULL;
    int valid = 0;

    for (int i = 0; i < len(levels) - 1; i++)
    {
        // Apply the mask
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
    return (pt_run[levels[len(levels) - 1]] >> 12);
}