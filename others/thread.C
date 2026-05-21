#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#define totalNumThreads 20
std::mutex gLock;

int shared_value = 0;
std::atomic<int> atomic_shared_value = totalNumThreads;

int main()
{
    std::vector<std::thread> threads;

    auto lambda = [&] {
      {
        std::lock_guard<std::mutex> lock(gLock);
        std::cout << "shared_value = " <<  shared_value++ << ", atomic_shared_value = " << --atomic_shared_value << " in thread id " << std::this_thread::get_id() << std::endl;
      }
    };

    for(int i=0; i<totalNumThreads; i++)
	{
      threads.push_back(std::thread(lambda));
	}

    for(int i=0; i<totalNumThreads; i++)
	{
      threads[i].join();
	}

    std::cout << "main thread terminated" << std::endl;
    return 0;
}
