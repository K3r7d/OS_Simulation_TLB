# OS-assignment

### 1.1. Make sched 
To use `make sched`, we redirect to file `os-cfg.h` and comment out 2 lines:
- `#define MM_PAGING 1`
- `#define MM_FIXED_MEMSZ 1`

Syntax: `make sched` and `./sched <name of input>`
##### 1.1.1. With 3rd parameter (live priority)

Uncomment `#define MLQ_SCHED 1` (we need it)

##### 1.1.2. Without 3rd paramter (default priority):
Comment `#define MLQ_SCHED 1` (we don't need it)

### 1.2. Make mem

**NOTE:** In this mode, we will use `paging.c` for compiling. In `paging.c`:

- Reading configure size: In case we ourselves want to adjust MEMRAM size and MEMSWP size, we configure the input `mem_size` path
    
    - Example: We want MEMRAM size to have a maximum resource of `x` pages (256 x `x`B), and first MEMSWP has `y` pages (256 x `y`B)
   
   - **UPDATE**: You can choose input file and configuration for memory size you want as arguments

- Syntax: `make mem` and `./mem <name of process in folder proc (related to memory)> <config>`

    - Example: `./mem paging1 config1`
##### 1.2.1. Fixed memory size:

With fixed mem size, uncomment the `#define MM_FIXED_MEMSZ 1`

##### 1.2.2. Non-fixed memory size:

Comment the `#define MM_FIXED_MEMSZ 1`

### 1.3. Make all

- Uncomment all mode above
- Syntax: `make all` and `./os <name of file in input>`
