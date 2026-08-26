# CPU versus GPU with CUDA

In this workshop you will run the same neighbour-counting calculation on a CPU and an NVIDIA GPU. You will start with working boilerplate and add only a few small CUDA sections.

## What are we building?

Imagine 10,000 people standing in a square. For every person, the programme counts how many other people are standing within a small radius. Each person is represented by a 2D point with an `x` and `y` position.

For one person, the calculation looks like this:

```text
Person i
   │
   ├── measure distance to person 0
   ├── measure distance to person 1
   ├── measure distance to person 2
   └── continue to person N-1
                     │
                     ▼
          count the nearby people
```

The same calculation is repeated for every person:

```text
All positions
      │
      ├──► count neighbours for person 0
      ├──► count neighbours for person 1
      ├──► count neighbours for person 2
      ├──► ...
      └──► count neighbours for person N-1
```

The result is an array of neighbour counts:

```text
person 0: 21 neighbours
person 1: 26 neighbours
person 2: 31 neighbours
...
```

Each output is independent. Counting person 7's neighbours does not require person 6's result. This independence is what makes the workload suitable for a GPU.

### Why does the work grow quickly?

There are `N` people, and each person is compared with `N` people:

```text
N people × N distance checks = N² distance checks
```

With 10,000 people, that is about 100 million distance checks. This gives us enough repeated work to see a meaningful CPU and GPU timing difference.

### How the CPU and GPU approach it

The CPU version counts one person's neighbours after another with a nested loop:

```text
CPU core
   │
   ├── person 0 ──► count neighbours
   ├── person 1 ──► count neighbours
   ├── person 2 ──► count neighbours
   └── continue until person N-1
```

The GPU version starts many threads. Each thread is responsible for one person:

```text
GPU grid
   │
   ├── thread 0 ──► count neighbours for person 0
   ├── thread 1 ──► count neighbours for person 1
   ├── thread 2 ──► count neighbours for person 2
   ├── thread 3 ──► count neighbours for person 3
   └── thousands of similar threads
```

```text
CPU                              GPU
few powerful cores              many simpler execution lanes
good at varied sequential work  good at repeated parallel work
fewer outputs at once           many outputs in progress at once
```

The GPU still performs all the distance checks. Its advantage is that it can calculate many independent neighbour counts at the same time.

> Important: A GPU is not automatically faster for every problem. This example benefits because it contains a large amount of repeated arithmetic, has many independent outputs, and uses the same instructions for every output.

### Application flow

Both implementations receive the same deterministic point positions. Their integer results are compared exactly at the end.

```text
                          initialise_points points
                                  │
                    ┌─────────────┴─────────────┐
                    │                           │
                    ▼                           ▼
             run CPU version             copy data to GPU
                    │                           │
                    │                           ▼
                    │                    run CUDA kernel
                    │                           │
                    │                           ▼
                    │                    copy result to CPU
                    │                           │
                    └─────────────┬─────────────┘
                                  ▼
                         compare the results
                                  │
                                  ▼
                       print timings and speedup
```

During the workshop, you will begin with the CPU result and move the inner calculation into a CUDA kernel. You will then avoid an unnecessary square root, add shared-memory tiling, and experiment with thread-block sizes.

```text
gpu_cpu_demo_2026/
├── workshop.cu
├── solution.cu
├── snippets.txt
└── README.md
```

You only need to follow this guide. `snippets.txt` contains backup copies of the code, and `solution.cu` is the completed version if you get stuck.

## Step 0: Check the machine

### Start an interactive job

The university cluster uses Slurm to manage access to its compute nodes. Do not run the workshop on the login node. Ask Slurm for an interactive shell on one compute node:

```bash
srun -N1 -p batch --pty bash
```

This command means:

- `srun` asks Slurm to start a job.
- `-N1` requests one compute node.
- `-p batch` selects the `batch` partition.
- `--pty` creates an interactive terminal.
- `bash` starts a Bash shell on the allocated node.

When the command succeeds, your terminal prompt may change. Commands entered after that point run on the allocated compute node rather than the login node.

