# 05. 스마트 포인터 — 학습 노트

> 2주 차. 04의 RAII를 **메모리**에 적용한 것이 스마트 포인터다.
> 오늘의 핵심 단어는 **소유권(ownership)**: "이 객체를 마지막에 지울 책임이 누구에게 있나?"

---

## 1. 오늘의 목표

1. `unique_ptr`, `shared_ptr`, `weak_ptr`, 그냥 `T*`/`T&`를 **소유권** 관점에서 구분한다.
2. `std::move`로 `unique_ptr`의 소유권을 넘기고, 넘긴 뒤 원래 변수가 어떻게 되는지 설명한다.
3. `shared_ptr` 순환 참조가 왜 메모리를 새게 만드는지 직접 확인하고 `weak_ptr`로 고친다.

---

## 2. 왜 이걸 배워야 하나?

### 2-1. 지금까지는 왜 필요 없었나?

지금까지 만든 객체는 전부 **지역 변수**였다. 블록이 끝나면 알아서 사라졌다. (03)

```cpp
{
    Sensor s("온도");   // 블록 안에서만 산다
}                       // 여기서 자동 소멸
```

그런데 이렇게만은 안 되는 경우가 있다.

- 센서를 **함수 안에서 만들어서 밖으로** 돌려주고 싶다. 지역 변수면 함수가 끝날 때 사라진다.
- 설정 파일을 읽어 보고 나서야 센서가 **몇 개**인지, **어떤 종류**인지 알 수 있다.
- 센서 객체가 너무 커서 복사하지 않고 **한 곳에 두고** 여러 곳에서 쓰고 싶다.

이럴 때 객체를 **힙(heap)**에 만든다. 힙에 만든 객체는 블록과 상관없이 살아 있고, **누군가 지워 줘야** 사라진다.

### 2-2. new/delete의 문제

옛날 방식은 `new`로 만들고 `delete`로 지웠다.

```cpp
Sensor* s = new Sensor("온도");
...
delete s;
```

04의 `powerOn/powerOff`와 **똑같은 문제**가 생긴다.

| 실수 | 결과 |
|---|---|
| `delete`를 잊음 (일찍 return, 예외) | 메모리 누수 + 소멸자 안 불림 (03 해 보기 3) |
| `delete`를 두 번 함 | 프로그램 충돌 (04 해 보기 1의 "OFF 두 번"과 같은 상황) |
| `delete`한 뒤에 또 씀 | 이상한 값, 충돌 (댕글링 포인터) |
| `Sensor*`만 보고는 **내가 지워야 하는지** 알 수 없음 | 위 세 가지 실수의 원인 |

마지막 줄이 특히 중요하다. `Sensor*`라는 타입은 "주소"라는 것만 알려 줄 뿐,
**"이걸 지울 책임이 나한테 있나?"**는 알려 주지 않는다.

### 2-3. 스마트 포인터 = 메모리용 Guard + 소유권 표시

스마트 포인터는 이 두 문제를 한꺼번에 푼다.

1. **RAII**: 스마트 포인터가 사라지면 소멸자가 알아서 `delete`한다. (04와 같은 원리)
2. **소유권 표시**: 타입 이름만 보고 "누가 지울 책임이 있는지" 알 수 있다.

---

## 3. 네 가지 한눈에 보기

| 타입 | 소유권 | 복사 | 언제 지워지나 | 비유 |
|---|---|---|---|---|
| `std::unique_ptr<T>` | **혼자** 소유 | ❌ (이동만) | 소유자가 사라질 때 | 차 키가 **딱 하나**. 남에게 주면 나는 없다 |
| `std::shared_ptr<T>` | **여럿이** 공동 소유 | ⭕ | **마지막** 소유자가 사라질 때 | 공유 회의실. 마지막 사람이 나갈 때 불을 끈다 |
| `std::weak_ptr<T>` | 소유 **안 함** (관찰) | ⭕ | 영향 없음 | 회의실 앞에서 "아직 누가 쓰고 있나?" 확인만 함 |
| `T*`, `T&` (02) | 소유 **안 함** (빌림) | ⭕ | 영향 없음 | 잠깐 빌려 쓰는 사람. 지울 권한 없음 |

