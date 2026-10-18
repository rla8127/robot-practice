// 07. STL 컨테이너와 알고리즘
//
// Python의 list / dict / sort / filter에 해당하는 것들이다.
//
//   std::vector<T>              : 가변 길이 배열 (Python list). 기본 선택.
//   std::array<T, N>            : 크기가 고정된 배열.
//   std::map<K, V>              : 키 순서로 정렬된 사전.
//   std::unordered_map<K, V>    : 해시 사전 (Python dict). 순서 없음, 보통 더 빠름.
//   <algorithm>                 : sort, find_if, count_if, remove_if ...
//   람다(lambda)                : [캡처](인자) { 본문 } — 이름 없는 함수

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct Command {
    std::string name;
    int priority;
};

int main() {
    std::cout << "[vector]\n";
    std::vector<double> positions = {3.0, 1.5, 7.2, 0.4};
    positions.push_back(5.0);
    std::cout << "  size = " << positions.size() << '\n';
    for (double p : positions) {  // 범위 기반 for
        std::cout << "  " << p;
    }
    std::cout << '\n';
    // positions[10]은 범위 검사를 안 한다(정의되지 않은 동작).
    // positions.at(10)은 std::out_of_range 예외를 던진다.

    std::cout << "\n[array]\n";
    std::array<int, 2> axisIds = {0, 1};  // X, Y 두 축처럼 개수가 고정일 때
    std::cout << "  축 개수 = " << axisIds.size() << '\n';

    std::cout << "\n[sort + 람다]\n";
    std::vector<Command> queue = {
        {"MoveTo", 2},
        {"Stop", 0},  // 숫자가 작을수록 우선
        {"Home", 1},
    };
    std::sort(queue.begin(), queue.end(),
              [](const Command& a, const Command& b) { return a.priority < b.priority; });
    for (const auto& cmd : queue) {
        std::cout << "  " << cmd.priority << ": " << cmd.name << '\n';
    }

    std::cout << "\n[find_if]\n";
    auto it = std::find_if(queue.begin(), queue.end(),
                           [](const Command& c) { return c.name == "Home"; });
    if (it != queue.end()) {  // 못 찾으면 end()를 반환한다.
        std::cout << "  찾음: " << it->name << '\n';
    }

    std::cout << "\n[remove_if + erase: 조건에 맞는 원소 지우기]\n";
    const double limit = 5.0;
    // 람다의 [limit]: 바깥 변수 limit을 복사해서 사용(캡처)한다.
    positions.erase(std::remove_if(positions.begin(), positions.end(),
                                   [limit](double p) { return p > limit; }),
                    positions.end());
    for (double p : positions) {
        std::cout << "  " << p;
    }
    std::cout << "  (5.0 초과 제거)\n";

    std::cout << "\n[map: 정렬된 사전]\n";
    std::map<std::string, int> errorCount;
    errorCount["timeout"] += 1;  // 없는 키에 []를 쓰면 0으로 만들어진다.
    errorCount["limit"] += 2;
    errorCount["timeout"] += 1;
    for (const auto& [key, count] : errorCount) {  // C++17 구조적 바인딩
        std::cout << "  " << key << " = " << count << '\n';
    }

    std::cout << "\n[unordered_map: 조회]\n";
    std::unordered_map<std::string, double> limits = {{"x_max", 100.0}, {"y_max", 50.0}};
    if (auto found = limits.find("x_max"); found != limits.end()) {
        std::cout << "  x_max = " << found->second << '\n';
    }
    if (limits.count("z_max") == 0) {
        std::cout << "  z_max 없음 (find/count로 확인하면 키가 새로 생기지 않는다)\n";
    }

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. queue에서 priority가 1 이하인 명령의 개수를 std::count_if로 세어 보자.
// 2. 센서 이름 → 최근 값 10개를 저장하는 std::map<std::string, std::vector<double>>을 만들고
//    각 센서의 평균을 출력해 보자.
// 3. 반복문 안에서 vector에 push_back 하면서 동시에 원소의 참조를 들고 있으면 왜 위험한가?
//    (키워드: iterator invalidation)
