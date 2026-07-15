#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configurable page size and memory settings
#define PAGE_SIZE     4      // each page holds 4 units
#define TOTAL_FRAMES  4      // physical memory has 4 frames
#define VIRTUAL_PAGES 8      // virtual address space has 8 pages

// Structure representing one physical frame in memory
typedef struct {
    int page_number;   // which virtual page is loaded here (-1 = empty)
    int is_valid;      // 1 if frame contains a valid page, 0 if empty
} Frame;

// Structure representing one entry in the page table
typedef struct {
    int frame_number;  // which physical frame holds this page (-1 = not loaded)
    int is_present;    // 1 if page is in memory, 0 if not (page fault needed)
} PageTableEntry;

// Physical memory - array of frames
Frame physical_memory[TOTAL_FRAMES];

// Page table - one entry per virtual page
PageTableEntry page_table[VIRTUAL_PAGES];

// Statistics counters
int page_faults = 0;
int page_hits   = 0;

// Initialize memory - all frames empty, all pages not present
void initialize_memory() {
    int i;

    // Set all physical frames as empty
    for (i = 0; i < TOTAL_FRAMES; i++) {
        physical_memory[i].page_number = -1;
        physical_memory[i].is_valid    = 0;
    }

    // Set all page table entries as not present
    for (i = 0; i < VIRTUAL_PAGES; i++) {
        page_table[i].frame_number = -1;
        page_table[i].is_present   = 0;
    }

    page_faults = 0;
    page_hits   = 0;
}

// Translate virtual address to physical address
// Returns physical address or -1 if page fault
int translate_address(int virtual_address) {
    // Calculate which page this address belongs to
    int page_number  = virtual_address / PAGE_SIZE;
    // Calculate offset within the page
    int page_offset  = virtual_address % PAGE_SIZE;

    printf("\nVirtual Address: %d -> Page: %d, Offset: %d\n",
           virtual_address, page_number, page_offset);

    // Check if page number is valid
    if (page_number >= VIRTUAL_PAGES) {
        printf("ERROR: Invalid virtual address %d\n", virtual_address);
        return -1;
    }

    // Check page table - is this page in memory?
    if (page_table[page_number].is_present) {
        // PAGE HIT - page is already in memory
        page_hits++;
        int frame   = page_table[page_number].frame_number;
        int physical = frame * PAGE_SIZE + page_offset;
        printf("PAGE HIT! Frame: %d, Physical Address: %d\n", frame, physical);
        return physical;
    } else {
        // PAGE FAULT - page is not in memory
        page_faults++;
        printf("PAGE FAULT! Page %d is not in memory\n", page_number);
        return -1;
    }
}

// Load a page into a specific frame
void load_page(int page_number, int frame_number) {
    // Remove old page from frame if there was one
    if (physical_memory[frame_number].is_valid) {
        int old_page = physical_memory[frame_number].page_number;
        page_table[old_page].is_present   = 0;
        page_table[old_page].frame_number = -1;
        printf("Evicting page %d from frame %d\n", old_page, frame_number);
    }

    // Load new page into frame
    physical_memory[frame_number].page_number = page_number;
    physical_memory[frame_number].is_valid    = 1;

    // Update page table
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].is_present   = 1;

    printf("Loaded page %d into frame %d\n", page_number, frame_number);
}

// Display current state of physical memory
void display_memory() {
    int i;
    printf("\n╔══════════════════════════════╗\n");
    printf("║     Physical Memory State    ║\n");
    printf("╠══════════════════════════════╣\n");
    printf("║ Frame │ Page  │ Status       ║\n");
    printf("╠══════════════════════════════╣\n");
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (physical_memory[i].is_valid) {
            printf("║  %3d  │  %3d  │ Occupied     ║\n",
                   i, physical_memory[i].page_number);
        } else {
            printf("║  %3d  │   -   │ Empty        ║\n", i);
        }
    }
    printf("╠══════════════════════════════╣\n");
    printf("║ Page Faults: %-16d║\n", page_faults);
    printf("║ Page Hits:   %-16d║\n", page_hits);
    printf("╚══════════════════════════════╝\n");
}

int main() {
    printf("========================================\n");
    printf("  Task 2 Layer 1: Paging System Demo\n");
    printf("  Page Size: %d | Frames: %d | Pages: %d\n",
           PAGE_SIZE, TOTAL_FRAMES, VIRTUAL_PAGES);
    printf("========================================\n");

    // Initialize memory
    initialize_memory();

    // Manually load some pages into frames to demonstrate paging
    printf("\n--- Loading pages into memory ---\n");
    load_page(0, 0);   // load virtual page 0 into frame 0
    load_page(1, 1);   // load virtual page 1 into frame 1
    load_page(3, 2);   // load virtual page 3 into frame 2

    // Show memory state
    display_memory();

    // Now test address translation
    printf("\n--- Address Translation Demo ---\n");
    translate_address(0);    // page 0, offset 0 -> HIT
    translate_address(5);    // page 1, offset 1 -> HIT
    translate_address(12);   // page 3, offset 0 -> HIT
    translate_address(8);    // page 2, offset 0 -> FAULT (not loaded)
    translate_address(20);   // page 5, offset 0 -> FAULT (not loaded)

    // Show final statistics
    display_memory();

    float total = page_hits + page_faults;
    printf("\nHit  Ratio: %.2f%%\n", (page_hits   / total) * 100);
    printf("Miss Ratio: %.2f%%\n", (page_faults / total) * 100);

    return 0;
}
