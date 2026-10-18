// 08. const, std::optional, 오류 처리
//
// const       : "바꾸지 않는다"는 약속을 컴파일러가 검사하게 한다.
// optional<T> : "값이 있을 수도, 없을 수도 있다" (Python의 None 반환과 비슷)
// 오류 처리 방법:
//   1) 예외(exception) : 정말 예상하지 못한 상황, 정상 흐름에서 거의 안 생기는 오류
//   2) 오류 값 반환     : 자주 생길 수 있고 호출한 쪽이 반드시 처리해야 하는 오류
//      (제어 소프트웨어에서는 "명령 거부 + 사유"를 값으로 돌려주는 방식을 많이 쓴다)

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// ── const ───────────────────────────────────────────────────
class Axis {
public:
    explicit Axis(double maxPosition) : maxPosition_(maxPosition) {}

    // const 멤버 함수: 객체를 바꾸지 않는다. const Axis&로도 호출할 수 있다.
    double position() const { return position_; }
    double maxPosition() const { return maxPosition_; }

    // 비 const 멤버 함수: 객체를 바꾼다.
    void setPosition(double p) { position_ = p; }

private:
    double position_ = 0.0;
    const double maxPosition_;  // 생성 후 절대 바뀌지 않는 값
};

void report(const Axis& axis) {
    std::cout << "  position = " << axis.position() << " / max = " << axis.maxPosition() << '\n';
    // axis.setPosition(0);  // ← 컴파일 에러: const 객체에 비 const 함수 호출
}

// ── optional ────────────────────────────────────────────────
// 문자열을 숫자로 바꾼다. 실패하면 "값 없음"을 반환한다.
std::optional<double> parseNumber(const std::string& text) {
    try {
        std::size_t used = 0;
        double value = std::stod(text, &used);
        if (used != text.size()) {
            return std::nullopt;  // "12abc"처럼 뒤에 쓰레기가 붙은 경우
        }
        return value;
    } catch (const std::exception&) {
        return std::nullopt;  // 숫자로 시작하지도 않는 경우
    }
}

// ── 오류 값 반환 ────────────────────────────────────────────
enum class MoveError {
    None,
    OutOfRange,
    NotHomed,
};

const char* toString(MoveError e) {
    switch (e) {
        case MoveError::None:       return "None";
        case MoveError::OutOfRange: return "OutOfRange";
        case MoveError::NotHomed:   return "NotHomed";
    }
    return "Unknown";
}

// [[nodiscard]]: 반환값을 무시하면 컴파일러가 경고한다. 오류를 무시하는 실수를 막는다.
[[nodiscard]] MoveError tryMove(Axis& axis, double target, bool homed) {
    if (!homed) {
        return MoveError::NotHomed;
    }
    if (target < 0.0 || target > axis.maxPosition()) {
        return MoveError::OutOfRange;
    }
    axis.setPosition(target);
    return MoveError::None;
}

// ── 예외 ────────────────────────────────────────────────────
Axis createAxis(double maxPosition) {
    if (maxPosition <= 0.0) {
        // 설정 파일이 잘못된 경우처럼, 여기서 더 진행할 수 없는 상황
        throw std::invalid_argument("maxPosition must be positive");
    }
    return Axis(maxPosition);
}

int main() {
    std::cout << "[const]\n";
    Axis x(100.0);
    x.setPosition(10.0);
    report(x);

    std::cout << "\n[optional]\n";
    for (const std::string input : {"42.5", "abc", "12abc"}) {
        if (auto value = parseNumber(input)) {  // 값이 있으면 true
            std::cout << "  \"" << input << "\" → " << *value << '\n';
        } else {
            std::cout << "  \"" << input << "\" → 숫자가 아님\n";
        }
    }

    std::cout << "\n[오류 값 반환]\n";
    std::cout << "  homing 전 이동: " << toString(tryMove(x, 50.0, false)) << '\n';
    std::cout << "  범위 밖 이동:   " << toString(tryMove(x, 150.0, true)) << '\n';
    std::cout << "  정상 이동:      " << toString(tryMove(x, 50.0, true)) << '\n';
    report(x);

    std::cout << "\n[예외]\n";
    try {
        Axis bad = createAxis(-1.0);
        report(bad);  // 예외가 나면 이 줄은 실행되지 않는다.
    } catch (const std::invalid_argument& e) {
        std::cout << "  설정 오류: " << e.what() << '\n';
    }

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. tryMove(x, 50.0, true); 처럼 반환값을 버리는 줄을 추가하고 경고를 확인하자.
// 2. MoveError만으로는 "얼마나 범위를 벗어났는지" 알 수 없다.
//    struct MoveResult { MoveError error; std::string message; }; 를 반환하도록 바꿔 보자.
// 3. 각 상황에 예외/오류 값/optional 중 무엇이 맞는지 이유와 함께 골라 보자.
//    (a) 설정 파일이 없음  (b) 사용자가 범위 밖 좌표 입력  (c) 센서 목록에서 이름으로 검색
