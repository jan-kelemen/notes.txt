#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <type_traits>
#include <string>
#include <sstream>
#include <vector>

struct join_threads {
    join_threads(std::vector<std::thread>& threads)
        : threads{ threads }
    {
    }

    ~join_threads() { std::for_each(std::begin(threads), std::end(threads), std::mem_fn(&std::thread::join)); }
private:
    std::vector<std::thread>& threads;
};

template <typename T>
class threadsafe_queue {
private: // Types
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

public: // Interface
    threadsafe_queue()
        : head(std::make_unique<node>())
        , tail(head.get())
    {
    }

    threadsafe_queue(threadsafe_queue const&) = delete;

    std::shared_ptr<T> try_pop();

    bool try_pop(T& value);

    std::shared_ptr<T> wait_and_pop();

    void wait_and_pop(T& value);

    void push(T new_value);

    bool empty();

    threadsafe_queue& operator=(threadsafe_queue const&) = delete;

private: // Interface
    node* get_tail()
    {
        std::lock_guard tail_lock{ tail_mutex };
        return tail;
    }

    std::unique_ptr<node> pop_head()
    {
        std::unique_ptr<node> old_head{ std::move(head) };
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock head_lock{ head_mutex };
        data_cond.wait(head_lock, [&]() { return head.get() != get_tail(); });
        return head_lock;
    }

    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock head_lock{ wait_for_data() };
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock head_lock{ wait_for_data() };
        value = std::move(*(head->data));
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard head_lock{ head_mutex };
        if (head.get() == get_tail()) {
            return nullptr;
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T& value)
    {
        std::lock_guard head_lock{ head_mutex };
        if (head.get() == get_tail()) {
            return nullptr;
        }
        value = std::move(*(head->data));
        return pop_head();
    }

private: // Data
    std::mutex head_mutex;
    std::unique_ptr<node> head;

    std::mutex tail_mutex;
    node* tail;

    std::condition_variable data_cond;
};

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
    std::unique_ptr<node> old_head{ try_pop_head() };
    return old_head ? old_head->data : nullptr;
}

template <typename T>
bool threadsafe_queue<T>::try_pop(T& value)
{
    std::unique_ptr<node> old_head{ try_pop_head(value) };
    return static_cast<bool>(old_head);
}

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    std::unique_ptr<node> old_head{ wait_pop_head() };
    return old_head->data;
}

template <typename T>
void threadsafe_queue<T>::wait_and_pop(T& value)
{
    std::unique_ptr<node> old_head{ wait_pop_head(value) };
}

template <typename T>
void threadsafe_queue<T>::push(T new_value)
{
    auto new_data{ std::make_shared<T>(std::move(new_value)) };
    auto new_node{ std::make_unique<node>() };
    {
        std::lock_guard tail_lock{ tail_mutex };
        tail->data = std::move(new_data);
        node* const new_tail{ new_node.get() };
        tail->next = std::move(new_node);
        tail = new_tail;
    }
    data_cond.notify_one();
}

template <typename T>
bool threadsafe_queue<T>::empty()
{
    std::lock_guard head_lock{ head_mutex };
    return head.get() == get_tail();
}

class function_wrapper {
    // Basically the same trick as in https://github.com/jan-kelemen/notes.txt/tree/master/talks/Sean_Parent_-_Interitance_Is_The_Base_Class_Of_Evil
    // Real function type is type erased through impl_type -> impl_base,
    // which enables them to be stored in the same container, std::unique_ptr in this case.
    // std::function is also type erased but requires that functions are copyable.

private: // Types
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base(){};
    };

    template <typename F>
    struct impl_type : impl_base {
        F f;
        impl_type(F&& f)
            : f(std::move(f))
        {
        }
        void call() override { f(); }
        ~impl_type() override{};
    };

public: // Interface
    function_wrapper() = default;

    function_wrapper(function_wrapper const&) = delete;

    template <typename F>
    function_wrapper(F&& f)
        : impl(new impl_type<F>(std::move(f)))
    {
    }

    function_wrapper(function_wrapper&& other) noexcept = default;

    void operator()() { impl->call(); }

    function_wrapper& operator=(function_wrapper const&) = delete;

    function_wrapper& operator=(function_wrapper&& other) noexcept = default;

