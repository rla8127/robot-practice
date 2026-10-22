# 04. RAII — 학습 노트

> 2주 차. 03에서 확인한 "소멸자는 블록이 끝나면 **반드시** 호출된다"를 **이용**해서,
> 모터 전원을 끄는 걸 **잊을 수 없는** 코드를 만든다.

---

## 1. 오늘의 목표

1. RAII가 무엇인지 한 문장으로 설명한다.
2. `return`으로 일찍 빠져나가거나 예외가 던져져도 정리 코드가 실행되는 이유를 설명한다.
3. RAII 클래스에서 **복사를 막아야 하는** 이유를 직접 확인한다.

---

## 2. 왜 이걸 배워야 하나?

장비 코드에는 **짝**이 있는 작업이 아주 많다.

| 얻기 | 돌려주기 |
|---|---|
| 모터 전원 ON | 모터 전원 OFF |
| 통신 포트 열기 | 포트 닫기 |
| 락(lock) 잠그기 | 락 풀기 |
| `new`로 메모리 받기 | `delete`로 돌려주기 |

문제는 **돌려주기를 사람이 직접** 써야 할 때다.
처음엔 잘 짝을 맞춰도, 나중에 누군가 중간에 `if (오류) return;`을 한 줄 넣는 순간 짝이 깨진다.

> 센서 오류로 함수가 일찍 끝났는데, **모터 전원은 켜진 채로** 남아 있다.

이건 버그 수준이 아니라 **사고**가 될 수 있다. RAII는 이 문제를 언어 규칙으로 막는 방법이다.

---

## 3. RAII를 한 문장으로

**RAII (Resource Acquisition Is Initialization)**
= "자원을 **얻는 건 생성자**에서, **돌려주는 건 소멸자**에서 한다."

이름은 어렵지만 아이디어는 단순하다.

```
자원을 객체 안에 넣는다
  → 객체가 사라질 때 소멸자가 반드시 불린다 (03에서 확인)
    → 소멸자에서 자원을 돌려주면, 돌려주기를 잊을 수 없다
```

비유: 호텔 **카드키**와 같다. 방에 들어가면(생성) 전기가 켜지고, 카드를 빼고 나가면(소멸) 전기가 꺼진다.
"전등 끄는 걸 깜빡했다"가 일어날 수 없다. 나가는 방법이 뭐든 카드는 빠지기 때문이다.

**이미 써 본 RAII:** 사실 `std::string`, `std::vector`도 RAII다.
안에서 메모리를 쓰지만, 우리가 `delete`를 한 번도 안 썼는데도 새지 않는다. 소멸자가 알아서 돌려주기 때문이다.
앞으로 배울 `std::unique_ptr`(05), `std::lock_guard`(10), `std::ofstream`(파일)도 전부 같은 원리다.

---

## 4. main.cpp 한 줄씩 읽기

### 4-1. 가상의 하드웨어 API

```cpp
void powerOn(int axis)  { std::cout << "  [HW] axis " << axis << " 전원 ON\n"; }
void powerOff(int axis) { std::cout << "  [HW] axis " << axis << " 전원 OFF\n"; }
```

실제 장비 드라이버는 이런 **C 스타일 함수**로 제공되는 경우가 많다. 켜는 함수, 끄는 함수가 따로 있고,
짝을 맞추는 건 **쓰는 사람 몫**이다. 여기서는 출력만 해서 흉내 낸다.

### 4-2. RAII 없이 짠 코드

```cpp
void runWithoutRaii(bool sensorError) {
    powerOn(1);
    if (sensorError) {
        std::cout << "  센서 오류! 일찍 return\n";
        return;  // ← powerOff(1)를 잊었다.
    }
    std::cout << "  작업 수행\n";
    powerOff(1);
}
```

정상 경로에서는 ON → 작업 → OFF로 짝이 맞는다.
하지만 `sensorError`가 true면 `return`으로 빠져나가면서 **`powerOff(1)`을 건너뛴다.**

"그럼 return 앞에 powerOff를 넣으면 되잖아?" — 맞다. 그런데

- 빠져나가는 곳이 5군데면 5군데 다 넣어야 하고,
- 예외(4-5절)로 빠져나가는 경우는 그 방법으로도 못 막는다.

