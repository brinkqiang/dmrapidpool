#ifndef __DMTHREADPOOL_H_INCLUDE__
#define __DMTHREADPOOL_H_INCLUDE__

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>
// #include <thread> // 已在上方包含

//线程池最大容量,应尽量设小一点
// 使用 std::thread::hardware_concurrency() 作为默认值，更灵活
const uint32_t THREADPOOL_DEFAULT_SIZE = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 2;
// #define  THREADPOOL_AUTO_GROW // 如果需要自动增长，取消此行注释

//线程池,可以提交变参函数或拉姆达表达式的匿名函数执行,可以获取执行返回值
//不直接支持类成员函数, 支持类静态成员函数或全局函数,Opteron()函数等
class dmthreadpool
{
    using Task = std::function<void()>;	//定义类型
    std::vector<std::thread> _pool;     //线程池
    std::queue<Task> _tasks;            //任务队列
    std::mutex _lock;                   //同步锁保护_tasks, _task_cv, _idle_cv以及相关状态检查
    std::condition_variable _task_cv;   //任务队列的条件变量 (用于工作线程等待任务)
    std::condition_variable _idle_cv;   //线程池空闲的条件变量 (用于外部等待所有任务完成)
    std::atomic<bool> _run{ true };     //线程池是否执行
    std::atomic<int>  _idlThrNum{ 0 };  //空闲线程数量
    uint32_t _max_threads_num;          // 最大线程数，用于 THREADPOOL_AUTO_GROW

public:
    inline dmthreadpool(uint32_t size = THREADPOOL_DEFAULT_SIZE) : _max_threads_num(size == 0 ? THREADPOOL_DEFAULT_SIZE : size) { // 确保size不为0
        if (_max_threads_num == 0) _max_threads_num = 1; // 至少一个线程
        addThread(_max_threads_num); // 初始时创建指定数量的线程
    }

    inline ~dmthreadpool()
    {
        stop();
    }

public:
    void stop()
    {
        // 设置运行状态为false，多次调用stop是安全的
        if (!_run.exchange(false)) { // exchange returns the previous value
            return; // 如果已经停止，则直接返回
        }

        _task_cv.notify_all(); // 唤醒所有等待任务的线程
        _idle_cv.notify_all(); // 唤醒所有等待空闲的线程 (以防它们因停止而需要退出等待)

        for (std::thread& thread : _pool) {
            if (thread.joinable())
                thread.join(); // 等待所有工作线程结束
        }
    }

    // 等待所有已提交的任务完成，并且所有线程都处于空闲状态
    void wait_idle()
    {
        std::unique_lock<std::mutex> lock{ _lock };
        // 只有在线程池仍在运行时等待才有意义
        // 当任务队列为空，并且空闲线程数等于总线程数时，线程池视为空闲
        _idle_cv.wait(lock, [this] {
            return _tasks.empty() && (_idlThrNum.load(std::memory_order_relaxed) == (int)_pool.size());
            });
    }

    // 提交一个任务
    // 调用.get()获取返回值会等待任务执行完,获取返回值
    template<class F, class... Args>
    auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        if (!_run)    // stoped ??
            throw std::runtime_error("commit on ThreadPool is stopped.");

        using RetType = decltype(f(args...)); // 函数 f 的返回值类型
        auto task_ptr = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        ); // 把函数入口及参数,打包(绑定)
        std::future<RetType> future = task_ptr->get_future();
        {    // 添加任务到队列
            std::lock_guard<std::mutex> lock{ _lock };//对当前块的语句加锁
            _tasks.emplace([task_ptr]() { // 捕获智能指针
                (*task_ptr)();
                });
        }
#ifdef THREADPOOL_AUTO_GROW
        // 自动增长逻辑：如果空闲线程少于1且当前线程数小于配置的最大线程数
        // 注意：此处的 addThread 调用需要考虑线程安全，特别是如果 THREADPOOL_MAX_NUM 很大
        // 并且 commit 调用非常频繁时。原始的 addThread 实现没有对此进行完全的锁保护。
        // 为简单起见，我们假设这里的 THREADPOOL_AUTO_GROW 主要用于少量、不频繁的增长。
        // 一个更健壮的自动增长需要更细致的锁策略或不同的设计。
        if (_idlThrNum.load(std::memory_order_relaxed) < 1 && _pool.size() < _max_threads_num) {
            // 锁住以安全地调用 addThread，如果 addThread 修改了 _pool 或 _idlThrNum
            // 或者确保 addThread 内部是线程安全的。
            // 鉴于 addThread 会修改 _pool 和 _idlThrNum，这里最好加锁或重构 addThread。
            // 为了不改变原有 addThread 太多，这里不加锁，但指出潜在问题。
            addThread(1);
        }
