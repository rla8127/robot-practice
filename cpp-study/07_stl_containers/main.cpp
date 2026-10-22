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
//
// STL = Standard Template Library. C++이 기본으로 주는 "자료 담는 상자 + 자주 쓰는 함수" 모음.
// <T>, <K, V>의 꺾쇠 안에는 "무엇을 담을지" 타입을 적는다. 예) std::vector<double> = double을 담는 vector
//
// 자세한 설명은 같은 폴더의 README.md를 본다.

#include <algorithm>       // std::sort, std::find_if, std::remove_if
#include <array>           // std::array
#include <iostream>
#include <map>             // std::map
#include <string>
#include <unordered_map>   // std::unordered_map
#include <vector>          // std::vector

// 모터에 내릴 명령 하나. 이름과 우선순위(숫자가 작을수록 먼저 처리)를 가진다.
struct Command {
    std::string name;
    int priority;
};

int main() {
    // ─── vector: 길이가 늘었다 줄었다 하는 배열 ─────────────────
    std::cout << "[vector]\n";
    std::vector<double> positions = {3.0, 1.5, 7.2, 0.4};  // { } 안의 값 4개로 시작
    positions.push_back(5.0);                               // 맨 뒤에 하나 추가 → 5개 (Python의 append)
    std::cout << "  size = " << positions.size() << '\n';
    for (double p : positions) {  // 범위 기반 for
        // "positions 안의 원소를 앞에서부터 하나씩 p에 담아서 반복" (Python의 for p in positions)
        // p는 원소의 "복사본"이다. double처럼 작은 값은 복사해도 괜찮다.
        std::cout << "  " << p;
    }
    std::cout << '\n';
    // positions[10]은 범위 검사를 안 한다(정의되지 않은 동작).
    // positions.at(10)은 std::out_of_range 예외를 던진다.
    //   정의되지 않은 동작(undefined behavior) = 무슨 일이 일어날지 C++이 보장하지 않음.
    //   멀쩡해 보일 수도, 엉뚱한 값이 나올 수도, 죽을 수도 있다. 그래서 가장 위험하다.

    // ─── array: 크기가 처음부터 고정된 배열 ─────────────────────
    std::cout << "\n[array]\n";
    std::array<int, 2> axisIds = {0, 1};  // X, Y 두 축처럼 개수가 고정일 때
    // <int, 2> : int를 정확히 2개 담는다. 2는 빌드할 때 정해지는 숫자여야 한다. push_back이 없다.
    std::cout << "  축 개수 = " << axisIds.size() << '\n';

    // ─── sort + 람다: 내가 정한 기준으로 정렬 ─────────────────────
    std::cout << "\n[sort + 람다]\n";
    std::vector<Command> queue = {
        {"MoveTo", 2},  // 안쪽 { }는 Command 하나 = {name, priority}
        {"Stop", 0},  // 숫자가 작을수록 우선
        {"Home", 1},
    };
    // std::sort(시작, 끝, 비교 함수)
    //   queue.begin() : 첫 원소를 가리키는 "반복자(iterator)" — 위치 표시
    //   queue.end()   : 마지막 원소 "다음" 칸을 가리키는 반복자 — 끝 표시 (원소가 아님!)
    //   세 번째 인자   : "a가 b보다 앞에 와야 하면 true"를 돌려주는 함수. 여기서는 람다로 즉석에서 만든다.
    // 람다 [](const Command& a, const Command& b) { return ...; } 읽는 법
    //   []       : 캡처 목록. 바깥 변수를 쓰지 않으니 비어 있다.
    //   (...)    : 인자. 비교할 명령 두 개를 const 참조로 받는다. (복사 없이 읽기만, 02)
    //   { ... }  : 본문. a의 우선순위 숫자가 더 작으면 a가 앞.
    std::sort(queue.begin(), queue.end(),
              [](const Command& a, const Command& b) { return a.priority < b.priority; });
    for (const auto& cmd : queue) {
        // const auto& : 원소 타입(Command)은 컴파일러가 알아서 정하고(auto), 복사 없이 읽기만 한다(const &).
        //               Command에는 string이 들어 있으니 복사보다 참조가 낫다.
        std::cout << "  " << cmd.priority << ": " << cmd.name << '\n';
    }

    // ─── find_if: 조건에 맞는 첫 원소 찾기 ─────────────────────
    std::cout << "\n[find_if]\n";
    // 람다가 true를 돌려주는 "첫 번째" 원소의 위치(반복자)를 돌려준다.
    // auto it : 반복자 타입 이름은 아주 길어서(std::vector<Command>::iterator) auto로 받는다.
    auto it = std::find_if(queue.begin(), queue.end(),
                           [](const Command& c) { return c.name == "Home"; });
    if (it != queue.end()) {  // 못 찾으면 end()를 반환한다.
        // 반복자는 포인터처럼 쓴다. it->name = "it가 가리키는 원소의 name" (02의 ->)
        std::cout << "  찾음: " << it->name << '\n';
    }

    // ─── remove_if + erase: 조건에 맞는 원소 지우기 ─────────────────
    std::cout << "\n[remove_if + erase: 조건에 맞는 원소 지우기]\n";
    const double limit = 5.0;
    // 람다의 [limit]: 바깥 변수 limit을 복사해서 사용(캡처)한다.
    //   캡처가 없으면 람다 본문에서 limit이라는 이름을 쓸 수 없다. (람다는 별도의 작은 함수이기 때문)
    // 두 단계로 동작한다. (README 6절 그림 참고)
    //   ① std::remove_if : 남길 원소를 앞으로 당겨 모으고, "남길 원소의 끝" 위치를 돌려준다.
    //                      이름과 달리 실제로 지우지는 않는다! vector의 size는 그대로다.
    //   ② positions.erase(그 위치, end()) : 그 위치부터 끝까지를 진짜로 지운다. 여기서 size가 줄어든다.
    positions.erase(std::remove_if(positions.begin(), positions.end(),
                                   [limit](double p) { return p > limit; }),
                    positions.end());
    for (double p : positions) {
        std::cout << "  " << p;
    }
    std::cout << "  (5.0 초과 제거)\n";

    // ─── map: 키로 값을 찾는 사전 (키 순서로 정렬됨) ─────────────────
    std::cout << "\n[map: 정렬된 사전]\n";
    std::map<std::string, int> errorCount;  // 키 = 에러 이름(string), 값 = 횟수(int)
    errorCount["timeout"] += 1;  // 없는 키에 []를 쓰면 0으로 만들어진다.
    // 그래서 "처음 보는 에러면 0에서 시작해서 +1"이 한 줄로 된다. (Python dict라면 KeyError가 난다)
    errorCount["limit"] += 2;
    errorCount["timeout"] += 1;
    for (const auto& [key, count] : errorCount) {  // C++17 구조적 바인딩
        // map의 원소 하나는 (키, 값) 한 쌍(std::pair)이다.
        // [key, count]는 그 쌍을 두 이름으로 나눠 받는 문법이다. (Python의 for key, count in d.items())
        // 넣은 순서가 아니라 키의 "가나다(알파벳)순"으로 나온다. limit < timeout
        std::cout << "  " << key << " = " << count << '\n';
    }

    // ─── unordered_map: 순서 없는 사전 (조회가 보통 더 빠름) ───────────
    std::cout << "\n[unordered_map: 조회]\n";
    std::unordered_map<std::string, double> limits = {{"x_max", 100.0}, {"y_max", 50.0}};
    // if (준비문; 조건) — C++17 문법. found는 이 if 블록 안에서만 쓸 수 있다.
    //   limits.find("x_max") : 키를 찾으면 그 위치를, 없으면 end()를 돌려준다. 키를 새로 만들지 않는다.
    //   found->second        : 찾은 (키, 값) 쌍에서 값. 키는 found->first
    if (auto found = limits.find("x_max"); found != limits.end()) {
        std::cout << "  x_max = " << found->second << '\n';
    }
    // count(키) : 그 키가 몇 개 있나. map은 키가 겹치지 않으니 0 아니면 1이다.
    if (limits.count("z_max") == 0) {
        std::cout << "  z_max 없음 (find/count로 확인하면 키가 새로 생기지 않는다)\n";
    }
    // 주의: 여기서 limits["z_max"]라고 쓰면 z_max = 0.0이 "몰래" 새로 생긴다.
    //       장비에서 "Z축 최대 위치 = 0"이 되어 버리는 셈이다. 조회는 find / count로 한다.

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. queue에서 priority가 1 이하인 명령의 개수를 std::count_if로 세어 보자.
// 2. 센서 이름 → 최근 값 10개를 저장하는 std::map<std::string, std::vector<double>>을 만들고
//    각 센서의 평균을 출력해 보자.
// 3. 반복문 안에서 vector에 push_back 하면서 동시에 원소의 참조를 들고 있으면 왜 위험한가?
//    (키워드: iterator invalidation)
// → 단계별 실습은 README.md의 "9. 단계별 실습"을 따라간다.