**선택 기준 (외워 두기):**
1. 기본은 **`unique_ptr`**.
2. 정말로 "주인이 여럿이고, 누가 마지막인지 모른다"일 때만 `shared_ptr`.
3. 함수가 객체를 **쓰기만** 하면, 스마트 포인터가 아니라 **`const T&` 또는 `T&`**로 받는다.

---

## 4. main.cpp 한 줄씩 읽기

### 4-1. 실험 재료: Sensor

```cpp
class Sensor {
public:
    explicit Sensor(std::string name) : name_(std::move(name)) {
        std::cout << "  + Sensor(" << name_ << ")\n";
    }
    ~Sensor() {
        std::cout << "  - ~Sensor(" << name_ << ")\n";
    }
    const std::string& name() const { return name_; }
private:
    std::string name_;
};
```

03의 `Tracer`와 같은 역할이다. `-` 줄이 출력되면 **메모리가 해제된 것**이다.
즉 오늘은 "`- ~Sensor`가 **언제, 몇 번** 나오나"만 잘 보면 된다.

### 4-2. unique_ptr 기본

```cpp
{
    auto s = std::make_unique<Sensor>("온도");
    printSensor(*s);
}  // delete를 안 써도 자동 해제
```

- `std::make_unique<Sensor>("온도")` : 힙에 `Sensor("온도")`를 만들고, 그걸 **소유하는** `unique_ptr`을 돌려준다.
  `new`를 직접 쓰지 않는다.
- `auto` : 타입을 컴파일러가 알아서 정하게 한다. 여기서는 `std::unique_ptr<Sensor>`.
- `*s` : 포인터가 **가리키는 객체 그 자체**. 02의 `*p`와 같다. `printSensor`는 `const Sensor&`를 받으니 객체를 넘긴다.
- `s->name()` : 02의 `->`와 같다. 스마트 포인터도 일반 포인터처럼 쓴다.
- 블록이 끝나면 `s`가 소멸 → 소멸자가 `delete` → `~Sensor(온도)` 출력.

```cpp
void printSensor(const Sensor& sensor);
```

이 함수가 `unique_ptr`이 아니라 **`const Sensor&`**를 받는 게 포인트다.
"나는 보기만 할 뿐, 지울 책임도 없고 주인이 누구든 상관없다"는 뜻이다. (3절 선택 기준 3번)

### 4-3. unique_ptr 소유권 이동

```cpp
auto a = std::make_unique<Sensor>("근접");
// auto b = a;          // ← 컴파일 에러: 복사 불가
auto b = std::move(a);  // 소유권을 b로 넘긴다.
```

**왜 복사가 안 될까?** 04의 `MotorPowerGuard`에서 복사를 막은 이유와 **완전히 같다.**
복사를 허용하면 `a`, `b` 둘 다 소멸할 때 같은 메모리를 `delete` → 두 번 해제 → 충돌.
그래서 `unique_ptr`도 안에 `= delete`가 들어 있다.

**대신 이동(move)은 된다.** 차 키를 **건네주는** 것이다.

```
이동 전:  a ──▶ [Sensor 근접]        b = (없음)
이동 후:  a = nullptr              b ──▶ [Sensor 근접]
```

- `std::move(a)`는 "a의 내용을 **가져가도 된다**"는 허락 표시다. 실제로 옮기는 건 `b`를 만드는 쪽이 한다.
- 옮긴 뒤 `a`는 **비어 있다(`nullptr`)**. 그래서 `a ? "소유 중" : "비어 있음"`이 `비어 있음`을 출력한다.
- 키는 여전히 **하나**다. 소멸할 때도 `b`만 `delete`한다. 두 번 해제가 일어나지 않는다.

> `if (a)`나 `a ? ... : ...`처럼 스마트 포인터를 조건에 쓰면 "**무언가를 소유하고 있나?**"를 묻는 것이다.

