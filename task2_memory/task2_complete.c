#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// TASK 2: Memory Management Simulation
// Student: Ritika Khatiwada
// Module: ST5004CEM Operating Systems and Security
// Description: Paging system with FIFO and LRU page
//              replacement algorithms, page fault tracking,
//              hit/miss ratio calculation and visualization
// ============================================================

// Configurable settings
#define PAGE_SIZE     4       // each page holds 4 units
#define TOTAL_FRAMES  4       // physical memory frames
#define VIRTUAL_PAGES 8       // total virtual pages
#define MAX_REF       20      // max reference string length

// ============================================================
// SECTION 1: PAGING SYSTEM
// ============================================================

typedef struct {
    int page_number;   // which virtual page is here
    int is_valid;      // 1 = occupied, 0 = empty
} Frame;

typedef struct {
    int frame_number;  // physical frame holding this page
    int is_present;    // 1 = in memory, 0 = not loaded
} PageTableEntry;

Frame          physical_memory[TOTAL_FRAMES];
PageTableEntry page_table[VIRTUAL_PAGES];
int            total_faults = 0;
int            total_hits   = 0;

// Initialize all frames and page table entries
void init_paging() {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        physical_memory[i].page_number = -1;
        physical_memory[i].is_valid    = 0;
    }
    for (i = 0; i < VIRTUAL_PAGES; i++) {
        page_table[i].frame_number = -1;
        page_table[i].is_present   = 0;
    }
    total_faults = 0;
    total_hits   = 0;
}

// Load a virtual page into a physical frame
void load_page(int page_num, int frame_num) {
    // Evict old page if frame was occupied
    if (physical_memory[frame_num].is_valid) {
        int old = physical_memory[frame_num].page_number;
        page_table[old].is_present   = 0;
        page_table[old].frame_number = -1;
    }
    // Load new page
    physical_memory[frame_num].page_number = page_num;
    physical_memory[frame_num].is_valid    = 1;
    page_table[page_num].frame_number      = frame_num;
    page_table[page_num].is_present        = 1;
}

// Translate virtual address to physical address
int translate_address(int vaddr) {
    int page   = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;

    if (page >= VIRTUAL_PAGES) {
        printf("  ERROR: Virtual address %d out of range\n", vaddr);
        return -1;
    }

    if (page_table[page].is_present) {
        total_hits++;
        int frame   = page_table[page].frame_number;
        int paddr   = frame * PAGE_SIZE + offset;
        printf("  VA:%3d -> Page:%d Offset:%d | HIT  -> PA:%3d\n",
               vaddr, page, offset, paddr);
        return paddr;
    } else {
        total_faults++;
        printf("  VA:%3d -> Page:%d Offset:%d | FAULT (not in memory)\n",
               vaddr, page, offset);
        return -1;
    }
}

// Display physical memory as a visual table
void display_memory_table(char *title) {
    int i;
    printf("\n  +-------+-------+----------+\n");
    printf("  | %-25s |\n", title);
    printf("  +-------+-------+----------+\n");
    printf("  | Frame | Page  | Status   |\n");
    printf("  +-------+-------+----------+\n");
    for (i = 0; i < TOTAL_FRAMES; i++) {
        if (physical_memory[i].is_valid)
            printf("  |  %3d  |  %3d  | Occupied |\n",
                   i, physical_memory[i].page_number);
        else
            printf("  |  %3d  |   -   | Empty    |\n", i);
    }
    printf("  +-------+-------+----------+\n");
    printf("  | Faults: %-3d  Hits: %-3d  |\n", total_faults, total_hits);
    printf("  +---------------------------+\n");
}

// ============================================================
// SECTION 2: FIFO PAGE REPLACEMENT
// ============================================================

typedef struct {
    int  frames[TOTAL_FRAMES];
    int  count;
    int  next_replace;
    int  page_faults;
    int  page_hits;
    // Log each step for visualization
    int  log_pages[MAX_REF];
    int  log_frames[MAX_REF][TOTAL_FRAMES];
    char log_result[MAX_REF][8];
    int  log_count;
} FIFO;

void fifo_init(FIFO *f) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) f->frames[i] = -1;
    f->count        = 0;
    f->next_replace = 0;
    f->page_faults  = 0;
    f->page_hits    = 0;
    f->log_count    = 0;
}

int fifo_has_page(FIFO *f, int page) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++)
        if (f->frames[i] == page) return 1;
    return 0;
}

