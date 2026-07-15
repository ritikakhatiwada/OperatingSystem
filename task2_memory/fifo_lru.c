#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Number of physical frames available in memory
#define TOTAL_FRAMES 4

// ============================================================
// FIFO PAGE REPLACEMENT
// Kicks out the page that has been in memory the longest
// Uses a simple queue - first in, first out
// ============================================================

typedef struct {
    int frames[TOTAL_FRAMES];  // pages currently in memory
    int count;                 // how many frames are occupied
    int next_replace;          // index of next frame to replace (FIFO pointer)
    int page_faults;           // total page faults so far
    int page_hits;             // total page hits so far
} FIFO_Memory;

// Initialize FIFO memory - all frames empty
void fifo_init(FIFO_Memory *mem) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        mem->frames[i] = -1;   // -1 means empty frame
    }
    mem->count        = 0;
    mem->next_replace = 0;
    mem->page_faults  = 0;
    mem->page_hits    = 0;
}

// Check if page is already in FIFO memory
int fifo_page_in_memory(FIFO_Memory *mem, int page) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (mem->frames[i] == page) return 1;
    }
    return 0;
}

// Print current state of FIFO frames
void fifo_display(FIFO_Memory *mem) {
    int i;
    printf("  Frames: [");
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (mem->frames[i] == -1) printf("  -");
        else printf("%3d", mem->frames[i]);
    }
    printf(" ]");
}

// Access a page using FIFO replacement
void fifo_access(FIFO_Memory *mem, int page) {
    if (fifo_page_in_memory(mem, page)) {
        // Page is already in memory - HIT
        mem->page_hits++;
        printf("Page %2d: HIT  ", page);
        fifo_display(mem);
        printf("\n");
    } else {
        // Page is not in memory - FAULT
        mem->page_faults++;

        if (mem->count < TOTAL_FRAMES) {
            // Memory not full yet - just load the page
            mem->frames[mem->count] = page;
            mem->count++;
        } else {
            // Memory is full - replace oldest page (FIFO)
            printf("Page %2d: FAULT - evicting page %d  ",
                   page, mem->frames[mem->next_replace]);
            mem->frames[mem->next_replace] = page;
            // Move FIFO pointer to next frame (circular)
            mem->next_replace = (mem->next_replace + 1) % TOTAL_FRAMES;
            fifo_display(mem);
            printf("\n");
            return;
        }

        printf("Page %2d: FAULT - loaded into memory  ", page);
        fifo_display(mem);
        printf("\n");
    }
}

// ============================================================
// LRU PAGE REPLACEMENT
// Kicks out the page that was least recently used
// Tracks when each page was last accessed
// ============================================================

typedef struct {
    int frames[TOTAL_FRAMES];      // pages currently in memory
    int last_used[TOTAL_FRAMES];   // timestamp of last use for each frame
    int count;                     // how many frames are occupied
    int time;                      // current timestamp counter
    int page_faults;               // total page faults
    int page_hits;                 // total page hits
} LRU_Memory;

// Initialize LRU memory - all frames empty
void lru_init(LRU_Memory *mem) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        mem->frames[i]    = -1;
        mem->last_used[i] = 0;
    }
    mem->count       = 0;
    mem->time        = 0;
    mem->page_faults = 0;
    mem->page_hits   = 0;
}

// Check if page is in LRU memory and return its index
int lru_find_page(LRU_Memory *mem, int page) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (mem->frames[i] == page) return i;
    }
    return -1;
}

// Find the least recently used frame to replace
int lru_find_victim(LRU_Memory *mem) {
    int i, min_time, victim;
    min_time = mem->last_used[0];
    victim   = 0;

    // Find frame with smallest timestamp = least recently used
    for (i = 1; i < TOTAL_FRAMES; i++) {
        if (mem->last_used[i] < min_time) {
            min_time = mem->last_used[i];
            victim   = i;
        }
    }
    return victim;
}

// Print current state of LRU frames
void lru_display(LRU_Memory *mem) {
    int i;
    printf("  Frames: [");
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (mem->frames[i] == -1) printf("  -");
        else printf("%3d", mem->frames[i]);
    }
    printf(" ]");
}