### 4-3. RAII 래퍼 클래스

```cpp
class MotorPowerGuard {
public:
    explicit MotorPowerGuard(int axis) : axis_(axis) {
        powerOn(axis_);      // 생성 = 자원 얻기
    }
    ~MotorPowerGuard() {
        powerOff(axis_);     // 소멸 = 자원 돌려주기
    }
    ...
private:
    int axis_;
};
```

03의 `Tracer`와 모양이 똑같다. 차이는 출력 대신 **진짜 일**(전원 ON/OFF)을 한다는 것뿐이다.
이름 끝의 **Guard**(지키는 것)는 "이 객체가 살아 있는 동안 자원을 지켜 준다"는 관례적인 이름이다.

### 4-4. RAII로 짠 코드

```cpp
void runWithRaii(bool sensorError) {
    MotorPowerGuard guard(2);   // 여기서 전원 ON
    if (sensorError) {
        std::cout << "  센서 오류! 일찍 return\n";
        return;                 // 함수를 나가면서 guard 소멸 → 전원 OFF
    }
    std::cout << "  작업 수행\n";
}                               // 정상 종료도 여기서 guard 소멸 → 전원 OFF
```

`powerOff`를 **한 번도 직접 안 썼다.** 그런데도 두 경로 모두 전원이 꺼진다.
`guard`는 지역 변수라서, **어떤 방법으로든 함수를 나가면** 소멸자가 불리기 때문이다. (03의 규칙 1번)

### 4-5. 예외가 던져져도?

```cpp
void runWithException() {
    MotorPowerGuard guard(3);
    std::cout << "  작업 중 예외 발생!\n";
    throw std::runtime_error("encoder timeout");
}
```

**예외(exception)**는 "더 진행할 수 없는 문제가 생겼으니, 처리할 수 있는 곳까지 **거슬러 올라가라**"는 신호다.
`throw`를 만나면 함수가 그 자리에서 멈추고, 바깥쪽의 `try { } catch { }`를 찾아 올라간다.

```cpp
try {
    runWithException();
} catch (const std::exception& e) {
    std::cout << "  catch: " << e.what() << '\n';
}
```

올라가는 동안 지나치는 함수들의 지역 변수는 **전부 소멸자가 불린다.** 이걸 **스택 되감기(stack unwinding)**라고 한다.
그래서 `guard`의 소멸자가 불리고 → 전원 OFF → 그다음에 `catch`가 실행된다.

```
throw ──▶ runWithException 탈출 (guard 소멸 → OFF) ──▶ main의 catch 도착
```

> 예외 자체는 08장에서 더 자세히 다룬다. 오늘은 "**예외로 나가도 소멸자는 불린다**"만 기억하면 된다.
> (단, 아무도 `catch`하지 않으면 얘기가 다르다. 해 보기 3에서 확인한다.)

### 4-6. 복사 금지 (`= delete`)

```cpp
MotorPowerGuard(const MotorPowerGuard&) = delete;
MotorPowerGuard& operator=(const MotorPowerGuard&) = delete;
```

처음 보는 문법이라 하나씩 떼어 본다.

- `MotorPowerGuard(const MotorPowerGuard&)` : **복사 생성자**. `MotorPowerGuard b = a;`처럼 "a를 본떠서 b를 새로 만들 때" 불린다.
- `operator=(const MotorPowerGuard&)` : **복사 대입 연산자**. 이미 있는 b에 `b = a;`로 덮어쓸 때 불린다.
- `= delete` : "이 함수는 **없는 것으로 친다.** 쓰려고 하면 컴파일 에러를 내라."

**왜 막아야 할까?** 복사를 허용하면 이렇게 된다.

```
guard(2) 생성 → 전원 ON  (1번)
copy = guard  → 복사본 생성. 생성자가 아니라 복사라서 ON은 안 불림
블록 끝       → copy 소멸 → 전원 OFF
              → guard 소멸 → 전원 OFF  ← 두 번째! 이미 꺼진 걸 또 끔
```

ON은 한 번인데 OFF가 **두 번**. 지금은 출력만 두 번이지만, 실제 장비라면
"이미 꺼진 축을 다시 끄라는 명령" 또는 메모리라면 **같은 메모리를 두 번 delete**(프로그램 충돌)가 된다.

