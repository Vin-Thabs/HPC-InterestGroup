#include <cuda_runtime.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define CUDA_CHECK(call)                                                        \
    do {                                                                        \
        cudaError_t error = (call);                                              \
        if (error != cudaSuccess) {                                              \
            std::fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__,         \
                         __LINE__, cudaGetErrorString(error));                    \
            std::exit(EXIT_FAILURE);                                             \
        }                                                                       \
    } while (0)

struct Point {
    float x;
    float y;
};

struct TimingResult {
    const char *name;
    double milliseconds;
    bool valid;
};

// ============================================================
// PASTE STEP 1: BASIC GPU KERNEL
// ============================================================

// ============================================================
// PASTE STEP 3: SQUARED DISTANCE GPU KERNEL
// ============================================================

// ============================================================
// PASTE STEP 5: SHARED MEMORY GPU KERNEL
// ============================================================

static void initialise_points(Point *points, int point_count) {
    for (int i = 0; i < point_count; ++i) {
        points[i].x = 0.01f * static_cast<float>(i % 100);
        points[i].y = 0.01f * static_cast<float>(i / 100);
    }
}

static void count_neighbours_cpu(const Point *points, int *neighbour_counts, int point_count) {
    const float radius = 0.047f;
    for (int i = 0; i < point_count; ++i) {
        int neighbours = 0;
        for (int j = 0; j < point_count; ++j) {
            float dx = points[j].x - points[i].x;
            float dy = points[j].y - points[i].y;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (i != j && distance < radius) ++neighbours;
        }
        neighbour_counts[i] = neighbours;
    }
}

static bool validate_counts(const int *reference, const int *candidate, int point_count) {
    for (int i = 0; i < point_count; ++i) {
        if (reference[i] != candidate[i]) {
            std::printf("First mismatch at %d: CPU %d, GPU %d\n",
                        i, reference[i], candidate[i]);
            return false;
        }
    }
    return true;
}

static void print_gpu_information() {
    int device = 0;
    cudaDeviceProp properties{};
    CUDA_CHECK(cudaGetDevice(&device));
    CUDA_CHECK(cudaGetDeviceProperties(&properties, device));

    std::printf("GPU: %s\n", properties.name);
    std::printf("Compute capability: %d.%d\n", properties.major, properties.minor);
    std::printf("Streaming multiprocessors: %d\n", properties.multiProcessorCount);
    std::printf("Warp size: %d\n", properties.warpSize);
    std::printf("Maximum threads per block: %d\n", properties.maxThreadsPerBlock);
    std::printf("Total global memory: %.2f GiB\n\n",
                static_cast<double>(properties.totalGlobalMem) /
                    (1024.0 * 1024.0 * 1024.0));
}

static float finish_kernel_timing(cudaEvent_t start, cudaEvent_t stop, int timing_repeats) {
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float milliseconds = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&milliseconds, start, stop));
    return milliseconds / static_cast<float>(timing_repeats);
}

int main(int argc, char **argv) {
    const int point_count = 10000;
    const int timing_repeats = 5;
    int threads_per_block = 256;

    if (argc > 2) {
        std::fprintf(stderr, "Usage: %s [threads-per-block]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        char *end = nullptr;
        long value = std::strtol(argv[1], &end, 10);
        if (*argv[1] == '\0' || *end != '\0' || value < 1 || value > 1024) {
            std::fprintf(stderr, "Block size must be an integer from 1 to 1024.\n");
            return EXIT_FAILURE;
        }
        threads_per_block = static_cast<int>(value);
    }

    CUDA_CHECK(cudaFree(nullptr));
    print_gpu_information();

    Point *points = new Point[point_count];
    int *cpu_counts = new int[point_count];
    int *gpu_counts = new int[point_count];
    initialise_points(points, point_count);

    auto cpu_start = std::chrono::steady_clock::now();
    count_neighbours_cpu(points, cpu_counts, point_count);
    auto cpu_stop = std::chrono::steady_clock::now();
    double cpu_ms =
        std::chrono::duration<double, std::milli>(cpu_stop - cpu_start).count();

    Point *device_points = nullptr;
    int *device_counts = nullptr;
    CUDA_CHECK(cudaMalloc(&device_points, point_count * sizeof(Point)));
    CUDA_CHECK(cudaMalloc(&device_counts, point_count * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(device_points, points, point_count * sizeof(Point),
                          cudaMemcpyHostToDevice));

    cudaEvent_t event_start;
    cudaEvent_t event_stop;
    CUDA_CHECK(cudaEventCreate(&event_start));
    CUDA_CHECK(cudaEventCreate(&event_stop));

    int block_count = (point_count + threads_per_block - 1) / threads_per_block;
    TimingResult timings[4];
    int timing_count = 1;
    timings[0] = {"CPU", cpu_ms, true};

    // ============================================================
    // PASTE STEP 2: BASIC GPU KERNEL LAUNCH
    // ============================================================

    // ============================================================
    // PASTE STEP 4: SQUARED DISTANCE GPU KERNEL LAUNCH
    // ============================================================

    // ============================================================
    // PASTE STEP 6: SHARED MEMORY GPU KERNEL LAUNCH
    // ============================================================

    std::printf("Points: %d\n", point_count);
    std::printf("Threads per block: %d\n\n", threads_per_block);
    std::printf("Implementation          Time        Speedup\n");
    std::printf("-------------------------------------------\n");
    for (int i = 0; i < timing_count; ++i) {
        double speedup = cpu_ms / timings[i].milliseconds;
        std::printf("%-22s %8.2f ms %9.2fx\n",
                    timings[i].name, timings[i].milliseconds, speedup);
    }

    if (timing_count > 1) {
        bool all_valid = true;
        for (int i = 1; i < timing_count; ++i) {
            all_valid = all_valid && timings[i].valid;
        }
        std::printf("\nValidation: %s\n", all_valid ? "PASS" : "FAIL");
    }

    CUDA_CHECK(cudaEventDestroy(event_start));
    CUDA_CHECK(cudaEventDestroy(event_stop));
    CUDA_CHECK(cudaFree(device_points));
    CUDA_CHECK(cudaFree(device_counts));
    delete[] points;
    delete[] cpu_counts;
    delete[] gpu_counts;
    return EXIT_SUCCESS;
}