// Access a page using LRU replacement
void lru_access(LRU_Memory *mem, int page) {
    int idx;
    mem->time++;  // increment global timestamp

    idx = lru_find_page(mem, page);

    if (idx != -1) {
        // Page is already in memory - HIT
        mem->page_hits++;
        // Update last used time for this page
        mem->last_used[idx] = mem->time;
        printf("Page %2d: HIT  ", page);
        lru_display(mem);
        printf("\n");
    } else {
        // Page is not in memory - FAULT
        mem->page_faults++;

        if (mem->count < TOTAL_FRAMES) {
            // Memory not full - load directly
            mem->frames[mem->count]    = page;
            mem->last_used[mem->count] = mem->time;
            mem->count++;
            printf("Page %2d: FAULT - loaded into memory  ", page);
        } else {
            // Memory full - find and replace LRU page
            int victim = lru_find_victim(mem);
            printf("Page %2d: FAULT - evicting page %d  ",
                   page, mem->frames[victim]);
            mem->frames[victim]    = page;
            mem->last_used[victim] = mem->time;
        }

        lru_display(mem);
        printf("\n");
    }
}

// ============================================================
// MAIN - Run both algorithms on same reference string
// ============================================================

int main() {
    // This is the page reference string - sequence of page accesses
    // Same string used for both algorithms so we can compare fairly
    int reference_string[] = {1, 3, 0, 3, 5, 6, 3, 0, 1, 6, 5, 3, 2, 0, 1};
    int ref_length = 15;
    int i;

    printf("========================================\n");
    printf("  Task 2 Layer 2: Page Replacement\n");
    printf("  Frames: %d\n", TOTAL_FRAMES);
    printf("========================================\n");

    printf("\nPage Reference String:\n  ");
    for (i = 0; i < ref_length; i++) {
        printf("%d ", reference_string[i]);
    }
    printf("\n");

    // ---- FIFO ALGORITHM ----
    printf("\n============ FIFO Algorithm ============\n");
    FIFO_Memory fifo;
    fifo_init(&fifo);

    for (i = 0; i < ref_length; i++) {
        fifo_access(&fifo, reference_string[i]);
    }

    printf("\nFIFO Results:\n");
    printf("  Page Faults: %d\n", fifo.page_faults);
    printf("  Page Hits:   %d\n", fifo.page_hits);
    printf("  Hit  Ratio:  %.2f%%\n",
           (float)fifo.page_hits / ref_length * 100);
    printf("  Miss Ratio:  %.2f%%\n",
           (float)fifo.page_faults / ref_length * 100);

    // ---- LRU ALGORITHM ----
    printf("\n============= LRU Algorithm ============\n");
    LRU_Memory lru;
    lru_init(&lru);

    for (i = 0; i < ref_length; i++) {
        lru_access(&lru, reference_string[i]);
    }

    printf("\nLRU Results:\n");
    printf("  Page Faults: %d\n", lru.page_faults);
    printf("  Page Hits:   %d\n", lru.page_hits);
    printf("  Hit  Ratio:  %.2f%%\n",
           (float)lru.page_hits / ref_length * 100);
    printf("  Miss Ratio:  %.2f%%\n",
           (float)lru.page_faults / ref_length * 100);

    // ---- COMPARISON ----
    printf("\n============= Comparison ===============\n");
    printf("  %-12s %-12s %-12s\n", "Metric", "FIFO", "LRU");
    printf("  %-12s %-12d %-12d\n", "Page Faults",
           fifo.page_faults, lru.page_faults);
    printf("  %-12s %-12d %-12d\n", "Page Hits",
           fifo.page_hits, lru.page_hits);
    printf("  %-12s %-11.2f%% %-11.2f%%\n", "Hit Ratio",
           (float)fifo.page_hits / ref_length * 100,
           (float)lru.page_hits  / ref_length * 100);
    printf("  %-12s %s\n", "Winner",
           lru.page_faults <= fifo.page_faults ? "LRU" : "FIFO");

    return 0;
}