### 4-4. 소유권을 받는 함수 (sink)

```cpp
void takeOwnership(std::unique_ptr<Sensor> sensor) {
    std::cout << "  takeOwnership이 " << sensor->name() << "를 소유함\n";
}  // ← 여기서 sensor가 소멸

takeOwnership(std::move(b));
std::cout << "  takeOwnership 호출 후 (이미 해제됨)\n";
```

`unique_ptr`을 **값으로** 받는 함수는 "**소유권을 넘겨받겠다**"는 뜻이다. 이런 함수를 sink(싱크, 빨아들이는 곳)라고 부른다.

- 부르는 쪽은 `std::move(b)`를 **꼭 써야** 한다. 안 쓰면 복사가 되어야 하는데 복사는 금지라 컴파일 에러.
  → 덕분에 "아, 여기서 소유권을 넘기는구나"가 코드에 **눈에 보이게** 드러난다.
- 함수 안의 `sensor`는 지역 변수(인자)라서 함수가 끝날 때 소멸 → `~Sensor(근접)` 출력.
- 그래서 `~Sensor(근접)`이 `takeOwnership 호출 후`보다 **먼저** 나온다.

**함수 인자 타입으로 의도 읽기 (02의 표를 확장):**

| 인자 타입 | 의미 |
|---|---|
| `const Sensor&` | 보기만 한다 |
| `Sensor&` | 고친다. 하지만 지우지는 않는다 |
| `Sensor*` | 고친다. 없을 수도 있다. 지우지는 않는다 |
| `std::unique_ptr<Sensor>` | **소유권을 가져간다.** 이제 내가 지운다 |
| `std::shared_ptr<Sensor>` | 공동 주인 명단에 **나도 올린다** |

### 4-5. 컨테이너가 소유하기

```cpp
{
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<Sensor>("리밋 스위치 L"));
    sensors.push_back(std::make_unique<Sensor>("리밋 스위치 R"));
    for (const auto& s : sensors) {
        printSensor(*s);
    }
}  // vector가 사라지면 안의 객체들도 전부 해제
```

센서 **여러 개**를 관리하는 실전 패턴이다.

- `vector`가 `unique_ptr`들을 소유하고, 각 `unique_ptr`이 `Sensor`를 소유한다. **소유의 사슬**이다.
- `vector`가 사라지면 → 안의 `unique_ptr`들이 사라지고 → 각자 `Sensor`를 `delete`.
- `push_back(std::make_unique<...>(...))`에 `std::move`가 없는 이유: `make_unique`가 돌려준 건 **이름 없는 임시 값**이라, 컴파일러가 알아서 이동한다. (03의 임시 객체)
- `for (const auto& s : sensors)` : 각 원소를 **참조**로 꺼낸다. `auto s`(값)로 쓰면 `unique_ptr`을 **복사**하려 해서 컴파일 에러가 난다.

> **주의 — 소멸 순서:** 출력을 보면 L, R 순서로(**만든 순서대로**) 소멸한다. 03의 "역순" 규칙과 다르다!
> 03의 규칙은 **같은 블록의 변수들**에 대한 것이다. `vector` 안 원소의 소멸 순서는 C++ 표준이 정해 두지 않았고,
> GCC의 `vector`는 앞에서부터 지운다. 그러니 **vector 원소 사이의 소멸 순서에 의존하는 코드는 쓰지 않는다.**

### 4-6. shared_ptr 공동 소유

```cpp
std::shared_ptr<Sensor> owner1 = std::make_shared<Sensor>("카메라");
std::cout << "  use_count = " << owner1.use_count() << '\n';  // 1
{
    std::shared_ptr<Sensor> owner2 = owner1;  // 복사 가능
    std::cout << "  use_count = " << owner1.use_count() << '\n';  // 2
}
std::cout << "  use_count = " << owner1.use_count() << '\n';  // 1
```

