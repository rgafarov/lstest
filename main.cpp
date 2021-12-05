#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>


// resource interface
class Resource {
public:
    virtual int inc(int max_threads, int thread_num) = 0;
};


// thread safe resource implementation based on std::atomic(spin lock) and acquire/release memory model
class ResourceAtomicSpin : public Resource {
    volatile int val = 0;
    std::atomic<int> spin_lock;
    virtual int inc(int max_threads, int thread_num) {
        int expect;
        // spin lock
        do {
            expect = 0;
        } while (spin_lock.compare_exchange_weak(expect, 1, std::memory_order_acquire, std::memory_order_acquire));
        // threade safe code
        if (val % max_threads == thread_num)
            val++;
        int ret = val;
        // spin unlock
        spin_lock.store(0, std::memory_order_release);
        // return copy of the resource
        return ret;
    }
};


// thread safe resource implementation based on std::atomic_flag and acquire/release memory model
class ResourceAtomicFlag : public Resource {
    volatile int val = 0;
    std::atomic_flag std_spin_lock = ATOMIC_FLAG_INIT;
    virtual int inc(int max_threads, int thread_num) {
        // std spin lock
        while (std_spin_lock.test_and_set(std::memory_order_acquire));
        // threade safe code
        if (val % max_threads == thread_num)
            val++;
        int ret = val;
        // std spin unlock
        std_spin_lock.clear(std::memory_order_release);
        // return copy of the resource
        return ret;
    }
};


// thread safe resource implementation based on std::mutex
class ResourceMutex : public Resource {
    volatile int val = 0;
    std::mutex m;
    virtual int inc(int max_threads, int thread_num) {
        m.lock();
        // threade safe code
        if (val % max_threads == thread_num)
            val++;
        int ret = val;
        m.unlock();
        // return copy of the resource
        return ret;
    }
};


// thread worker function
void worker(int max_threads, int thread_num, int max_count, Resource& r) {
    while (r.inc(max_threads, thread_num) < max_count);
};


// function for threads managing and execution performace measurement
int execSync(int max_threads, int max_count, Resource& r) {
    std::vector<std::thread> threads;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < max_threads; i++) {
        std::thread th(worker, max_threads, i, max_count, std::ref(r));
        threads.push_back(std::move(th));
    }

    for (int i = 0; i < max_threads; i++) {
        threads[i].join();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float>  d = t2 - t1;
    std::chrono::milliseconds dms = std::chrono::duration_cast<std::chrono::milliseconds>(d);

    return dms.count();
}


int main(int argc, char** argv) {
    constexpr int max_count_dflt = 1000000;
    int max_count = max_count_dflt;

    const int max_threads_dflt = std::thread::hardware_concurrency();
    int max_threads = max_threads_dflt;

    // trying to get max count value for resource
    if (argc > 1) {
        try {
            max_count = std::stoi(argv[1]);
            max_count = max_count < 0 ? max_count_dflt : max_count;
        } catch (...) {
            max_count = max_count_dflt;
        }
    }

    std::cout << "lstest started (CPU cores: " << max_threads_dflt << ")" << std::endl;

    // atomic based spinlock with number of cores threads
    ResourceAtomicSpin rspin;
    int exec_time = execSync(max_threads, max_count, rspin);
    std::cout << "execution time: " << exec_time << "ms "
              << "(custom spinlock, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    // atomic based spinlock with 2 * number of cores threads
    ResourceAtomicSpin rspin2;
    max_threads *= 2;
    exec_time = execSync(max_threads, max_count, rspin2);
    std::cout << "execution time: " << exec_time << "ms "
              << "(custom spinlock, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    // std spinlock with with number of cores threads
    ResourceAtomicFlag rstdspin;
    max_threads = max_threads_dflt;
    exec_time = execSync(max_threads, max_count, rstdspin);
    std::cout << "execution time: " << exec_time << "ms "
              << "(std spinlock, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    // std spinlock with 2 * number of cores threads
    ResourceAtomicFlag rstdspin2;
    max_threads *= 2;
    exec_time = execSync(max_threads, max_count, rstdspin2);
    std::cout << "execution time: " << exec_time << "ms "
              << "(std spinlock, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    // mutex with number of cores threads
    ResourceMutex rm;
    max_threads = max_threads_dflt;
    exec_time = execSync(max_threads, max_count, rm);
    std::cout << "execution time: " << exec_time << "ms "
              << "(mutex, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    // mutex with 2 * number of cores threads
    ResourceMutex rm2;
    max_threads *= 2;
    exec_time = execSync(max_threads, max_count, rm2);
    std::cout << "execution time: " << exec_time << "ms "
              << "(mutex, threads = " << max_threads << ", " << "count = " <<  max_count << ")" << std::endl;

    std::cout << "lstest stopped" << std::endl;

    return 0;
}