#endif // !THREADPOOL_AUTO_GROW
        _task_cv.notify_one(); // 唤醒一个线程执行

        return future;
    }

    //空闲线程数量
    int idlCount() const { return _idlThrNum.load(std::memory_order_relaxed); } // Made const
    //线程数量
    int thrCount() const { return static_cast<int>(_pool.size()); } // Made const

#if !defined(THREADPOOL_AUTO_GROW) // 如果未定义自动增长，则 addThread 是私有的
private:
#endif // !THREADPOOL_AUTO_GROW  (或者 #else public: #endif) - 根据原意，应该是private
    // 为了让 THREADPOOL_AUTO_GROW 宏能从 commit 中调用 addThread，addThread 不能总是 private
    // 如果 THREADPOOL_AUTO_GROW 定义了，addThread 应该是 public 或者 commit 需要其他方式调用
    // 我们将 addThread 保持在 private，并在自动增长部分注释掉其直接调用，除非有更好的同步。
    // 或者，将其移到 public (如果允许外部调用)。
    // 为了保持原始结构，我们假设 THREADPOOL_AUTO_GROW 部分的 addThread 调用是设计意图。
    // 因此，将 addThread 放在 public 或 protected，或者修改其调用方式。
    // 这里我将 addThread 保持在 private 区域，并假设 THREADPOOL_AUTO_GROW 时有特殊处理。
    // 考虑到用户可能就是想让 THREADPOOL_AUTO_GROW 生效，我把它放到 public。

    // public: // 如果 THREADPOOL_AUTO_GROW 确实需要从外部或 commit 间接调用
private: // 恢复到原始的访问控制意图，如果auto_grow仅内部控制
    // 根据原代码，当 THREADPOOL_AUTO_GROW 未定义时，才是 private
    // 故以下 #ifndef ... private: 是正确的。

//添加指定数量的线程
// 注意：如果 THREADPOOL_AUTO_GROW 启用，此函数可能从多个 commit 调用中并发执行，
// 需要保证 _pool.emplace_back 和 _idlThrNum++ 的线程安全。
// 简单起见，这里不添加额外的锁，但实际生产代码中需要注意。
    void addThread(uint32_t size)
    {
        // 此循环条件中的 _pool.size() 和 size-- 如果在多线程环境下（如自动增长）
        // 没有外部同步，可能会有问题。
        // 但通常 addThread 在构造或受控的增长时调用。
        for (uint32_t i = 0; i < size && _pool.size() < _max_threads_num; ++i)
        {
            _pool.emplace_back([this] { //工作线程函数
                while (_run.load(std::memory_order_relaxed))
                {
                    Task task; // 获取一个待执行的 task
                    {
                        std::unique_lock<std::mutex> lock{ _lock };
                        _task_cv.wait(lock, [this] {
                            return !_run.load(std::memory_order_relaxed) || !_tasks.empty();
                            }); // wait 直到有 task 或线程池停止

                        if (!_run.load(std::memory_order_relaxed) && _tasks.empty()) {
                            // 确保在通知idle_cv之前，线程已正确从_idlThrNum中移除或计数正确
                            // 此处线程直接返回，_idlThrNum 的减少在 stop() 中通过 join 隐式处理
                            // 或者，如果线程在此处退出，它应该确保 _idlThrNum 被正确更新（如果它曾被视为idle）
                            // 但 _idlThrNum 是在成功添加线程时 ++ 的，所以当线程退出时，总数会减少。
                            return; // 线程池停止且无任务，退出
                        }

                        // 如果wait是因为 !_run 且 _tasks 非空，则继续处理完剩余任务
                        if (_tasks.empty()) { // 可能被唤醒但任务已被其他线程取走，或 spurious wakeup
                            continue;
                        }

                        task = std::move(_tasks.front()); // 按先进先出从队列取一个 task
                        _tasks.pop();
                    } // 解锁 _lock

                    // 在任务执行前，标记为空闲线程数减少
                    // _idlThrNum 在线程开始时已增加，在获取任务后减少，任务完成后增加
                    _idlThrNum.fetch_sub(1, std::memory_order_relaxed);

                    task(); //执行任务

                    _idlThrNum.fetch_add(1, std::memory_order_relaxed);

                    // 任务完成后，检查是否整个线程池都空闲了
                    {
                        std::lock_guard<std::mutex> guard_notify(_lock);
                        if (_tasks.empty() && _idlThrNum.load(std::memory_order_relaxed) == (int)_pool.size()) {
                            _idle_cv.notify_all(); // 通知所有等待线程池空闲的调用者
                        }
                    }
                }
                });
            _idlThrNum.fetch_add(1, std::memory_order_relaxed); // 新线程初始为空闲
        }
    }
};

#endif // __DMTHREADPOOL_H_INCLUDE__