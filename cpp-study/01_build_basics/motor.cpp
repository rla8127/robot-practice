// motor.cpp — 소스 파일
//
// motor.h에서 선언한 함수들을 여기서 정의한다.
// 이 파일은 따로 컴파일되어 motor.o(오브젝트 파일)가 되고,
// 나중에 링커가 main.o와 합쳐서 실행 파일을 만든다.

#include "motor.h"

#include <cmath>  // std::abs

// 멤버 초기화 리스트(: id_(id))로 멤버를 초기화한다.
// 생성자 본문 안에서 대입하는 것보다 이 방식이 권장된다.
Motor::Motor(int id) : id_(id) {}

bool Motor::moveTo(double target) {
    if (target < 0.0 || target > 10.0) {
        return false;
    }
    target_ = target;
    return true;
}

void Motor::update() {
    const double diff = target_ - position_;
    if (std::abs(diff) <= kStep) {
        position_ = target_;  // 한 걸음 이내면 바로 도착
    } else {
        position_ += (diff > 0) ? kStep : -kStep;
    }
}

bool Motor::isArrived() const {
    return position_ == target_;
}

double Motor::position() const {
    return position_;
}

int Motor::id() const {
    return id_;
}
