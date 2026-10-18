// motor.h — 헤더 파일
//
// 헤더에는 "이런 클래스/함수가 있다"는 선언(declaration)을 둔다.
// 이 파일을 #include 하는 모든 .cpp는 Motor가 어떻게 생겼는지 알 수 있다.
// 실제 동작 코드(정의, definition)는 motor.cpp에 있다.

#pragma once  // 같은 헤더가 한 .cpp에 두 번 포함되는 것을 막는다.

class Motor {
public:
    // explicit: Motor m = 3; 같은 의도치 않은 자동 변환을 막는다.
    explicit Motor(int id);

    void moveTo(double target);  // 목표 위치 설정
    void update();               // 한 번 호출될 때마다 목표 쪽으로 한 걸음 이동

    // 뒤의 const: "이 함수는 객체 상태를 바꾸지 않는다"는 약속 (08에서 자세히)
    bool isArrived() const;
    double position() const;
    int id() const;

private:
    int id_;
    double position_ = 0.0;
    double target_ = 0.0;

    static constexpr double kStep = 1.0;  // 한 번 update에 이동하는 거리
};
