# 02. 값 · 참조 · 포인터 — 학습 노트

> 1주 차. 함수에 모터 상태를 넘기는 **네 가지 방법**을 비교한다.
> "복사본을 넘길까, 원본을 넘길까, 원본이 없을 수도 있을까?"를 스스로 고를 수 있게 되는 것이 목표다.

---

## 1. 오늘의 목표

1. 값(`T`), 참조(`T&`), const 참조(`const T&`), 포인터(`T*`)의 차이를 **내 말로** 설명한다.
2. 함수 인자를 보고 "이 함수가 원본을 바꿀 수 있나?"를 바로 판단한다.
3. 포인터를 쓸 때 왜 `nullptr` 검사가 필요한지 직접 확인한다.

---

## 2. 왜 이걸 배워야 하나?

Python에서는 함수에 리스트를 넘기면 원본이 바뀌고, 숫자를 넘기면 안 바뀐다.
언어가 알아서 정해 주니까 크게 고민할 일이 없었다.

C++은 **내가 직접 고른다.** 그리고 잘못 고르면 이런 일이 생긴다.

- 모터 위치를 갱신하는 함수인데 **복사본**을 받았다 → 함수는 열심히 움직였는데 진짜 모터 상태는 그대로.
- 상태를 보기만 하는 함수인데 **원본**을 받았다 → 실수 한 줄로 모터 상태가 바뀌어도 컴파일러가 못 막아 준다.
- "센서가 장착 안 됐을 수도 있다"를 표현해야 하는데 방법이 없다 → 포인터가 필요한 자리.

---

## 3. 네 가지 방법 한눈에 보기

| 쓰는 법 | 이름 | 원본을 바꿀 수 있나 | 복사가 일어나나 | "없음"을 표현할 수 있나 |
|---|---|---|---|---|
| `MotorState s` | 값 | ❌ (복사본만 바뀜) | ⭕ 통째로 복사 | ❌ |
| `MotorState& s` | 참조 | ⭕ | ❌ | ❌ |
| `const MotorState& s` | const 참조 | ❌ (읽기만) | ❌ | ❌ |
| `MotorState* s` | 포인터 | ⭕ | ❌ (주소만 복사) | ⭕ `nullptr` |

01에서 배운 비유를 이어 가면:

- **값** = 설계도 **복사본**을 준다. 복사본을 고쳐도 진짜 모터는 그대로.
- **참조** = 진짜 모터의 **조종기**를 준다.
- **const 참조** = 진짜 모터의 **상태 모니터**를 준다. 보기만 가능.
- **포인터** = 모터가 놓인 **자리 번호(주소)**가 적힌 쪽지를 준다. 쪽지가 비어 있을(`nullptr`) 수도 있으니 열어 보기 전에 확인해야 한다.

---

## 4. main.cpp 한 줄씩 읽기

### 4-1. 실험 재료: MotorState

```cpp
struct MotorState {
    std::string name;
    double position = 0.0;
};
```

이름과 위치만 가진 아주 단순한 상태 묶음이다.
`struct`는 `class`와 거의 같은데, 기본이 `public`이라 멤버에 바로 접근할 수 있다.
"데이터만 묶어 두는 상자"에는 보통 `struct`를 쓴다.

```cpp
MotorState x{"X축", 0.0};
```

`{ }` 안에 멤버 순서대로 값을 넣어 만든다. name = "X축", position = 0.0.

### 4-2. 값으로 받기

```cpp
void moveByValue(MotorState state) {
    state.position += 10.0;
}
```

함수가 불릴 때 `x`가 **통째로 복사**되어 `state`라는 새 변수가 생긴다.
`state`를 10 움직여도, 함수가 끝나면 `state`는 사라진다. `x`는 손도 안 댔다.

```
main의 x      [X축, 0]      ← 그대로
함수의 state  [X축, 0 → 10] ← 함수 끝나면 사라짐
```

### 4-3. 참조로 받기

```cpp
void moveByReference(MotorState& state) {
    state.position += 10.0;
}
```

`state`는 `x`의 **다른 이름(별명)**이다. 새 변수가 생기지 않는다.
`state`를 움직이면 곧 `x`를 움직인 것이다.

```
main의 x ──┬── [X축, 0 → 10]
함수의 state ┘   (같은 것을 두 이름으로 부름)
```

### 4-4. const 참조로 받기

```cpp
void print(const MotorState& state) {
    std::cout << "  " << state.name << " position = " << state.position << '\n';
    // state.position = 0;  // ← 주석을 풀면 컴파일 에러
}
```

원본을 복사 없이 받되, `const`가 붙어서 **읽기만** 할 수 있다.
`std::string`처럼 복사 비용이 있는 걸 담은 구조체를 넘길 때 **기본 선택**이다.

### 4-5. 포인터로 받기

