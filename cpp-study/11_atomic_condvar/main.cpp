// 11. atomic과 condition_variable — 제어 루프와 명령 큐
//
// std::atomic<T>           : 단순한 값(bool, int 등)을 lock 없이 안전하게 읽고 쓴다.
//                            예) "정지해라" 플래그
// std::condition_variable  : "조건이 될 때까지 잠들어 기다리기".
//                            바쁘게 반복 확인(busy-wait)하지 않아서 CPU를 낭비하지 않는다.
//                            예) 명령이 들어올 때까지 대기하는 작업 스레드
//
// 아래 예제 구조:
//   [main 스레드]  ── 명령 push ──▶  [CommandQueue]  ── pop ──▶  [worker 스레드]
//   [monitor 스레드] : stop 플래그가 켜질 때까지 주기적으로 상태 출력

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

using namespace std::chrono_literals;  // 100ms 같은 표기를 쓸 수 있게

// 스레드 안전한 명령 큐
class CommandQueue {
public:
    void push(std::string command) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(command));
        }
        cv_.notify_one();  // 기다리는 스레드 하나를 깨운다.
    }

    // 명령이 올 때까지 기다린다. close()되고 큐가 비면 nullopt를 반환한다.
    std::optional<std::string> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        // wait는 (1) lock을 풀고 잠들었다가 (2) 깨어나면 다시 lock을 잡고 조건을 검사한다.
        // 조건을 꼭 함께 넘겨야 한다. 이유 없이 깨어나는 경우(spurious wakeup)가 있기 때문이다.
        cv_.wait(lock, [this] { return !queue_.empty() || closed_; });

        if (queue_.empty()) {
            return std::nullopt;  // closed_이고 남은 명령도 없음
        }
        std::string command = std::move(queue_.front());
        queue_.pop();
        return command;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();  // 기다리는 모든 스레드를 깨운다.
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
    bool closed_ = false;
};

int main() {
    CommandQueue commands;
    std::atomic<bool> stopMonitor{false};
    std::atomic<int> processedCount{0};

    // 작업 스레드: 큐에서 명령을 꺼내 처리한다.
    std::thread worker([&] {
        while (auto command = commands.pop()) {
            std::cout << "  [worker] 실행: " << *command << '\n';
            std::this_thread::sleep_for(150ms);  // 명령 수행에 시간이 걸린다고 가정
            ++processedCount;
        }
        std::cout << "  [worker] 큐가 닫혀서 종료\n";
    });

    // 감시 스레드: 주기적으로 상태를 출력한다.
    std::thread monitor([&] {
        while (!stopMonitor.load()) {
            std::cout << "  [monitor] 처리한 명령 수 = " << processedCount.load() << '\n';
            std::this_thread::sleep_for(200ms);
        }
        std::cout << "  [monitor] 종료\n";
    });

    // main 스레드: 명령을 보낸다.
    for (const char* cmd : {"Initialize", "Home", "MoveTo(10, 20)", "Stop"}) {
        std::cout << "[main] 명령 전송: " << cmd << '\n';
        commands.push(cmd);
        std::this_thread::sleep_for(50ms);
    }

    // 종료 순서가 중요하다: 큐를 닫고 → worker가 남은 명령을 다 처리하길 기다리고 → monitor를 멈춘다.
    commands.close();
    worker.join();
    stopMonitor = true;
    monitor.join();

    std::cout << "[main] 총 " << processedCount.load() << "개 명령 처리 완료\n";
    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 출력이 가끔 한 줄에 섞여 나올 수 있다. 왜 그런가? std::cout 출력도 mutex로 보호해 보자.
// 2. pop()에 timeout을 추가하자: cv_.wait_for(lock, 1s, 조건)을 써서
//    1초 동안 명령이 없으면 "idle" 로그를 남기게 만들어 보자.
// 3. "Stop" 명령은 큐 순서를 무시하고 즉시 처리되어야 한다면 어떻게 설계하겠는가?
//    (제어 소프트웨어에서 매우 중요한 질문이다. 정답을 코드로 만들기 전에 먼저 글로 써 보자.)
