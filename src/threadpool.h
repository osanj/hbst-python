#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <vector>


class Threadpool {

private:
    uint32_t count;
    std::mutex m;
    std::condition_variable v;
    std::deque<std::packaged_task<void()>> tasks;
    std::vector<std::future<void>> finished;

    void threadTask();

public:
    Threadpool(uint32_t count);
    ~Threadpool();
    
    template<class F, class R=std::result_of_t<F&()>>
    std::future<R> queue(F&& f);
    
    void start();
    void abort();
    void cancelPending();
    void finish();
};