`shared_ptr`은 **주인이 몇 명인지 세는 숫자(참조 카운트, reference count)**를 함께 들고 다닌다.

```
owner1 생성          카운트 1
owner2 = owner1      카운트 2   (복사 = 주인 명단에 한 명 추가)
owner2 블록 끝        카운트 1   (소멸 = 명단에서 한 명 빠짐. 아직 0이 아니니 해제 안 함)
owner1.reset()       카운트 0   → 이때 비로소 delete
```

**언제 쓰나?** 예를 들어 카메라 한 대를 **영상 저장 모듈**과 **물체 인식 모듈**이 같이 쓰는데,
둘 중 누가 먼저 끝날지 모를 때. 마지막으로 끝나는 쪽이 정리하면 된다.

**하지만 기본 선택이 아닌 이유:**
- 카운트를 세는 비용이 있다. (스레드 안전하게 세느라 `unique_ptr`보다 느리다)
- **누가 언제 지우는지** 코드만 보고 알기 어려워진다. 장비 코드에서는 "이 객체는 언제 사라지나?"가 분명한 게 좋다.
- 순환 참조(해 보기 3)라는 함정이 있다.

### 4-7. weak_ptr: 소유하지 않고 지켜보기

```cpp
std::weak_ptr<Sensor> watcher = owner1;  // count가 늘지 않는다.
std::cout << "  weak_ptr 추가 후 use_count = " << owner1.use_count() << '\n';  // 1

owner1.reset();  // 마지막 소유자가 놓으면 해제
std::cout << "  watcher.expired() = " << std::boolalpha << watcher.expired() << '\n';  // true
```

- `weak_ptr`은 주인 명단에 **이름을 올리지 않는다.** 그래서 `use_count`가 그대로 1이다.
- 그 대신 "**아직 살아 있나?**"는 물어볼 수 있다. `expired()`가 `true`면 이미 사라졌다는 뜻이다.
- `owner1.reset()` : "나는 이제 이 객체 안 쓴다." 명단에서 빠짐 → 카운트 0 → 해제.
- `std::boolalpha` : `bool`을 `1/0` 대신 `true/false`로 출력하게 하는 설정이다.

**02의 `T*`와 뭐가 다른가?** `T*`는 대상이 사라져도 **모른다**(댕글링). `weak_ptr`은 사라졌는지 **알 수 있다.**

실제로 객체를 쓰려면 `lock()`으로 잠깐 `shared_ptr`을 얻는다. (해 보기 4)

---

## 5. 실행하고 예상과 비교

**실행 전에** 각 구간에서 `- ~Sensor(...)`가 **어느 줄 다음에** 나올지 예상해 본다. 특히:

- `[unique_ptr 소유권 이동]`: `~Sensor(근접)`은 `takeOwnership 호출 후`보다 먼저? 나중?
- `[shared_ptr 공동 소유]`: `~Sensor(카메라)`는 `watcher.expired()`보다 먼저? 나중?

```bash
cd ~/robot-practice/cpp-study
cmake --build build --target 05_smart_pointers
./build/05_smart_pointers
```

<details>
<summary>실제 결과 (예상을 다 쓴 뒤에 펼친다)</summary>

```
[unique_ptr 기본]
  + Sensor(온도)
  sensor: 온도
  - ~Sensor(온도)

[unique_ptr 소유권 이동]
  + Sensor(근접)
  a는 비어 있음(nullptr)
  b는 소유 중
  takeOwnership이 근접를 소유함
  - ~Sensor(근접)
  takeOwnership 호출 후 (이미 해제됨)

[vector<unique_ptr>: 컨테이너가 소유]
  + Sensor(리밋 스위치 L)
  + Sensor(리밋 스위치 R)
  sensor: 리밋 스위치 L
  sensor: 리밋 스위치 R
  - ~Sensor(리밋 스위치 L)
  - ~Sensor(리밋 스위치 R)

[shared_ptr 공동 소유]
  + Sensor(카메라)
  use_count = 1
  use_count = 2
  use_count = 1
  weak_ptr 추가 후 use_count = 1
  - ~Sensor(카메라)
  watcher.expired() = true
```

