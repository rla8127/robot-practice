// 03. 객체 수명(lifetime) — 언제 만들어지고 언제 사라지는가
//
// C++ 객체는 "만들어지는 순간"에 생성자가, "사라지는 순간"에 소멸자가 반드시 호출된다.
// Python의 GC처럼 "언젠가" 지워지는 게 아니라, 시점이 코드에 정확히 정해져 있다.
//
//   - 지역 변수: 선언된 { } 블록이 끝날 때 소멸
//   - 같은 블록 안의 여러 객체: 생성된 순서의 "역순"으로 소멸
//   - 클래스 멤버: 선언된 순서대로 생성, 역순으로 소멸
//   - 전역 변수: main 전에 생성, main 이후에 소멸
//
// 실행해서 출력 순서를 보며 위 규칙을 확인하자.

#include <iostream>
#include <string>

// 생성/소멸 시점을 출력하는 추적용 클래스
class Tracer {
public:
    explicit Tracer(std::string name) : name_(std::move(name)) {
        std::cout << "  + 생성: " << name_ << '\n';
    }
    ~Tracer() {
        std::cout << "  - 소멸: " << name_ << '\n';
    }

private:
    std::string name_;
};

// 멤버의 생성/소멸 순서를 확인하기 위한 클래스
class Robot {
public:
    Robot() : motor_("Robot::motor_"), sensor_("Robot::sensor_") {
        std::cout << "  (Robot 생성자 본문 실행)\n";
    }
    ~Robot() {
        std::cout << "  (Robot 소멸자 본문 실행)\n";
    }

private:
    // 초기화 리스트에 쓴 순서가 아니라, 여기 선언된 순서대로 생성된다.
    Tracer motor_;
    Tracer sensor_;
};

Tracer g_global("전역 객체");

int main() {
    std::cout << "[main 시작]\n";

    Tracer a("a");
    {
        std::cout << "[안쪽 블록 시작]\n";
        Tracer b("b");
        Tracer c("c");
        std::cout << "[안쪽 블록 끝]\n";
    }  // ← 여기서 c, b 순서로 소멸 (역순)

    std::cout << "[Robot 생성]\n";
    {
        Robot robot;
    }  // ← 소멸자 본문 → sensor_ → motor_ 순서

    std::cout << "[임시 객체]\n";
    Tracer("임시");  // 이름 없는 임시 객체는 이 문장(;)이 끝나자마자 소멸
    std::cout << "  (임시 객체 다음 줄)\n";

    std::cout << "[main 끝]\n";
    return 0;
}  // ← a 소멸, 그 다음 main이 끝난 뒤 전역 객체 소멸

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 실행 전에 출력 순서를 종이에 먼저 예상해 보고, 실제 결과와 비교하자.
// 2. Robot의 초기화 리스트 순서를 sensor_, motor_로 바꿔도 생성 순서가 같은지 확인하자.
//    (-Wall이 켜져 있으면 -Wreorder 경고가 뜬다. 왜일까?)
// 3. Tracer를 new로 만들고 delete를 잊으면 어떻게 되는가? 소멸자 출력이 나오는지 보자.