> Tip: The command can wait in the queue until a node is available. This is normal. Keep the terminal open while you wait.

Select the CUDA 13.3 compiler installed on the cluster:

```bash
export PATH=/usr/local/cuda-13.3/bin:$PATH
```

CUDA 13.3 is used because it works with the cluster's GCC 15 host compiler. CUDA 12.9 is also installed, but it rejects GCC versions later than 14.

Move into the workshop directory:

```bash
cd gpu_cpu_demo_2026
```

Now check the GPU and CUDA compiler:

```bash
nvidia-smi
nvcc --version
which nvcc
```

- `nvidia-smi` shows the NVIDIA GPU, driver, memory use, and running GPU processes.
- `nvcc --version` shows whether the CUDA compiler is available and which version is installed.
- `which nvcc` shows the location of the compiler.

> Tip: A working NVIDIA driver does not always mean that `nvcc` is installed. The driver runs CUDA programmes, while the CUDA toolkit supplies the compiler and development files.

At the end of the workshop, leave the allocated shell and release the node:

```bash
exit
```

## Step 1: Run the CPU version

Compile and run the starter file:

```bash
nvcc -O3 -arch=sm_86 workshop.cu -o demo
./demo
```

`-arch=sm_86` builds the GPU code for the cluster's RTX 3060, which has compute capability 8.6.

The programme first uses `cudaGetDeviceProperties` to report the GPU name, compute capability, streaming multiprocessors, warp size, maximum threads per block, and total global memory.

```text
CPU:
few powerful cores

GPU:
many simpler execution lanes
```

The workload counts nearby people for every point. Each point must be compared with every other point, so the work grows approximately as N squared.

```cpp
for each person i
    for each person j
        measure the distance from i to j
        count j if it is within the radius
```

The CPU time is measured with `std::chrono`. Your time is measured on your machine, so it will not necessarily match anyone else's result.

> Deeper note: `-O3` asks the compiler to optimise the programme. Performance comparisons should use an optimised CPU build, otherwise the comparison can be misleading.

## Step 2: Add a basic CUDA kernel

A CUDA launch has this hierarchy:

```text
Grid
  Block
    Thread
```

For this calculation:

```text
one GPU thread = one person
```

Every thread needs to discover which person it owns. This is the most important CUDA expression in the workshop:

```cpp
int i = blockIdx.x * blockDim.x + threadIdx.x;
```

- `blockIdx.x` is the block number.
- `blockDim.x` is the number of threads in a block.
- `threadIdx.x` is the thread's position inside its block.
- Combining them produces a unique global thread index.

### Paste the kernel

Find this marker near the top of `workshop.cu`:

```cpp
// PASTE STEP 1: BASIC GPU KERNEL
```

Paste this code immediately below it:

```cpp
__global__ void count_neighbours_basic(const Point *points, int *neighbour_counts, int point_count) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= point_count) return;

    const float radius = 0.047f;
    int neighbours = 0;
    for (int j = 0; j < point_count; ++j) {
        float dx = points[j].x - points[i].x;
        float dy = points[j].y - points[i].y;
        float distance = sqrtf(dx * dx + dy * dy);
        if (i != j && distance < radius) ++neighbours;
    }
    neighbour_counts[i] = neighbours;
}
```

Compile again:

```bash
nvcc -O3 -arch=sm_86 workshop.cu -o demo
```

The kernel now exists, but it does not run until the CPU launches it.

> Tip: The `if (i >= point_count)` guard matters because the grid is rounded up to a complete number of blocks. Some threads at the end may not own a person.

### Paste the kernel launch

Find this marker inside `main`:

```cpp
// PASTE STEP 2: BASIC GPU KERNEL LAUNCH
```

Paste this code immediately below it:

```cpp
    count_neighbours_basic<<<block_count, threads_per_block>>>(device_points, device_counts, point_count);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaEventRecord(event_start));
    for (int r = 0; r < timing_repeats; ++r)
        count_neighbours_basic<<<block_count, threads_per_block>>>(device_points, device_counts, point_count);
    float basic_ms = finish_kernel_timing(event_start, event_stop, timing_repeats);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(gpu_counts, device_counts, point_count * sizeof(int), cudaMemcpyDeviceToHost));
    timings[timing_count++] = {"GPU basic", basic_ms, validate_counts(cpu_counts, gpu_counts, point_count)};
```

The starter code already calculates the launch dimensions:

```cpp
int threads_per_block = 256;
int block_count = (point_count + threads_per_block - 1) / threads_per_block;
```

In this launch, `block_count` is the number of blocks in the grid and `threads_per_block` is the number of threads in each block:

```cpp
count_neighbours_basic<<<block_count, threads_per_block>>>(...);
```

Compile and run:

```bash
nvcc -O3 -arch=sm_86 workshop.cu -o demo
./demo
```

You should see CPU and GPU basic timing rows followed by `Validation: PASS`.

> Deeper note: CUDA kernel launches are asynchronous. The CPU can continue before the GPU has finished. CUDA events are recorded in the GPU's work stream, which makes them suitable for measuring kernel execution time.

## Step 3: Optimisation 1, avoid the square root

The basic kernel calculates the actual distance with `sqrtf`, then checks whether that distance is less than the radius. A square root is unnecessary here. Squaring both sides gives an equivalent test:

```text
distance < radius

is equivalent to

distance squared < radius squared
```

This is both simpler for the GPU and exact for our neighbour-counting decision.

### Paste the squared-distance kernel

Find `// PASTE STEP 3: SQUARED DISTANCE GPU KERNEL` and paste below it:

```cpp
__global__ void count_neighbours_squared(const Point *points, int *neighbour_counts, int point_count) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= point_count) return;

    const float radius_squared = 0.047f * 0.047f;
    Point current_point = points[i];
    int neighbours = 0;
    for (int j = 0; j < point_count; ++j) {
        float dx = points[j].x - current_point.x;
        float dy = points[j].y - current_point.y;
        float distance_squared = dx * dx + dy * dy;
        if (i != j && distance_squared < radius_squared) ++neighbours;
    }
    neighbour_counts[i] = neighbours;
}
```

### Paste its launch

Find `// PASTE STEP 4: SQUARED DISTANCE GPU KERNEL LAUNCH` and paste below it:

```cpp
    count_neighbours_squared<<<block_count, threads_per_block>>>(device_points, device_counts, point_count);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaEventRecord(event_start));
    for (int r = 0; r < timing_repeats; ++r)
        count_neighbours_squared<<<block_count, threads_per_block>>>(device_points, device_counts, point_count);
    float squared_ms = finish_kernel_timing(event_start, event_stop, timing_repeats);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(gpu_counts, device_counts, point_count * sizeof(int), cudaMemcpyDeviceToHost));
    timings[timing_count++] = {"GPU squared distance", squared_ms, validate_counts(cpu_counts, gpu_counts, point_count)};
```

Compile and run:

```bash
nvcc -O3 -arch=sm_86 workshop.cu -o demo
./demo
```

Observe the new `GPU squared distance` row. It may or may not be noticeably faster on your GPU.

> Tip: Never assume that an optimisation helps. Measure it. GPU architecture, compiler decisions, and the rest of the kernel all affect the result.

## Step 4: Optimisation 2, shared-memory tiling

Every person checks all point positions. Without tiling, threads repeatedly request point data from global memory.

```text
Global memory:
large and relatively expensive

Shared memory:
small, fast, and shared by one block
```

With tiling, threads in a block cooperate:

1. Each thread loads one point into shared memory.
2. The block waits until the tile is ready.
3. Every thread in the block reuses the tile.
4. The block repeats this for the next tile.

### Paste the shared-memory kernel

Find `// PASTE STEP 5: SHARED MEMORY GPU KERNEL` and paste below it:

```cpp
__global__ void count_neighbours_shared(const Point *points, int *neighbour_counts, int point_count) {
    extern __shared__ Point tile[];
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    Point current_point{};
    if (i < point_count) current_point = points[i];
    const float radius_squared = 0.047f * 0.047f;
    int neighbours = 0;

    for (int start = 0; start < point_count; start += blockDim.x) {
        int j = start + threadIdx.x;
        if (j < point_count) tile[threadIdx.x] = points[j];
        __syncthreads();

        int tile_count = point_count - start;
        if (tile_count > blockDim.x) tile_count = blockDim.x;
        if (i < point_count) {
            for (int k = 0; k < tile_count; ++k) {
                float dx = tile[k].x - current_point.x;
                float dy = tile[k].y - current_point.y;
                float distance_squared = dx * dx + dy * dy;
                if (start + k != i && distance_squared < radius_squared) ++neighbours;
            }
        }
        __syncthreads();
    }
    if (i < point_count) neighbour_counts[i] = neighbours;
}
```

`__syncthreads()` is a meeting point for all threads in a block. No thread may start using or replacing the tile until all threads reach the matching point.

> Deeper note: `__syncthreads()` works within one block only. Blocks execute independently and cannot use this function to wait for one another.

### Paste its launch

Find `// PASTE STEP 6: SHARED MEMORY GPU KERNEL LAUNCH` and paste below it:

```cpp
    size_t shared_bytes = threads_per_block * sizeof(Point);
    count_neighbours_shared<<<block_count, threads_per_block, shared_bytes>>>(device_points, device_counts, point_count);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaEventRecord(event_start));
    for (int r = 0; r < timing_repeats; ++r)
        count_neighbours_shared<<<block_count, threads_per_block, shared_bytes>>>(device_points, device_counts, point_count);
    float shared_ms = finish_kernel_timing(event_start, event_stop, timing_repeats);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(gpu_counts, device_counts, point_count * sizeof(int), cudaMemcpyDeviceToHost));
    timings[timing_count++] = {"GPU shared memory", shared_ms, validate_counts(cpu_counts, gpu_counts, point_count)};
```

The third launch value supplies the amount of dynamic shared memory:

```cpp
count_neighbours_shared<<<block_count, threads_per_block, shared_bytes>>>(...);
```

Compile and run:

```bash
nvcc -O3 -arch=sm_86 workshop.cu -o demo
./demo
```

You should see all four timing rows and `Validation: PASS`.

> Tip: Modern GPUs also have effective hardware caches. Shared-memory tiling makes data reuse explicit, but its benefit still depends on the GPU and workload.

## Step 5: Optimisation 3, threads per block

The argument handling is already included. Try several block sizes:

```bash
./demo 32
./demo 64
./demo 128
./demo 256
./demo 512
```

- A warp is a group of threads that the GPU schedules together. NVIDIA warps normally contain 32 threads.
- Occupancy describes how many warps can remain active on a streaming multiprocessor.
- Latency hiding means running other ready warps while one warp waits.
- GPU architecture affects register limits, shared memory, scheduling, and the best block size.

Do not assume that one block size is always best. Compare your own measured values.

> Deeper note: High occupancy can help hide latency, but maximum occupancy does not guarantee maximum performance. A useful block size balances occupancy with register use, shared-memory use, and the work performed by each thread.

## Reading the results

The table contains only implementations that you have added. Every displayed time is measured. Speedup is calculated as:

```text
CPU time / GPU kernel time
```

The GPU timing covers the kernel only. It does not include initial CUDA setup, memory allocation, or copying the input to the GPU. This isolates the execution difference being studied.

`Validation: PASS` means every GPU neighbour count exactly matches the CPU reference.

> Tip: Run the programme more than once. The first run, GPU temperature, clock speed, and other activity on the machine can affect timing.

## If you get stuck

The complete solution is `solution.cu`:

```bash
nvcc -O3 -arch=sm_86 solution.cu -o demo
./demo
```

You can also choose a block size:

```bash
./demo 256
```

By the end of the workshop, make sure you can explain this line in your own words:

```cpp
int i = blockIdx.x * blockDim.x + threadIdx.x;
```
