// 09. 클래스, 인터페이스, 다형성
//
// 인터페이스: "무엇을 할 수 있는가"만 정하고 "어떻게 하는가"는 정하지 않는 클래스.
//             C++에서는 순수 가상 함수(= 0)만 가진 클래스로 만든다.
// 다형성:     같은 IMotor& 로 호출해도, 실제 객체 종류에 따라 다른 구현이 실행된다.
//
// 이게 왜 중요한가?
//   제어 로직(Controller)이 IMotor에만 의존하면, 실제 모터 대신 MockMotor를 끼워서
//   하드웨어 없이 테스트할 수 있다. (의존성 주입, Dependency Injection)

#include <algorithm>  // std::min, std::max
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ── 인터페이스 ──────────────────────────────────────────────
class IMotor {
public:
    // 가상 소멸자: IMotor* 로 지울 때 실제 타입(MockMotor 등)의 소멸자가 호출되게 한다.
    // 이게 없으면 unique_ptr<IMotor>로 MockMotor를 지울 때 정의되지 않은 동작이 된다.
    virtual ~IMotor() = default;

    virtual std::string name() const = 0;
    virtual void moveTo(double target) = 0;
    virtual void update() = 0;
    virtual double position() const = 0;
    virtual bool isArrived() const = 0;
};

// ── 구현 1: 한 번에 1.0씩 움직이는 가상 모터 ─────────────────
class MockMotor : public IMotor {
public:
    explicit MockMotor(std::string name) : name_(std::move(name)) {}

    // override: 부모의 가상 함수를 재정의한다는 표시. 오타가 나면 컴파일 에러로 잡아 준다.
    std::string name() const override { return name_; }
    void moveTo(double target) override { target_ = target; }
    void update() override {
        if (position_ < target_) position_ = std::min(position_ + 1.0, target_);
        if (position_ > target_) position_ = std::max(position_ - 1.0, target_);
    }
    double position() const override { return position_; }
    bool isArrived() const override { return position_ == target_; }

private:
    std::string name_;
    double position_ = 0.0;
    double target_ = 0.0;
};

// ── 구현 2: 즉시 도착하는 테스트용 모터 ──────────────────────
class InstantMotor final : public IMotor {  // final: 더 이상 상속 금지
public:
    std::string name() const override { return "Instant"; }
    void moveTo(double target) override { position_ = target; }
    void update() override {}
    double position() const override { return position_; }
    bool isArrived() const override { return true; }

private:
    double position_ = 0.0;
};

// ── 제어 로직: IMotor만 알고, 구체적인 모터 종류는 모른다 ────
class Controller {
public:
    // 모터를 소유하지 않고 참조로 빌려 쓴다. (소유는 main이 한다)
    explicit Controller(IMotor& motor) : motor_(motor) {}

    int runMove(double target) {
        motor_.moveTo(target);
        int ticks = 0;
        while (!motor_.isArrived() && ticks < 100) {  // 100 tick 넘으면 포기 (timeout)
            motor_.update();
            ++ticks;
        }
        return ticks;
    }

private:
    IMotor& motor_;
};

int main() {
    std::cout << "[같은 Controller, 다른 모터]\n";
    MockMotor mock("X축");
    InstantMotor instant;

    Controller c1(mock);
    Controller c2(instant);
    std::cout << "  " << mock.name() << ": " << c1.runMove(5.0) << " ticks\n";
    std::cout << "  " << instant.name() << ": " << c2.runMove(5.0) << " ticks\n";

    std::cout << "\n[인터페이스 포인터로 여러 모터 관리]\n";
    std::vector<std::unique_ptr<IMotor>> motors;
    motors.push_back(std::make_unique<MockMotor>("X축"));
    motors.push_back(std::make_unique<MockMotor>("Y축"));
    motors.push_back(std::make_unique<InstantMotor>());

    for (auto& m : motors) {
        m->moveTo(3.0);
        m->update();  // 가상 함수: 실제 타입의 update가 호출된다.
        std::cout << "  " << m->name() << " position = " << m->position() << '\n';
    }

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. IMotor를 상속해 "가끔 멈춰서 도착하지 않는" StuckMotor를 만들자.
//    Controller가 100 tick timeout으로 빠져나오는지 확인하자. (Fault injection의 시작)
// 2. MockMotor의 update 선언에서 override를 지우고, 이름을 updat로 오타 내 보자.
//    override가 있을 때와 없을 때 컴파일러 반응을 비교하자.
// 3. ISensor 인터페이스(read() 함수)를 만들고 MockSensor를 구현해 보자.