</details>

**체크 포인트**
- `+`와 `-`의 개수가 **같다** (5개씩). 누수 없음. `new`/`delete`를 한 번도 안 썼는데도.
- `[unique_ptr 소유권 이동]` 구간에는 `+ Sensor(근접)`이 **한 번**만 있다. 이동은 새 Sensor를 만들지 않고 **주인만 바꾼다.**

---

## 6. 직접 해 보기

### 해 보기 1: 복사 금지 에러 읽기 (5분) — 연습 과제 1번

`// auto b = a;` 주석을 풀고 빌드한다. (아래 `auto b = std::move(a);`는 이름이 겹치니 잠깐 주석 처리)

```
error: use of deleted function 'std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&) [with _Tp = Sensor; ...]'
```

04 해 보기 1의 에러와 **모양이 같다.** `use of deleted function` + 복사 생성자.
`unique_ptr`도 결국 `MotorPowerGuard`처럼 복사를 `= delete`한 RAII 클래스라는 증거다. 확인했으면 되돌린다.

### 해 보기 2: 이동한 뒤에 쓰면? (10분)

`auto b = std::move(a);` 다음 줄에 추가하고 빌드·실행한다.

```cpp
std::cout << "  a의 이름: " << a->name() << '\n';
```

1. **컴파일은 된다.** 경고도 없다. (컴파일러는 실행 중에 a가 비었는지 모른다)
2. 실행하면 `Segmentation fault`로 죽는다. 02 해 보기 3(nullptr 검사 빼기)과 **같은 이유**다. 왜인지 설명해 본다.

**규칙:** `std::move(x)` 한 뒤의 `x`는 **다시 값을 넣기 전까지 쓰지 않는다.**
확인했으면 추가한 줄을 지운다.

### 해 보기 3: 순환 참조 (20분) — 연습 과제 2번

새 파일 `05_smart_pointers/practice.cpp`에 작성한다.

```cpp
#include <iostream>
#include <memory>
#include <string>

struct Node {
    explicit Node(std::string n) : name(std::move(n)) { std::cout << "  + " << name << '\n'; }
    ~Node() { std::cout << "  - " << name << '\n'; }

    std::string name;
    std::shared_ptr<Node> next;
};

int main() {
    {
        auto a = std::make_shared<Node>("A");
        auto b = std::make_shared<Node>("B");
        a->next = b;   // A가 B를 소유
        b->next = a;   // B가 A를 소유  ← 서로 붙잡음
        std::cout << "  a.use_count = " << a.use_count() << '\n';
    }
    std::cout << "블록 끝\n";
}
```

```bash
cd ~/robot-practice/cpp-study/05_smart_pointers
g++ -std=c++17 -Wall -Wextra practice.cpp -o practice && ./practice
```

**(1)** 실행 전 예상: `- A`, `- B`가 출력될까?

**(2)** 실행하면 `- A`, `- B`가 **하나도 안 나온다.** `use_count`가 2인 것을 단서로, 블록이 끝날 때 카운트가 어떻게 변하는지 따라가 본다.

```
블록 끝: 변수 a 소멸 → A의 카운트 2 → ___
         변수 b 소멸 → B의 카운트 2 → ___
         A가 지워지려면 B가 먼저 A를 놓아야 하고,
         B가 지워지려면 ___ 가 먼저 ___ 를 놓아야 한다.
```

서로가 서로를 붙잡고 있어서 **아무도 먼저 놓지 못한다.** 이게 **순환 참조(circular reference)**다.
`shared_ptr`만 쓰면 메모리 누수가 생기는 대표적인 경우다.

**(3)** 고치기: 둘 중 **한쪽만** "소유하지 않고 지켜보기"로 바꾼다.

```cpp
struct Node {
    ...
    std::shared_ptr<Node> next;
    std::______<Node> prev;   // 소유하지 않는 쪽
};
```