void fifo_access(FIFO *f, int page) {
    int i;

    // Save log entry
    f->log_pages[f->log_count] = page;
    for (i = 0; i < TOTAL_FRAMES; i++)
        f->log_frames[f->log_count][i] = f->frames[i];

    if (fifo_has_page(f, page)) {
        f->page_hits++;
        strcpy(f->log_result[f->log_count], "HIT");
    } else {
        f->page_faults++;
        strcpy(f->log_result[f->log_count], "FAULT");

        if (f->count < TOTAL_FRAMES) {
            f->frames[f->count++] = page;
        } else {
            f->frames[f->next_replace] = page;
            f->next_replace = (f->next_replace + 1) % TOTAL_FRAMES;
        }

        // Update log with new state after replacement
        for (i = 0; i < TOTAL_FRAMES; i++)
            f->log_frames[f->log_count][i] = f->frames[i];
    }
    f->log_count++;
}

// Display FIFO results as a detailed visualization table
void fifo_display(FIFO *f, int ref_len) {
    int i, j;
    printf("\n  FIFO Step-by-Step Visualization:\n");
    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");
    printf("  | Step | Page   ");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("| Fr%-3d ", j);
    printf("| Result |\n");
    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");

    for (i = 0; i < f->log_count; i++) {
        printf("  | %4d | %6d ", i+1, f->log_pages[i]);
        for (j = 0; j < TOTAL_FRAMES; j++) {
            if (f->log_frames[i][j] == -1)
                printf("|     - ");
            else
                printf("| %5d ", f->log_frames[i][j]);
        }
        printf("| %-6s |\n", f->log_result[i]);
    }

    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");
    printf("  | Faults: %-3d  Hits: %-3d  Hit Rate: %5.2f%%  Miss Rate: %5.2f%% |\n",
           f->page_faults, f->page_hits,
           (float)f->page_hits   / ref_len * 100,
           (float)f->page_faults / ref_len * 100);
    printf("  +");
    for (j = 0; j < 8 + TOTAL_FRAMES * 8; j++) printf("-");
    printf("+\n");
}

// ============================================================
// SECTION 3: LRU PAGE REPLACEMENT
// ============================================================

typedef struct {
    int  frames[TOTAL_FRAMES];
    int  last_used[TOTAL_FRAMES];
    int  count;
    int  time;
    int  page_faults;
    int  page_hits;
    // Log
    int  log_pages[MAX_REF];
    int  log_frames[MAX_REF][TOTAL_FRAMES];
    char log_result[MAX_REF][8];
    int  log_count;
} LRU;

void lru_init(LRU *l) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++) {
        l->frames[i]    = -1;
        l->last_used[i] = 0;
    }
    l->count       = 0;
    l->time        = 0;
    l->page_faults = 0;
    l->page_hits   = 0;
    l->log_count   = 0;
}

int lru_find(LRU *l, int page) {
    int i;
    for (i = 0; i < TOTAL_FRAMES; i++)
        if (l->frames[i] == page) return i;
    return -1;
}

int lru_victim(LRU *l) {
    int i, min = l->last_used[0], v = 0;
    for (i = 1; i < TOTAL_FRAMES; i++)
        if (l->last_used[i] < min) { min = l->last_used[i]; v = i; }
    return v;
}

void lru_access(LRU *l, int page) {
    int i, idx;
    l->time++;

    // Save log
    l->log_pages[l->log_count] = page;
    for (i = 0; i < TOTAL_FRAMES; i++)
        l->log_frames[l->log_count][i] = l->frames[i];

    idx = lru_find(l, page);

    if (idx != -1) {
        l->page_hits++;
        l->last_used[idx] = l->time;
        strcpy(l->log_result[l->log_count], "HIT");
    } else {
        l->page_faults++;
        strcpy(l->log_result[l->log_count], "FAULT");

        if (l->count < TOTAL_FRAMES) {
            l->frames[l->count]    = page;
            l->last_used[l->count] = l->time;
            l->count++;
        } else {
            int v = lru_victim(l);
            l->frames[v]    = page;
            l->last_used[v] = l->time;
        }

        // Update log
        for (i = 0; i < TOTAL_FRAMES; i++)
            l->log_frames[l->log_count][i] = l->frames[i];
    }
    l->log_count++;
}