private: // Data
    std::unique_ptr<impl_base> impl;
};

class work_stealing_queue {
private: // Types
    using data_type = function_wrapper;

public: // Interface
    work_stealing_queue() = default;

    work_stealing_queue(work_stealing_queue const&) = delete;

    work_stealing_queue& operator=(work_stealing_queue const&) = delete;

    void push(data_type data)
    {
        std::lock_guard lock{ the_mutex };
        the_queue.push_front(std::move(data));
    }

    bool empty() const
    {
        std::lock_guard lock{ the_mutex };
        return the_queue.empty();
    }

    bool try_pop(data_type& res)
    {
        std::lock_guard lock{ the_mutex };
        if (the_queue.empty()) {
            return false;
        }

        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }

    bool try_steal(data_type& res)
    {
        std::lock_guard lock{ the_mutex };
        if (the_queue.empty()) {
            return false;
        }
        res = std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }

private: // Data
    mutable std::mutex the_mutex;
    std::deque<data_type> the_queue;
};

class thread_pool {
private: // Types
    using task_type = function_wrapper;

public: // Interface
    thread_pool()
        : done{ false }
        , joiner{ threads }
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            std::generate_n(std::back_inserter(queues), thread_count, std::make_unique<work_stealing_queue>);

            for (unsigned i = 0; i != thread_count; ++i) {
                threads.emplace_back(&thread_pool::worker_thread, this, i);
            }
        }
        catch (...) {
            done = true;
            throw;
        }
    }

    template <typename FunctionType>
    std::future<std::invoke_result_t<FunctionType> > submit(FunctionType f)
    {
        using result_type = std::invoke_result_t<FunctionType>;

        std::packaged_task<result_type()> task{ f };
        std::future<result_type> res{ task.get_future() };
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        }
        else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    ~thread_pool() { done = true; }

private: // Helpers
    void worker_thread(unsigned my_index_)
    {
        my_index = my_index;
        local_work_queue = queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type& task)
    {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type& task)
    {
        return pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task)
    {
        for (unsigned i = 0; i != queues.size(); ++i) {
            unsigned const index = (my_index + i + 1) % queues.size();
            if (queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

    void run_pending_task()
    {
        task_type task;
        if (pop_task_from_local_queue(task) || pop_task_from_pool_queue(task) || pop_task_from_other_thread_queue(task)) {
            task();
        }
        else {
            std::this_thread::yield();
        }
    }

private: // Data
    std::atomic_bool done;
    threadsafe_queue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue> > queues;
    std::vector<std::thread> threads;
    join_threads joiner;
    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned my_index;
};

thread_local work_stealing_queue* thread_pool::local_work_queue{};
thread_local unsigned thread_pool::my_index{};

int main()
{
    thread_pool pool;

    std::vector<std::future<void> > results;
    for (unsigned i{ 0 }; i != std::thread::hardware_concurrency() - 1; ++i) {
        results.push_back(pool.submit([ issuer = std::this_thread::get_id(), &pool ]() {
            using namespace std::chrono_literals;

            std::stringstream s;

            s << "Task issued on " << issuer << ", executed on " << std::this_thread::get_id() << " going to sleep\n";

            std::cout << s.str();

            pool.submit([issuer = std::this_thread::get_id()]() {
                std::stringstream s;
                s << "Task issued on " << issuer << ", executed on " << std::this_thread::get_id() << " stolen?\n";
                std::cout << s.str();
            });

            std::this_thread::sleep_for(20s);
        }));
    }

    for (unsigned i{ 0 }; i != std::thread::hardware_concurrency(); ++i) {
        results.push_back(pool.submit([issuer = std::this_thread::get_id()]() {
            using namespace std::chrono_literals;
            std::stringstream s;
            s << "Task issued on " << issuer << ", executed on " << std::this_thread::get_id() << "\n";
            std::cout << s.str();
        }));
    }

    for (auto&& r : results) {
        r.get();
    }
}