```cpp
void moveIfExists(MotorState* state) {
    if (state == nullptr) {
        std::cout << "  모터가 없어서 이동하지 않음\n";
        return;
    }
    state->position += 10.0;
}
```

- `MotorState*` = "MotorState가 있는 **주소**를 담는 변수".
- `nullptr` = "아무것도 가리키지 않음". 참조는 이런 상태가 될 수 없지만 포인터는 될 수 있다.
- `state->position` = "state가 가리키는 곳에 가서 position을 꺼내라". `(*state).position`의 줄임말이다.

부르는 쪽에서는 주소를 꺼내서 넘긴다.

```cpp
moveIfExists(&x);       // &x : x의 주소
moveIfExists(nullptr);  // "모터 없음"
```

### 4-6. 기호 정리: `&`와 `*`는 자리에 따라 뜻이 다르다

01의 Q3에서 본 내용을 다시 정리한다. 가장 헷갈리는 부분이라 표로 외워 두자.

| 코드 | 위치 | 뜻 |
|---|---|---|
| `MotorState& r = x;` | **타입** 옆 `&` | 참조 선언 (r은 x의 별명) |
| `&x` | **변수** 앞 `&` | x의 주소를 꺼낸다 |
| `MotorState* p = &x;` | **타입** 옆 `*` | 포인터 선언 (p는 주소를 담는 변수) |
| `*p` | **변수** 앞 `*` | p가 가리키는 곳의 값 (역참조) |
| `p->position` | | `(*p).position`의 줄임 |

**요령:** 선언문에서 타입 옆에 붙으면 "종류", 식 안에서 변수 앞에 붙으면 "동작".

### 4-7. 주소 찍어 보기

```cpp
MotorState& alias = x;
MotorState* ptr = &x;
std::cout << "  &x     = " << &x << '\n';
std::cout << "  &alias = " << &alias << '\n';
std::cout << "  ptr    = " << ptr << '\n';
std::cout << "  &ptr   = " << &ptr << '\n';
```

- `&x`와 `&alias`가 **같다** → 참조는 새 변수가 아니라 같은 것의 다른 이름이라는 증거.
- `ptr`의 값도 같다 → ptr 안에 x의 주소가 들어 있다.
- `&ptr`만 **다르다** → 포인터는 그 자체로 **별도의 변수**다. 메모리에 자기 자리가 따로 있다.

```
메모리
  0x...5f8  [ x: "X축", 20 ]   ← x, alias 둘 다 여기
  0x...5f0  [ ptr: 0x...5f8 ]  ← ptr 자기 자리. 안에 x의 주소가 적혀 있음
```

---

## 5. 실행하고 예상과 비교

**실행하기 전에** 아래 빈칸을 먼저 채워 본다.

```
[값으로 전달]
  X축 position = ___
[참조로 전달]
  X축 position = ___
[포인터로 전달]
  X축 position = ___
  ___________________
```

그다음 실행한다.

```bash
cd ~/robot-practice/cpp-study
cmake --build build --target 02_references_pointers
./build/02_references_pointers
```

실제 결과 (주소 숫자는 실행할 때마다, 컴퓨터마다 다르다):

```
[값으로 전달]
  X축 position = 0
[참조로 전달]
  X축 position = 10
[포인터로 전달]
  X축 position = 20
  모터가 없어서 이동하지 않음
[주소 확인]
  &x     = 0xfffffe8405f8
  &alias = 0xfffffe8405f8  (x와 같은 주소)
  ptr    = 0xfffffe8405f8  (x와 같은 주소)
  &ptr   = 0xfffffe8405f0  (포인터 변수 자체의 주소는 다르다)
```

**체크 포인트:** 포인터 단계가 20인 이유를 설명할 수 있나? (값 단계에서 안 바뀌었고, 참조 단계에서 10이 됐고, 거기서 +10)

---

## 6. 직접 해 보기

### 해 보기 1: 컴파일 에러 읽기 (5분)

`print()` 안의 `// state.position = 0;` 주석을 풀고 빌드한다. 이런 에러가 나온다.

```
error: assignment of member 'MotorState::position' in read-only object
```

"read-only(읽기 전용) 객체의 멤버에 값을 넣으려 했다"는 뜻이다. `const`가 실수를 막아 준 것.
확인했으면 다시 주석 처리한다.

### 해 보기 2: 참조는 "빈 상태"가 될 수 없다 (5분)

`main()` 안에 한 줄 추가하고 빌드해 본다.

```cpp
MotorState& empty;
```

```
error: 'empty' declared as reference but not initialized
```

참조는 **만들 때 반드시 무언가에 묶여야** 한다. 그래서 "없음"을 표현할 수 없고, 대신 nullptr 검사도 필요 없다.
포인터와의 가장 큰 차이다. 확인했으면 지운다.