`b->next = a;` 줄을 `b->prev = a;`로 바꾸고 다시 실행한다. `- A`, `- B`가 나오면 성공.

> 로봇 예: `Robot`이 `Arm`들을 **소유**하고, 각 `Arm`은 자기 `Robot`을 **참조**해야 한다면?
> 부모 → 자식은 소유(`unique_ptr`/`shared_ptr`), 자식 → 부모는 **비소유**(`T*` 또는 `weak_ptr`)로 둔다.

### 해 보기 4: weak_ptr로 실제로 쓰기 (10분)

`main.cpp`의 `[shared_ptr 공동 소유]` 블록에서, `owner1.reset();` **바로 앞과 뒤**에 아래를 각각 넣어 본다.

```cpp
if (auto locked = watcher.lock()) {
    std::cout << "  아직 살아 있음: " << locked->name() << '\n';
} else {
    std::cout << "  이미 사라짐\n";
}
```

1. `reset()` 앞에서는 어떤 줄이, 뒤에서는 어떤 줄이 출력되나?
2. `watcher.lock()`은 살아 있으면 `shared_ptr`을, 사라졌으면 **빈** `shared_ptr`을 돌려준다.
   `if (auto locked = ...)`는 "받아 보고, 비어 있지 않으면" 이라는 뜻이다.
3. (생각해 보기) `locked`가 살아 있는 동안 `use_count`는 몇일까? 직접 출력해서 확인한다.

### 해 보기 5: 소유권 설계해 보기 (15분) — 연습 과제 3번

코드를 쓰지 말고 **표만** 채운다. 각 관계에 무엇을 쓸지, 이유는 한 줄로.

| 관계 | 고를 것 (`unique_ptr` / `shared_ptr` / `weak_ptr` / `T&`·`T*`) | 이유 |
|---|---|---|
| `Robot`이 자기 바퀴 모터 2개를 가짐 | | |
| 로그 출력 함수가 모터 상태를 받아서 출력만 함 | | |
| 카메라 1대를 녹화 모듈과 인식 모듈이 같이 쓰고, 누가 먼저 끝날지 모름 | | |
| 모니터링 화면이 카메라를 지켜보되, 카메라 수명에는 영향을 주면 안 됨 | | |
| 설정을 읽고 만든 센서를 `SensorManager`에게 **넘겨줌** | | |

마지막에 "**기본은 unique_ptr, 정말 공동 소유가 필요할 때만 shared_ptr**"인 이유를 두 문장으로 정리한다.

---

## 7. 확인 질문 (한 줄씩)

1. `auto b = a;`는 안 되는데 `auto b = std::move(a);`는 되는 이유는? 이동 후 `a`는 어떤 상태인가?
2. `printSensor`가 `std::unique_ptr<Sensor>`가 아니라 `const Sensor&`를 받는 이유는?
3. `shared_ptr` 두 개가 서로를 가리키면 왜 해제되지 않나? 어떻게 고치나?

---

## 8. 내 답과 메모

> 아래에 직접 적는다.

- 5절 소멸 시점 예상에서 틀린 곳:
- 질문 1 답:
- 질문 2 답:
- 질문 3 답:
- 해 보기 2가 죽는 이유:
- 해 보기 4의 locked 살아 있는 동안 use_count:
- 해 보기 5 표:
- 막혔던 곳 / 에러 메시지:
- 새로 이해한 것:

---

## 9. 다음 장 연결

오늘 `std::move`로 `unique_ptr`의 소유권을 **옮겼다.** 복사는 막고 이동만 허용한 덕분에 "주인은 항상 한 명"이 지켜졌다.

그럼 04의 `MotorPowerGuard`도 이동할 수 있게 만들 수 있을까? `std::move`는 실제로 무슨 일을 할까?
06 복사 vs 이동에서 **복사 생성자 / 이동 생성자**를 직접 만들어 보며 답한다.

---

## 10. 질문과 답

> 공부하다 생긴 질문을 Claude에게 물어보고, 답을 여기에 정리한다.
