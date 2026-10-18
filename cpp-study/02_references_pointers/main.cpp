// 02. 참조(reference)와 포인터(pointer)
//
// Python은 모든 변수가 "객체를 가리키는 이름"이라 신경 쓸 일이 적다.
// C++은 값을 복사할지, 원본을 가리킬지를 직접 고른다.
//
//   T       값(value)    : 복사본을 만든다. 원본과 무관하다.
//   T&      참조          : 원본의 다른 이름. 반드시 무언가를 가리키고, 바꿀 수 없다.
//   const T& const 참조   : 원본을 읽기만 한다. 큰 객체를 복사 없이 넘길 때 기본 선택.
//   T*      포인터        : 주소를 담는 변수. nullptr(아무것도 안 가리킴)일 수 있다.

#include <iostream>
#include <string>

struct MotorState {
    std::string name;
    double position = 0.0;
};

// 값으로 받기: 복사본을 바꾸므로 호출한 쪽의 원본은 그대로다.
void moveByValue(MotorState state) {
    state.position += 10.0;
}

// 참조로 받기: 원본을 직접 바꾼다.
void moveByReference(MotorState& state) {
    state.position += 10.0;
}

// const 참조로 받기: 복사 없이 읽기만 한다. 수정하려 하면 컴파일 에러.
void print(const MotorState& state) {
    std::cout << "  " << state.name << " position = " << state.position << '\n';
    // state.position = 0;  // ← 주석을 풀면 컴파일 에러
}

// 포인터로 받기: "없을 수도 있음"을 표현할 수 있다. 대신 nullptr 확인이 필요하다.
void moveIfExists(MotorState* state) {
    if (state == nullptr) {
        std::cout << "  모터가 없어서 이동하지 않음\n";
        return;
    }
    state->position += 10.0;  // 포인터는 -> 로 멤버에 접근한다.
}

int main() {
    MotorState x{"X축", 0.0};

    std::cout << "[값으로 전달]\n";
    moveByValue(x);
    print(x);  // 0 — 원본은 안 바뀜

    std::cout << "[참조로 전달]\n";
    moveByReference(x);
    print(x);  // 10

    std::cout << "[포인터로 전달]\n";
    moveIfExists(&x);       // &x: x의 주소
    print(x);               // 20
    moveIfExists(nullptr);  // 안전하게 처리됨

    std::cout << "[주소 확인]\n";
    MotorState& alias = x;  // alias는 x의 다른 이름
    MotorState* ptr = &x;
    std::cout << "  &x     = " << &x << '\n';
    std::cout << "  &alias = " << &alias << "  (x와 같은 주소)\n";
    std::cout << "  ptr    = " << ptr << "  (x와 같은 주소)\n";
    std::cout << "  &ptr   = " << &ptr << "  (포인터 변수 자체의 주소는 다르다)\n";

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. swap(int& a, int& b) 함수를 직접 만들어 보자. 값으로 받으면 왜 동작하지 않는가?
// 2. 함수 안의 지역 변수를 참조로 반환하면(int& bad() { int v = 1; return v; }) 무슨 일이 생기는가?
//    컴파일러 경고를 읽어 보자. (힌트: 03_object_lifetime)
// 3. 언제 const T&, T&, T*를 고르는지 각각 한 문장으로 정리해 보자.