### 해 보기 3: nullptr 검사를 빼면? (10분)

`moveIfExists`의 `if (state == nullptr) { ... }` 블록을 **잠깐 주석 처리**하고 빌드·실행한다.

- 어떤 일이 일어나는지 본다. (힌트: 프로그램이 끝까지 출력되나?)
- `Segmentation fault`라는 말이 나오면, 그게 "아무것도 없는 주소(0번지)에 접근하려 했다"는 뜻이다.

실제 장비 프로그램이라면 제어 프로그램이 통째로 죽는 상황이다.
**포인터를 받으면 쓰기 전에 확인한다**를 손으로 기억해 두자. 확인했으면 되돌린다.

### 해 보기 4: swap 함수 만들기 (20분)

새 파일 `02_references_pointers/practice.cpp`에 작성한다. (예제 코드를 복사하지 않는다)

두 모터의 위치를 맞바꾸는 함수를 만든다. 먼저 **값으로 받는 버전**부터.

```cpp
#include <iostream>

void swapByValue(double a, double b) {
    double temp = a;
    a = b;
    b = temp;
}

int main() {
    double x = 1.0;
    double y = 2.0;
    swapByValue(x, y);
    std::cout << "x = " << x << ", y = " << y << '\n';
}
```

1. 실행 전에 출력을 예상한다. 실행해서 확인한다. 왜 그렇게 나오는지 4-2절을 다시 본다.
2. `swapByRef(______ a, ______ b)` 를 만들어서 **진짜로 바뀌게** 한다. 함수 본문은 같아도 된다.
3. (도전) 포인터 버전 `swapByPtr(double* a, double* b)`도 만들어 본다. 본문에서 `*`를 어디에 붙여야 할지가 핵심이다. 부를 때는 `swapByPtr(___, ___)`.

빌드는 간단히 g++로 한다.

```bash
cd ~/robot-practice/cpp-study/02_references_pointers
g++ -std=c++17 -Wall -Wextra practice.cpp -o practice && ./practice
```

### 해 보기 5: 사라진 원본을 가리키는 참조 (10분)

같은 `practice.cpp`에 함수 하나를 추가하고 빌드만 해 본다.

```cpp
double& badPosition() {
    double pos = 3.5;
    return pos;
}
```

```
warning: reference to local variable 'pos' returned [-Wreturn-local-addr]
```

"지역 변수의 참조를 돌려준다"는 경고다. 왜 위험한지 스스로 생각해 본다.

- 힌트 1: `pos`는 함수가 끝나면 어떻게 되나?
- 힌트 2: 그럼 돌려받은 별명은 **무엇의** 별명인가?

이게 바로 다음 장 **03 객체 수명**의 주제다. 답이 잘 안 떠오르면 03을 끝내고 다시 와도 된다.
**경고를 무시하지 말자.** CMake에 `-Wall -Wextra`를 켜 둔 이유가 이런 버그를 미리 잡기 위해서다.

---

## 7. 언제 무엇을 쓰나 (내가 정리하기)

아래 표의 오른쪽 칸을 **내 말로** 한 문장씩 채운다. (연습 과제 3번)

| 상황 | 고를 것 | 이유 (직접 쓰기) |
|---|---|---|
| `int`, `double`처럼 작은 값을 읽기만 함 | 값 `T` | |
| 큰 객체(문자열 포함 구조체 등)를 읽기만 함 | `const T&` | |
| 함수가 원본을 바꿔야 함 | `T&` | |
| 대상이 없을 수도 있음 (예: 옵션 센서) | `T*` | |

> 참고: 나중에 `std::optional`(08장)과 스마트 포인터(05장)를 배우면 "없을 수도 있음"과 "누가 소유하나"를 더 안전하게 표현할 수 있다.
> 지금 단계에서 **포인터(`T*`)는 "빌려 쓰기만 하고, 없을 수도 있다"**는 뜻으로만 쓴다.

---

## 8. 확인 질문 (한 줄씩)

1. `moveByValue(x)`를 불렀는데 `x`가 안 바뀐 이유는?
2. `&alias`는 `&x`와 같은데 `&ptr`은 다른 이유는?
3. 모터 상태를 화면에 출력만 하는 함수의 인자 타입으로 `MotorState`보다 `const MotorState&`가 나은 이유 두 가지는?

---

## 9. 내 답과 메모

> 아래에 직접 적는다.

- 질문 1 답:
- 질문 2 답:
- 질문 3 답:
- 해 보기 3에서 본 것:
- 해 보기 5 경고가 위험한 이유:
- 막혔던 곳 / 에러 메시지:
- 새로 이해한 것:

---

## 10. 질문과 답

> 공부하다 생긴 질문을 Claude에게 물어보고, 답을 여기에 정리한다.