**규칙:** 자원을 하나 쥐고 있는 RAII 클래스는 **복사를 막는다.** 자원은 한 개인데 주인이 둘이 되면 안 되기 때문이다.
("그럼 넘겨주고 싶을 땐?" → **이동(move)**이라는 방법이 있다. 05에서 맛보고 06에서 제대로 배운다.)

---

## 5. 실행하고 예상과 비교

**실행 전에** 각 구간에서 `전원 OFF`가 **나오는지 안 나오는지**만 먼저 예상해 본다.

| 구간 | OFF 나올까? (예상) |
|---|---|
| RAII 없음 + 오류 | |
| RAII + 정상 | |
| RAII + 오류 | |
| RAII + 예외 | |

```bash
cd ~/robot-practice/cpp-study
cmake --build build --target 04_raii
./build/04_raii
```

<details>
<summary>실제 결과 (예상을 다 쓴 뒤에 펼친다)</summary>

```
[RAII 없음 + 오류]
  [HW] axis 1 전원 ON
  센서 오류! 일찍 return

[RAII + 정상]
  [HW] axis 2 전원 ON
  작업 수행
  [HW] axis 2 전원 OFF

[RAII + 오류]
  [HW] axis 2 전원 ON
  센서 오류! 일찍 return
  [HW] axis 2 전원 OFF

[RAII + 예외]
  [HW] axis 3 전원 ON
  작업 중 예외 발생!
  [HW] axis 3 전원 OFF
  catch: encoder timeout
```

</details>

**체크 포인트**
- 첫 구간에 **axis 1 OFF가 끝까지 없다.** 프로그램이 끝날 때까지 1번 축 전원은 켜진 채다. (연습 과제 1번)
- 예외 구간에서 `OFF`가 `catch:`보다 **먼저** 나온다. 스택 되감기가 catch에 도착하기 전에 일어난다는 증거다.

---

## 6. 직접 해 보기

### 해 보기 1: 복사 금지 확인 (10분) — 연습 과제 3번

**(1)** `runWithRaii` 함수 안, `guard` 선언 바로 다음 줄에 추가하고 빌드한다.

```cpp
MotorPowerGuard copy = guard;
```

```
error: use of deleted function 'MotorPowerGuard::MotorPowerGuard(const MotorPowerGuard&)'
```

"delete된(없는 것으로 친) 함수를 썼다"는 뜻. `= delete`가 실수를 막았다.

**(2)** 이번엔 `= delete` 두 줄을 **주석 처리**하고 다시 빌드·실행한다.

- `[RAII + 정상]` 구간에서 `axis 2 전원 OFF`가 몇 번 나오나?
- 4-6절의 그림과 같은지 확인한다.

**(3)** 확인했으면 `= delete`와 추가한 줄을 모두 되돌린다.

### 해 보기 2: 하나의 함수에 Guard 두 개 (10분)

`main()` 맨 끝(`return 0;` 앞)에 추가한다.

```cpp
std::cout << "\n[2축 동시]\n";
{
    MotorPowerGuard x(10);
    MotorPowerGuard y(20);
    std::cout << "  2축 작업 수행\n";
}
```

1. 실행 전 예상: OFF는 **어느 축부터** 나올까? (힌트: 03의 규칙 2번)
2. 실행해서 확인한다.

> 실제 장비에서도 여러 자원을 차례로 얻었으면, **역순으로** 돌려주는 게 보통 안전하다.
> RAII를 쓰면 이 순서를 언어가 자동으로 맞춰 준다.

### 해 보기 3: 아무도 catch하지 않으면? (10분)

`main()`의 `try { ... } catch (...) { ... }` 부분을 **`runWithException();` 한 줄로** 바꾸고 빌드·실행한다.

이런 메시지와 함께 프로그램이 멈춘다.

```
terminate called after throwing an instance of 'std::runtime_error'
  what():  encoder timeout
Aborted (core dumped)
```

- `axis 3 전원 OFF`가 **나왔나?**
- 4-5절에서는 "예외로 나가도 소멸자가 불린다"고 했는데, 왜 다를까?

<details>
<summary>힌트 (먼저 생각해 본 뒤 펼친다)</summary>

