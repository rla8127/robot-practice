// 01. 빌드 과정 — 전처리 → 컴파일 → 링크 → 실행
//
// [1] 전처리 (preprocess): #include, #define 등을 처리해서 텍스트를 펼친다.
//       g++ -E main.cpp -o main.i      → main.i를 열어 보면 iostream 내용이 다 들어 있다.
// [2] 컴파일 (compile): .cpp 하나를 기계어 오브젝트 파일(.o)로 바꾼다.
//       g++ -std=c++17 -c main.cpp     → main.o
//       g++ -std=c++17 -c motor.cpp    → motor.o
//     이 단계에서 main.cpp는 Motor 함수가 "어딘가 있다"는 선언만 알면 된다.
// [3] 링크 (link): 여러 .o 파일을 합치고, 서로 호출하는 함수를 연결한다.
//       g++ main.o motor.o -o motor_demo
//     motor.o를 빼고 링크하면 "undefined reference to Motor::Motor(int)" 에러가 난다.
//     → 이게 바로 "링크 에러"다. 컴파일 에러와 구분할 수 있어야 한다.
// [4] 실행: ./motor_demo
//
// CMake는 위 과정을 대신 해 주는 도구다. (README의 빌드 방법 참고)

#include <iostream>

#include "motor.h"

int main() {
    Motor motor(1);
    motor.moveTo(3.5);

    std::cout << "Motor " << motor.id() << " 이동 시작\n";

    int tick = 0;
    while (!motor.isArrived()) {
        motor.update();
        ++tick;
        std::cout << "  tick " << tick << ": position = " << motor.position() << '\n';
    }

    if (!motor.moveTo(15.0)) {
        std::cout << "15.0은 범위 밖이라 거부됨. 현재 위치 = " << motor.position() << '\n';
    }

    std::cout << "도착! (" << tick << " ticks)\n";
    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 위 주석의 g++ 명령을 직접 하나씩 실행해 보고, 생기는 파일(.i, .o)을 확인하자.
// 2. 링크할 때 motor.o를 일부러 빼고 에러 메시지를 읽어 보자.
// 3. Motor에 최대 이동 범위(예: 0~10)를 추가하고, 범위를 넘는 moveTo는 무시하게 만들어 보자.
