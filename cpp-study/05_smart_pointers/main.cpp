// 05. 스마트 포인터 — 소유권(ownership)을 코드로 표현하기
//
// 힙(heap)에 객체를 만들 때 new/delete를 직접 쓰면 delete를 잊거나 두 번 하기 쉽다.
// 스마트 포인터는 04의 RAII를 메모리에 적용한 것이다.
//
//   std::unique_ptr<T> : 소유자가 딱 한 명. 복사 불가, 이동(move)만 가능. 기본 선택.
//   std::shared_ptr<T> : 여러 명이 공동 소유. 마지막 소유자가 사라지면 해제.
//   std::weak_ptr<T>   : shared_ptr를 "소유하지 않고" 관찰만 한다.
//   T* / T&            : 소유하지 않는 관찰자. "빌려 쓰기"만 한다.
//
// 규칙: new/delete 대신 std::make_unique / std::make_shared를 쓴다.

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Sensor {
public:
    explicit Sensor(std::string name) : name_(std::move(name)) {
        std::cout << "  + Sensor(" << name_ << ")\n";
    }
    ~Sensor() {
        std::cout << "  - ~Sensor(" << name_ << ")\n";
    }
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// 관찰만 하는 함수는 소유권과 무관하게 참조(또는 포인터)로 받는다.
void printSensor(const Sensor& sensor) {
    std::cout << "  sensor: " << sensor.name() << '\n';
}

// unique_ptr를 "값으로" 받는 함수 = "소유권을 넘겨받겠다"는 뜻 (sink)
void takeOwnership(std::unique_ptr<Sensor> sensor) {
    std::cout << "  takeOwnership이 " << sensor->name() << "를 소유함\n";
}  // ← 여기서 sensor가 소멸

int main() {
    std::cout << "[unique_ptr 기본]\n";
    {
        auto s = std::make_unique<Sensor>("온도");
        printSensor(*s);  // *s: 가리키는 객체
    }  // delete를 안 써도 자동 해제

    std::cout << "\n[unique_ptr 소유권 이동]\n";
    {
        auto a = std::make_unique<Sensor>("근접");
        // auto b = a;          // ← 컴파일 에러: 복사 불가 (소유자는 한 명)
        auto b = std::move(a);  // 소유권을 b로 넘긴다.
        std::cout << "  a는 " << (a ? "소유 중" : "비어 있음(nullptr)") << '\n';
        std::cout << "  b는 " << (b ? "소유 중" : "비어 있음(nullptr)") << '\n';

        takeOwnership(std::move(b));
        std::cout << "  takeOwnership 호출 후 (이미 해제됨)\n";
    }

    std::cout << "\n[vector<unique_ptr>: 컨테이너가 소유]\n";
    {
        std::vector<std::unique_ptr<Sensor>> sensors;
        sensors.push_back(std::make_unique<Sensor>("리밋 스위치 L"));
        sensors.push_back(std::make_unique<Sensor>("리밋 스위치 R"));
        for (const auto& s : sensors) {
            printSensor(*s);
        }
    }  // vector가 사라지면 안의 객체들도 전부 해제

    std::cout << "\n[shared_ptr 공동 소유]\n";
    {
        std::shared_ptr<Sensor> owner1 = std::make_shared<Sensor>("카메라");
        std::cout << "  use_count = " << owner1.use_count() << '\n';  // 1
        {
            std::shared_ptr<Sensor> owner2 = owner1;  // 복사 가능
            std::cout << "  use_count = " << owner1.use_count() << '\n';  // 2
        }
        std::cout << "  use_count = " << owner1.use_count() << '\n';  // 1

        std::weak_ptr<Sensor> watcher = owner1;  // 소유하지 않으므로 count가 늘지 않는다.
        std::cout << "  weak_ptr 추가 후 use_count = " << owner1.use_count() << '\n';

        owner1.reset();  // 마지막 소유자가 놓으면 해제
        std::cout << "  watcher.expired() = " << std::boolalpha << watcher.expired() << '\n';
    }

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 주석 처리된 auto b = a; 를 풀고 컴파일 에러 메시지를 읽어 보자.
// 2. shared_ptr 두 개가 서로를 가리키면(순환 참조) 왜 해제되지 않는가?
//    struct Node { std::shared_ptr<Node> next; }; 로 실험하고, weak_ptr로 고쳐 보자.
// 3. "기본은 unique_ptr, 정말 공동 소유가 필요할 때만 shared_ptr"인 이유를 정리해 보자.