예외를 받아 줄 `catch`가 아무 데도 없으면 프로그램은 `std::terminate()`로 **즉시 종료**된다.
이때 스택 되감기를 할지 말지는 컴파일러가 정해도 되는데(표준이 보장하지 않음), GCC는 **하지 않는다.**
그래서 소멸자가 불리지 않는다.

→ RAII가 제대로 동작하려면 **예외는 어딘가에서 반드시 catch해야 한다.**
실제 제어 프로그램은 보통 `main()` 가장 바깥에 `try/catch`를 둔다.

</details>

확인했으면 원래대로 되돌린다.

### 해 보기 4: ScopedTimer 만들기 (30분) — 연습 과제 2번

같은 아이디어로 **"블록이 실행되는 데 걸린 시간"을 재는** 클래스를 만든다.
새 파일 `04_raii/practice.cpp`에 처음부터 작성한다.

**목표 사용법:**

```cpp
int main() {
    {
        ScopedTimer timer("홈 복귀");
        // 오래 걸리는 작업 흉내
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }   // ← 여기서 "[홈 복귀] 200 ms" 비슷한 출력이 나와야 함
}
```

**설계 힌트 (빈칸을 채운다):**

| 언제 | 할 일 |
|---|---|
| 생성자 | 이름을 저장하고, **지금 시각**을 멤버에 저장 |
| 소멸자 | **지금 시각 − 저장한 시각**을 계산해서 출력 |
| 복사 | ______ (4-6절을 참고해서 정한다) |

**쓸 도구 (`#include <chrono>`, `#include <thread>`):**

- 지금 시각: `std::chrono::steady_clock::now()`
- 그 시각의 타입: `std::chrono::steady_clock::time_point`
- 두 시각의 차이를 ms 정수로: `std::chrono::duration_cast<std::chrono::milliseconds>(끝 - 시작).count()`

> `steady_clock`은 "시계를 사람이 바꿔도 거꾸로 가지 않는 시계"다. 시간 **간격**을 잴 때는 항상 이걸 쓴다.

```bash
cd ~/robot-practice/cpp-study/04_raii
g++ -std=c++17 -Wall -Wextra practice.cpp -o practice && ./practice
```

**완료 조건**
- [ ] 200 ms 근처 값이 출력된다. (정확히 200이 아니어도 된다. 왜일까?)
- [ ] 블록 안에서 `return`으로 일찍 나가도 시간이 출력된다. (함수로 감싸서 확인)
- [ ] `ScopedTimer t2 = timer;`가 컴파일 에러가 난다.

> 이 ScopedTimer는 나중에 "제어 주기가 1ms 안에 끝나는지" 확인할 때 실제로 쓸 수 있다.

---

## 7. 확인 질문 (한 줄씩)

1. RAII를 "생성자", "소멸자", "자원"이라는 단어를 넣어 한 문장으로 설명하면?
2. `runWithoutRaii`의 `return` 앞에 `powerOff(1);`을 넣는 것보다 RAII가 나은 이유 두 가지는?
3. `MotorPowerGuard`의 복사를 막지 않으면 어떤 일이 생기나?

---

## 8. 내 답과 메모

> 아래에 직접 적는다.

- 5절 OFF 예상에서 틀린 구간:
- 질문 1 답:
- 질문 2 답:
- 질문 3 답:
- 해 보기 3에서 OFF가 안 나온 이유 (내 말로):
- ScopedTimer가 정확히 200이 아닌 이유:
- 막혔던 곳 / 에러 메시지:
- 새로 이해한 것:

---

## 9. 다음 장 연결

오늘 만든 `MotorPowerGuard`는 "모터 전원"이라는 자원을 지켰다.
**메모리**도 자원이다. `new`로 얻고 `delete`로 돌려줘야 한다. 03 해 보기 3에서 `delete`를 잊으면 소멸자가 안 불리는 것도 봤다.

05 스마트 포인터는 **메모리용 Guard**다. 표준 라이브러리가 이미 만들어 두었다.
`MotorPowerGuard`에서 본 "복사 금지"도 `std::unique_ptr`에 그대로 들어 있다.

---

## 10. 질문과 답

> 공부하다 생긴 질문을 Claude에게 물어보고, 답을 여기에 정리한다.
