// 10. 스레드(thread)와 뮤텍스(mutex)
//
// 스레드: 한 프로그램 안에서 동시에 실행되는 실행 흐름.
//         예) 제어 루프 스레드 + 통신 스레드 + 로그 스레드
// 문제:   여러 스레드가 같은 데이터를 동시에 읽고 쓰면 결과가 망가진다. (data race)
// 해결:   std::mutex로 한 번에 한 스레드만 접근하게 한다.
//         std::lock_guard는 04의 RAII 그대로다. 생성 시 lock, 소멸 시 unlock.
//
// ⚠ data race는 C++에서 "정의되지 않은 동작(UB)"이다. 아래 unsafe 예시는
//   문제를 눈으로 보기 위한 시연용일 뿐, 실제 코드에서는 절대 이렇게 쓰면 안 된다.

#include <functional>  // std::ref
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int kThreads = 4;
constexpr int kIncrements = 100000;

// ── 보호 없음 (잘못된 예) ───────────────────────────────────
void unsafeIncrement(int& counter) {
    for (int i = 0; i < kIncrements; ++i) {
        ++counter;  // 읽기 → 더하기 → 쓰기 사이에 다른 스레드가 끼어들 수 있다.
    }
}

// ── mutex로 보호 (올바른 예) ────────────────────────────────
class SafeCounter {
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);  // 이 블록 동안 다른 스레드는 대기
        ++value_;
    }  // lock 소멸 → 자동 unlock

    int value() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

private:
    mutable std::mutex mutex_;  // mutable: const 함수(value) 안에서도 lock할 수 있게
    int value_ = 0;
};

int main() {
    const int expected = kThreads * kIncrements;

    std::cout << "[보호 없음]\n";
    {
        int counter = 0;
        std::vector<std::thread> threads;
        for (int i = 0; i < kThreads; ++i) {
            // 스레드 함수에 참조를 넘기려면 std::ref가 필요하다. (기본은 복사)
            threads.emplace_back(unsafeIncrement, std::ref(counter));
        }
        for (auto& t : threads) {
            t.join();  // 스레드가 끝날 때까지 기다린다.
        }
        std::cout << "  기대값 " << expected << ", 실제값 " << counter << '\n';
    }

    std::cout << "\n[mutex 사용]\n";
    {
        SafeCounter counter;
        std::vector<std::thread> threads;
        for (int i = 0; i < kThreads; ++i) {
            // 람다로 스레드 함수를 만들 수도 있다. [&]: 바깥 변수를 참조로 캡처
            threads.emplace_back([&counter] {
                for (int j = 0; j < kIncrements; ++j) {
                    counter.increment();
                }
            });
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "  기대값 " << expected << ", 실제값 " << counter.value() << '\n';
    }

    // join()도 detach()도 하지 않은 std::thread가 소멸되면 std::terminate로 프로그램이 죽는다.
    // C++20의 std::jthread는 소멸 시 자동으로 join한다.

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 여러 번 실행해서 "보호 없음"의 결과가 매번 다른지 확인하자.
// 2. join() 줄을 지우고 실행하면 어떻게 되는가?
// 3. mutex 두 개(A, B)를 스레드1은 A→B, 스레드2는 B→A 순서로 잠그면 무슨 일이 생기는가?
//    (Deadlock) std::scoped_lock lock(a, b); 로 어떻게 해결되는지 찾아보자.
// 4. 빌드 시 -fsanitize=thread 옵션을 붙여 "보호 없음"을 실행해 보자. (ThreadSanitizer)