void lru_display(LRU *l, int ref_len) {
    int i, j;
    printf("\n  LRU Step-by-Step Visualization:\n");
    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");
    printf("  | Step | Page   ");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("| Fr%-3d ", j);
    printf("| Result |\n");
    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");

    for (i = 0; i < l->log_count; i++) {
        printf("  | %4d | %6d ", i+1, l->log_pages[i]);
        for (j = 0; j < TOTAL_FRAMES; j++) {
            if (l->log_frames[i][j] == -1)
                printf("|     - ");
            else
                printf("| %5d ", l->log_frames[i][j]);
        }
        printf("| %-6s |\n", l->log_result[i]);
    }

    printf("  +------+--------");
    for (j = 0; j < TOTAL_FRAMES; j++) printf("+-------");
    printf("+--------+\n");
    printf("  | Faults: %-3d  Hits: %-3d  Hit Rate: %5.2f%%  Miss Rate: %5.2f%% |\n",
           l->page_faults, l->page_hits,
           (float)l->page_hits   / ref_len * 100,
           (float)l->page_faults / ref_len * 100);
    printf("  +");
    for (j = 0; j < 8 + TOTAL_FRAMES * 8; j++) printf("-");
    printf("+\n");
}

// ============================================================
// SECTION 4: MAIN
// ============================================================

int main() {
    int i;

    // Page reference string used for both algorithms
    int ref[] = {1, 3, 0, 3, 5, 6, 3, 0, 1, 6, 5, 3, 2, 0, 1};
    int ref_len = 15;

    printf("=============================================\n");
    printf("  ST5004CEM Task 2: Memory Management\n");
    printf("  Student: Ritika Khatiwada\n");
    printf("  Page Size: %d | Frames: %d | Pages: %d\n",
           PAGE_SIZE, TOTAL_FRAMES, VIRTUAL_PAGES);
    printf("=============================================\n");

    // ---- DEMO 1: PAGING SYSTEM ----
    printf("\n======= DEMO 1: Paging System =======\n");
    init_paging();

    // Load some pages manually
    printf("\nLoading pages into memory:\n");
    load_page(0, 0);
    load_page(1, 1);
    load_page(3, 2);

    display_memory_table("Initial Memory State");

    // Test address translation
    printf("\nAddress Translation:\n");
    translate_address(0);
    translate_address(5);
    translate_address(12);
    translate_address(8);
    translate_address(20);

    display_memory_table("After Translation");

    float total = total_hits + total_faults;
    printf("\n  Paging Hit  Ratio: %.2f%%\n", total_hits   / total * 100);
    printf("  Paging Miss Ratio: %.2f%%\n", total_faults / total * 100);

    // ---- DEMO 2: FIFO ----
    printf("\n======= DEMO 2: FIFO Algorithm =======\n");
    printf("\nReference String: ");
    for (i = 0; i < ref_len; i++) printf("%d ", ref[i]);
    printf("\n");

    FIFO fifo;
    fifo_init(&fifo);
    for (i = 0; i < ref_len; i++) fifo_access(&fifo, ref[i]);
    fifo_display(&fifo, ref_len);

    // ---- DEMO 3: LRU ----
    printf("\n======= DEMO 3: LRU Algorithm =======\n");
    printf("\nReference String: ");
    for (i = 0; i < ref_len; i++) printf("%d ", ref[i]);
    printf("\n");

    LRU lru;
    lru_init(&lru);
    for (i = 0; i < ref_len; i++) lru_access(&lru, ref[i]);
    lru_display(&lru, ref_len);

    // ---- DEMO 4: COMPARISON ----
    printf("\n======= DEMO 4: Algorithm Comparison =======\n");
    printf("\n  +----------------+-------+-------+\n");
    printf("  | Metric         | FIFO  | LRU   |\n");
    printf("  +----------------+-------+-------+\n");
    printf("  | Page Faults    | %5d | %5d |\n",
           fifo.page_faults, lru.page_faults);
    printf("  | Page Hits      | %5d | %5d |\n",
           fifo.page_hits, lru.page_hits);
    printf("  | Hit  Rate      | %4.1f%% | %4.1f%% |\n",
           (float)fifo.page_hits / ref_len * 100,
           (float)lru.page_hits  / ref_len * 100);
    printf("  | Miss Rate      | %4.1f%% | %4.1f%% |\n",
           (float)fifo.page_faults / ref_len * 100,
           (float)lru.page_faults  / ref_len * 100);
    printf("  +----------------+-------+-------+\n");
    printf("  | Winner: %-24s |\n",
           fifo.page_faults <= lru.page_faults ? "FIFO (fewer faults)" : "LRU  (fewer faults)");
   
