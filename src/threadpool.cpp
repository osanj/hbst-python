#include "threadpool.h"


Threadpool::Threadpool(uint32_t count_) : count(count_) {}

Threadpool::~Threadpool() {
    finish();
}

void Threadpool::threadTask() {
    while(true){
        std::packaged_task<void()> f;
        {
            std::unique_lock<std::mutex> l(m);
            if (tasks.empty()) {
                v.wait(l, [&]{
                    return !tasks.empty();
                });
            }
            f = std::move(tasks.front());
            tasks.pop_front();
        }

        if (!f.valid()) {
            return;
        }

        f();
    }
}

template<class F, class R>
std::future<R> Threadpool::enqueue(F&& f) {
    std::packaged_task<R()> p(std::forward<F>(f));

    auto r = p.get_future();
    {
        std::unique_lock<std::mutex> l(m);
        tasks.emplace_back(std::move(p));
    }

    v.notify_one();
    return r;
}

uint32_t Threadpool::size() {
    return count;
}

void Threadpool::start() {
    for (std::size_t i = 0; i < count; ++i) {
        finished.push_back(std::async(std::launch::async, [this]{ threadTask(); }));
    }
}

void Threadpool::abort() {
    cancelPending();
    finish();
}

void Threadpool::cancelPending() {
    std::unique_lock<std::mutex> l(m);
    tasks.clear();
}

void Threadpool::finish() {
    {
        std::unique_lock<std::mutex> l(m);
        for (auto&& unused : finished) {
            tasks.push_back({});
        }
    }
    v.notify_all();
    finished.clear();
}
