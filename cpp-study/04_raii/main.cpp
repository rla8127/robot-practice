// 04. RAII (Resource Acquisition Is Initialization)
//
// "자원을 얻는 것은 생성자에서, 돌려주는 것은 소멸자에서."
// 03에서 본 것처럼 소멸자는 블록을 벗어날 때 "반드시" 호출된다.
// return으로 일찍 빠져나가도, 예외가 던져져도 마찬가지다.
// 그래서 자원(메모리, 파일, 락, 모터 전원 등)을 객체 수명에 묶으면 해제를 잊을 수 없다.
//
// 표준 라이브러리의 RAII 예: std::unique_ptr, std::vector, std::ofstream, std::lock_guard

#include <iostream>
#include <stdexcept>

// 가상의 하드웨어 API (C 스타일: 켜고 끄는 걸 사람이 직접 짝 맞춰야 함)
void powerOn(int axis)  { std::cout << "  [HW] axis " << axis << " 전원 ON\n"; }
void powerOff(int axis) { std::cout << "  [HW] axis " << axis << " 전원 OFF\n"; }

// RAII 래퍼: 생성되면 전원 ON, 소멸되면 전원 OFF
class MotorPowerGuard {
public:
    explicit MotorPowerGuard(int axis) : axis_(axis) {
        powerOn(axis_);
    }
    ~MotorPowerGuard() {
        powerOff(axis_);
    }

    // 복사를 금지한다. 복사본이 생기면 소멸자가 두 번 불려 전원을 두 번 끄게 된다.
    MotorPowerGuard(const MotorPowerGuard&) = delete;
    MotorPowerGuard& operator=(const MotorPowerGuard&) = delete;

private:
    int axis_;
};

// RAII 없이 짠 코드: 중간에 return이나 예외가 생기면 powerOff가 빠진다.
void runWithoutRaii(bool sensorError) {
    powerOn(1);
    if (sensorError) {
        std::cout << "  센서 오류! 일찍 return\n";
        return;  // ← powerOff(1)를 잊었다. 모터 전원이 켜진 채로 남는다.
    }
    std::cout << "  작업 수행\n";
    powerOff(1);
}

// RAII로 짠 코드: 어떤 경로로 나가든 전원이 꺼진다.
void runWithRaii(bool sensorError) {
    MotorPowerGuard guard(2);
    if (sensorError) {
        std::cout << "  센서 오류! 일찍 return\n";
        return;  // guard 소멸자가 자동으로 powerOff(2)
    }
    std::cout << "  작업 수행\n";
}  // 정상 종료 시에도 여기서 powerOff(2)

void runWithException() {
    MotorPowerGuard guard(3);
    std::cout << "  작업 중 예외 발생!\n";
    throw std::runtime_error("encoder timeout");
}  // 예외가 함수를 빠져나갈 때도 guard 소멸자가 호출된다 (stack unwinding)

int main() {
    std::cout << "[RAII 없음 + 오류]\n";
    runWithoutRaii(true);   // OFF가 출력되지 않는 것에 주목

    std::cout << "\n[RAII + 정상]\n";
    runWithRaii(false);

    std::cout << "\n[RAII + 오류]\n";
    runWithRaii(true);

    std::cout << "\n[RAII + 예외]\n";
    try {
        runWithException();
    } catch (const std::exception& e) {
        std::cout << "  catch: " << e.what() << '\n';
    }

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 실행 결과에서 runWithoutRaii가 전원을 끄지 않은 것을 확인하자.
// 2. 같은 아이디어로 "함수 실행 시간을 재는" ScopedTimer 클래스를 만들어 보자.
//    생성자에서 시작 시각을 저장하고, 소멸자에서 경과 시간을 출력한다. (<chrono> 사용)
// 3. 복사 금지(= delete) 줄을 지우고 guard를 복사해 보자. OFF가 몇 번 출력되는가?
