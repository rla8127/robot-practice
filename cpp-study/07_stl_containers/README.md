# 07. STL 컨테이너와 알고리즘 — 학습 노트

> 3주 차 첫 주제. Python의 `list`, `dict`, `sorted()`, `filter()`에 해당하는 C++ 도구를 배운다.
> 이름은 많지만, 하나씩 순서대로 보면 Python에서 이미 해 본 일들이다.

---

## 1. 오늘 배울 내용 한 문장

**여러 개의 데이터를 담는 상자(컨테이너)를 고르고, 그 안에서 정렬·검색·삭제를 직접 반복문 없이 해 본다.**

---

## 2. 이 개념이 필요한 이유

장비 SW는 **여러 개**를 다루는 일이 대부분이다.

| 다루는 것 | 예 |
|---|---|
| 위치 기록 | 모터 위치를 1ms마다 기록한 값 수천 개 |
| 명령 대기열 | `MoveTo`, `Stop`, `Home` 같은 명령 여러 개를 우선순위대로 처리 |
| 에러 집계 | "timeout 3번, limit 2번"처럼 이름별로 횟수 세기 |
| 설정 표 | "x_max = 100, y_max = 50"처럼 이름으로 제한값 찾기 |

이걸 매번 배열과 `for`문으로 직접 짜면:

- 크기를 미리 정해야 하고, 넘치면 메모리를 망가뜨린다.
- 정렬·검색을 직접 짜다가 버그를 만든다.
- 메모리를 `new`/`delete`로 직접 관리해야 한다. (03~05에서 본 문제들)

**STL(Standard Template Library)**은 C++이 기본으로 주는 **"데이터 담는 상자 + 자주 쓰는 작업"** 모음이다.
이미 검증된 코드라서 직접 짜는 것보다 **빠르고 안전하다.**
그리고 06에서 배운 것처럼, 이 상자들은 모두 **RAII**라서 메모리 정리를 신경 쓸 필요가 없다.

---

## 3. 일상적인 비유

공구를 정리하는 상자를 고른다고 생각하자.

| 컨테이너 | 비유 | 특징 |
|---|---|---|
| `std::vector` | **칸이 늘어나는 공구함** | 칸이 한 줄로 붙어 있다. 꽉 차면 더 큰 공구함으로 통째로 옮긴다 |
| `std::array` | **칸 수가 정해진 공구함** | 처음부터 칸 수가 고정. 늘릴 수 없다 |
| `std::map` | **가나다순으로 꽂힌 서류철** | 이름표(키)로 찾는다. 항상 이름순으로 정렬돼 있다 |
| `std::unordered_map` | **번호 붙은 사물함** | 이름표를 보고 사물함 번호를 바로 계산해서 연다. 순서는 없지만 빠르다 |

그리고 **알고리즘**(`sort`, `find_if`, ...)은 공구함에 쓰는 **작업 지시서**다.
"이 공구함을 크기순으로 정렬해", "빨간 공구 중 첫 번째를 찾아" 같은 일을 대신 해 준다.

---

## 4. 가장 간단한 코드 예제

새 개념은 **`std::vector`** 하나만. Python `list`와 나란히 놓고 본다.

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<double> positions = {3.0, 1.5};   // positions = [3.0, 1.5]
    positions.push_back(7.2);                     // positions.append(7.2)

    std::cout << positions.size() << '\n';        // len(positions)       → 3
    std::cout << positions[0] << '\n';            // positions[0]         → 3

    for (double p : positions) {                  // for p in positions:
        std::cout << p << ' ';                    //     print(p)
    }
    std::cout << '\n';
}
```

| 조각 | 뜻 |
|---|---|
| `std::vector<double>` | **double을 담는** vector. 꺾쇠 `< >` 안에 **담을 타입**을 적는다 |
| `= {3.0, 1.5}` | 처음에 담을 값들 |
| `push_back(7.2)` | **맨 뒤에** 하나 추가 |
| `size()` | 지금 몇 개 들어 있나 |
| `positions[0]` | 0번째 원소. Python처럼 0부터 센다 |
| `for (double p : positions)` | 원소를 앞에서부터 하나씩 `p`에 **복사해서** 반복 |

Python과 다른 점은 딱 하나: **담을 타입을 정해야 한다.** `std::vector<double>`에는 double만 들어간다. 문자열을 넣으면 컴파일 에러다.

> **잠깐 확인 ①**
> `std::vector<std::string> names;`는 무엇을 담는 상자일까? 여기에 `push_back(3.0)`을 하면 어떻게 될까?
> (문자열을 담는 vector. 3.0은 문자열이 아니라서 컴파일 에러)

---

## 5. 코드 한 줄씩 설명 (main.cpp)

main.cpp는 구간마다 **새 개념이 하나씩** 나온다. 순서대로 본다.

### 5-1. vector — `[]`와 `at()`

```cpp
// positions[10]은 범위 검사를 안 한다(정의되지 않은 동작).
// positions.at(10)은 std::out_of_range 예외를 던진다.
```

원소가 5개인데 10번째를 달라고 하면?

| 쓰는 법 | 범위 밖일 때 | 속도 |
|---|---|---|
| `positions[10]` | **아무 검사도 안 한다.** 메모리 아무 데나 읽는다 | 빠름 |
| `positions.at(10)` | **예외**를 던진다 (04에서 본 `throw`) | 아주 약간 느림 |

`[]`의 결과는 **정의되지 않은 동작(undefined behavior, UB)**이다.
"정의되지 않았다" = **무슨 일이 일어날지 C++이 아무것도 약속하지 않는다**는 뜻이다.
멀쩡해 보일 수도, 엉뚱한 값이 나올 수도, 죽을 수도 있다. **매번 다를 수도 있다.** 그래서 가장 찾기 어려운 버그가 된다.

**Python과의 차이:** Python은 `positions[10]`에서 항상 `IndexError`를 낸다. C++의 `[]`는 **안 낸다.**

### 5-2. array — 크기가 고정된 배열

```cpp
std::array<int, 2> axisIds = {0, 1};
```

| 조각 | 뜻 |
|---|---|
| `int` | int를 담는다 |
| `2` | **정확히 2개.** 빌드할 때 정해지는 숫자여야 한다 (01 Q1의 `constexpr` 같은 것) |

`push_back`이 **없다.** 늘릴 수 없기 때문이다.
X·Y 두 축, 바퀴 4개처럼 **개수가 절대 안 바뀌는 것**에 쓴다. "축이 3개로 늘었다"는 실수를 컴파일러가 막아 준다.

### 5-3. sort + 람다 — 내가 정한 기준으로 정렬

여기서 새로 나오는 게 두 개(**반복자**, **람다**)라서 따로따로 본다.

#### (1) 반복자: `begin()`과 `end()`

```cpp
std::sort(queue.begin(), queue.end(), ...);
```

STL 알고리즘은 "**어디부터 어디까지**"를 받는다. 그 위치 표시를 **반복자(iterator)**라고 한다.
지금은 "**원소를 가리키는 손가락**" 정도로 생각하면 된다. 포인터(02)와 비슷하게 `*it`, `it->`로 쓴다.

```
queue:      [ MoveTo ][ Stop ][ Home ][  (끝)  ]
              ▲                         ▲
          begin()                     end()
```

- `begin()` : **첫 원소**를 가리킨다.
- `end()` : 마지막 원소의 **다음 칸**을 가리킨다. **원소가 아니다!** 거기엔 아무것도 없다.
  그래서 "끝까지 갔는데 못 찾았다"를 표시하는 데 쓴다. (5-4절)

Python의 `range(0, 3)`이 0, 1, 2까지이고 3은 안 들어가는 것과 같은 방식이다.

#### (2) 람다: 즉석에서 만드는 이름 없는 함수

```cpp
[](const Command& a, const Command& b) { return a.priority < b.priority; }
```

Python의 `lambda a, b: a.priority < b.priority`와 같다. 조각을 나누면:

```
[]                                     ( const Command& a, const Command& b )    { return a.priority < b.priority; }
└─ 캡처: 바깥 변수를 쓸지 (5-5절)        └─ 인자: 비교할 명령 두 개                   └─ 본문: a가 b보다 앞이면 true
```

`std::sort`는 이 함수를 계속 불러서 "**a가 b보다 앞에 와야 하나?**"를 물어본다. true면 a를 앞에 둔다.
우선순위 숫자가 **작은 게 앞**이니 `a.priority < b.priority`.

> **잠깐 확인 ②**
> 우선순위 숫자가 **큰** 명령부터 처리하고 싶다면 람다 본문을 어떻게 바꾸면 될까?

#### (3) 결과 출력: `const auto&`

```cpp
for (const auto& cmd : queue) {
```

- `auto` : 원소 타입(`Command`)을 컴파일러가 알아서 정한다.
- `const &` : 복사 없이(`&`) 읽기만(`const`). (02)

`Command`에는 `std::string`이 들어 있어서 복사하면 비용이 든다. 그래서 4절의 `double p`(복사)와 달리 참조로 받는다.

### 5-4. find_if — 조건에 맞는 첫 원소 찾기

```cpp
auto it = std::find_if(queue.begin(), queue.end(),
                       [](const Command& c) { return c.name == "Home"; });
if (it != queue.end()) {
    std::cout << "  찾음: " << it->name << '\n';
}
```

- 람다가 **true를 돌려주는 첫 원소**의 위치(반복자)를 돌려준다.
- **못 찾으면 `end()`를 돌려준다.** 그래서 쓰기 전에 반드시 `it != queue.end()`를 확인한다.
  (02에서 포인터를 쓰기 전에 `nullptr`인지 확인한 것과 같은 이유다)
- `it->name` : 반복자가 가리키는 원소의 `name`. 포인터의 `->`와 같다.
- `auto it` : 반복자의 진짜 타입 이름은 `std::vector<Command>::iterator`로 길어서 `auto`로 받는다.

### 5-5. remove_if + erase — 조건에 맞는 원소 지우기

```cpp
const double limit = 5.0;
positions.erase(std::remove_if(positions.begin(), positions.end(),
                               [limit](double p) { return p > limit; }),
                positions.end());
```

여기 새로 나온 건 **람다의 캡처 `[limit]`** 하나다. 나머지는 두 단계 동작이다.

**캡처 `[limit]`:** 람다는 별도의 작은 함수라서, 원래는 바깥 변수 `limit`을 **볼 수 없다.**
`[limit]`이라고 적으면 "바깥의 `limit`을 **복사해서** 람다 안으로 가져가겠다"는 뜻이다.
`[]`를 비워 두고 본문에서 `limit`을 쓰면 컴파일 에러가 난다.

**두 단계 동작:** 이름과 달리 `remove_if`는 **지우지 않는다!** 6절 ③ 그림에서 단계별로 본다.

1. `std::remove_if(...)` : 남길 원소를 **앞으로 당겨 모으고**, "남길 원소가 끝나는 위치"를 돌려준다. **size는 그대로**다.
2. `positions.erase(그 위치, end())` : 그 위치부터 끝까지를 **진짜로 지운다.** 여기서 size가 줄어든다.

> 이 두 줄 패턴을 **erase-remove 관용구**라고 부른다. 모양을 통째로 외워 두는 편이 낫다.
> (C++20부터는 `std::erase_if(positions, 람다);` 한 줄로 된다. 우리는 C++17이라 이렇게 쓴다)

### 5-6. map — 이름으로 찾는 사전

```cpp
std::map<std::string, int> errorCount;
errorCount["timeout"] += 1;
errorCount["limit"] += 2;
errorCount["timeout"] += 1;
```

- `std::map<std::string, int>` : 키는 `string`, 값은 `int`인 사전. 꺾쇠 안에 **키 타입, 값 타입** 순서.
- `errorCount["timeout"]` : 키 `"timeout"`의 값.
  **키가 없으면 값을 0으로 해서 새로 만든다.** 그래서 "처음 보는 에러면 0에서 시작해 +1"이 한 줄로 된다.
  (Python dict라면 `KeyError`가 난다. `defaultdict(int)`처럼 동작한다고 보면 된다)

```cpp
for (const auto& [key, count] : errorCount) {
```

- map의 원소 하나는 **(키, 값) 한 쌍**이다. 이 쌍의 타입을 `std::pair`라고 한다. 앞이 `.first`(키), 뒤가 `.second`(값).
- `[key, count]` : 그 쌍을 **두 이름으로 나눠** 받는 C++17 문법이다. **구조적 바인딩(structured binding)**이라고 부른다.
  Python의 `for key, count in d.items():`와 같다.
- 출력 순서는 넣은 순서가 아니라 **키의 알파벳순**이다. (`limit` < `timeout`) map은 항상 정렬된 상태를 유지한다.

### 5-7. unordered_map — find와 count로 조회

```cpp
std::unordered_map<std::string, double> limits = {{"x_max", 100.0}, {"y_max", 50.0}};

if (auto found = limits.find("x_max"); found != limits.end()) {
    std::cout << "  x_max = " << found->second << '\n';
}
```

| 조각 | 뜻 |
|---|---|
| `{{"x_max", 100.0}, ...}` | 바깥 `{ }` = 사전 전체, 안쪽 `{ }` = (키, 값) 한 쌍 |
| `limits.find("x_max")` | 키를 찾으면 그 위치(반복자), 없으면 `end()`. **키를 새로 만들지 않는다** |
| `if (준비; 조건)` | C++17 문법. 준비문에서 만든 `found`는 **이 if 안에서만** 쓸 수 있다 |
| `found->second` | 찾은 쌍의 **값**. 키는 `found->first` |

`if (준비; 조건)`은 이렇게 두 줄로 쓴 것과 같다. 차이는 `found`가 if 밖으로 새어 나가지 않는다는 것뿐이다.

```cpp
auto found = limits.find("x_max");
if (found != limits.end()) { ... }
```

```cpp
if (limits.count("z_max") == 0) {
```

- `count(키)` : 그 키가 몇 개 있나. 키는 겹치지 않으니 **0 아니면 1**. "있나 없나" 확인용이다.

**왜 `limits["z_max"]`로 확인하면 안 될까?** 5-6절에서 본 것처럼 `[]`는 키가 없으면 **새로 만든다.**
그러면 `z_max = 0.0`이 사전에 **몰래 생긴다.** (해 보기 3에서 확인)

---

## 6. 메모리·객체 상태 변화 시각화

### ① vector는 한 줄로 붙은 칸이다

```cpp
std::vector<double> positions = {3.0, 1.5, 7.2, 0.4};
```

```
positions (스택)                     힙
┌──────────────────┐               ┌─────┬─────┬─────┬─────┐
│ 시작 주소 ────────┼──────────────▶│ 3.0 │ 1.5 │ 7.2 │ 0.4 │
│ size     = 4     │               └─────┴─────┴─────┴─────┘
│ capacity = 4     │                 [0]   [1]   [2]   [3]
└──────────────────┘
```

06의 `Buffer`와 같은 모양이다. vector 객체 자체는 작고, 원소들은 **힙에 한 줄로 붙어** 있다.

- **size** : 지금 들어 있는 원소 개수
- **capacity** : 새 자리를 받지 않고 넣을 수 있는 최대 개수 (공구함의 **칸 수**)

### ② push_back으로 칸이 모자라면: 재할당

```cpp
positions.push_back(5.0);   // size 4 → 5, 그런데 capacity가 4
```

```
1. 더 큰 새 자리를 받는다 (보통 2배)
   새 자리: [     ][     ][     ][     ][     ][     ][     ][     ]   capacity = 8

2. 기존 원소를 새 자리로 옮긴다 (06의 "vector 재할당 → 이동")
   새 자리: [ 3.0 ][ 1.5 ][ 7.2 ][ 0.4 ][     ][     ][     ][     ]

3. 새 원소를 넣는다
   새 자리: [ 3.0 ][ 1.5 ][ 7.2 ][ 0.4 ][ 5.0 ][     ][     ][     ]   size = 5

4. 옛 자리는 돌려준다 (delete)
   옛 자리: ✘ 사라짐
```

**4번이 중요하다.** 옛 자리를 가리키던 참조·포인터·반복자는 **전부 사라진 메모리를 가리키게** 된다.
이걸 **반복자 무효화(iterator invalidation)**라고 한다. (연습 과제 3번, 해 보기 5)

### ③ remove_if + erase 단계별

`positions = [3.0, 1.5, 7.2, 0.4, 5.0]`에서 5.0 **초과**(7.2)를 지운다.

```
시작:            [ 3.0 ][ 1.5 ][ 7.2 ][ 0.4 ][ 5.0 ]      size = 5
                                ✘ 지울 것

remove_if 후:    [ 3.0 ][ 1.5 ][ 0.4 ][ 5.0 ][  ?  ]      size = 5 (그대로!)
                                ◀──── 당겨짐        ▲
                                                   └ remove_if가 돌려준 위치
                                                     (? = 어떤 값인지 정해져 있지 않음)

erase 후:        [ 3.0 ][ 1.5 ][ 0.4 ][ 5.0 ]             size = 4
```

`remove_if`는 **남길 것을 앞으로 모으기만** 했다. 실제로 줄인 건 `erase`다.
`erase`를 빼먹으면 size가 그대로 5이고, 맨 끝에 정체불명의 값(`?`)이 남는다.

### ④ map과 unordered_map의 찾는 방법

```
map (정렬된 서류철)                   unordered_map (번호 사물함)
"limit" → 2                         "x_max" ─계산→ 사물함 7번 → 100.0
"timeout" → 2                       "y_max" ─계산→ 사물함 2번 → 50.0

항상 키 순서대로 정렬되어 있다          키에서 사물함 번호를 바로 계산한다 (해시)
반씩 줄여 가며 찾는다 (사전 찾기처럼)    순서는 없다. 보통 더 빠르다
```

---

## 7. 비슷한 개념과 차이점

### 7-1. vector vs array

| | `std::vector<T>` | `std::array<T, N>` |
|---|---|---|
| 크기 | 실행 중에 늘고 줄어든다 | 빌드할 때 고정 |
| `push_back` | ⭕ | ❌ |
| 원소 위치 | 힙 | 객체 안 (지역 변수면 스택) |
| 언제 쓰나 | **기본 선택.** 개수를 모르거나 바뀔 때 | 축 개수처럼 **절대 안 바뀔 때** |

### 7-2. map vs unordered_map

| | `std::map` | `std::unordered_map` |
|---|---|---|
| 순서 | 키 순서로 **정렬** | **순서 없음** (출력 순서를 믿으면 안 됨) |
| 찾는 속도 | 원소가 많으면 조금 느려짐 | 보통 더 빠름 |
| 언제 쓰나 | 정렬된 출력이 필요할 때 (로그, 리포트) | 조회만 하면 될 때 (설정 표, 캐시) |

**Python dict와의 차이:** Python 3.7+의 dict는 **넣은 순서**를 기억한다. C++의 `unordered_map`은 **안 기억한다.** `map`은 넣은 순서가 아니라 **키 순서**다.

### 7-3. 원소를 꺼내는 세 가지 방법 (map / unordered_map)

| 쓰는 법 | 키가 있으면 | 키가 **없으면** | 언제 쓰나 |
|---|---|---|---|
| `m["key"]` | 값 | **0으로 새로 만든다** | 값을 **넣거나 더할** 때 |
| `m.at("key")` | 값 | **예외** (`std::out_of_range`) | 반드시 있어야 하는 키 |
| `m.find("key")` | 위치 | `end()` | **있는지 확인하고** 꺼낼 때 |

vector의 `[]` / `at()`(5-1절)과 비교하면: vector의 `[]`는 **검사를 안 하고**, map의 `[]`는 **새로 만든다.** 둘 다 `[]`지만 하는 일이 다르다.

### 7-4. 인덱스 vs 반복자 vs 포인터

| | 인덱스 `positions[2]` | 반복자 `it` | 포인터 `double* p` (02) |
|---|---|---|---|
| 정체 | 몇 번째 칸인지 **숫자** | 원소를 가리키는 **손가락** | 메모리 **주소** |
| 원소 꺼내기 | `positions[i]` | `*it`, `it->` | `*p`, `p->` |
| "없음" 표시 | `size()` 이상 | `end()` | `nullptr` |
| 재할당 후 | **계속 유효** (같은 번째 칸) | **무효** | **무효** |
| 모든 컨테이너에서? | vector·array만 | ⭕ map 포함 전부 | — |

반복자는 **포인터를 일반화**한 것이다. map처럼 칸이 한 줄로 붙어 있지 않은 상자에서도 "다음 원소"로 이동할 수 있게 해 준다.
그래서 `sort`, `find_if` 같은 알고리즘이 **어떤 컨테이너든** 반복자만 받으면 동작한다.

### 7-5. 직접 짠 for문 vs 알고리즘

```cpp
// 직접 짠 for문
std::size_t found = queue.size();   // "못 찾음" 표시로 size()를 쓴다
for (std::size_t i = 0; i < queue.size(); ++i) {
    if (queue[i].name == "Home") { found = i; break; }
}

// 알고리즘
auto it = std::find_if(queue.begin(), queue.end(),
                       [](const Command& c) { return c.name == "Home"; });
```

결과는 같다. 알고리즘 쪽이 나은 점:

- **이름이 하는 일을 말해 준다.** `find_if`를 보면 "찾는구나"를 바로 안다. for문은 끝까지 읽어야 안다.
- 인덱스 범위 실수(`<=` vs `<`)를 할 일이 없다.

---

## 8. 장비제어 SW 실전 예제

**상황:** 명령 대기열의 이동 명령을 처리하기 전에, 축별 **제한값 표**로 범위를 검사한다. 범위를 넘는 명령은 **거부하고 에러를 센다.**
(01에서 모터 하나에 넣은 안전 범위를, 여러 축과 여러 명령으로 넓힌 것이다)

```cpp
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct MoveCommand {
    std::string axis;    // "x", "y", ...
    double target;       // 목표 위치 (mm)
};

int main() {
    const std::unordered_map<std::string, double> maxPosition = {{"x", 100.0}, {"y", 50.0}};
    std::map<std::string, int> rejectCount;

    std::vector<MoveCommand> queue = {{"x", 30.0}, {"y", 80.0}, {"z", 10.0}, {"x", 120.0}};

    for (const auto& cmd : queue) {
        auto found = maxPosition.find(cmd.axis);
        if (found == maxPosition.end()) {        // 표에 없는 축
            ++rejectCount["unknown_axis"];
            continue;
        }
        if (cmd.target < 0.0 || cmd.target > found->second) {   // 범위 밖
            ++rejectCount["out_of_range"];
            continue;
        }
        std::cout << "실행: " << cmd.axis << " → " << cmd.target << '\n';
    }

    for (const auto& [reason, count] : rejectCount) {
        std::cout << "거부 " << reason << ": " << count << "건\n";
    }
}
```

출력:

```
실행: x → 30
거부 out_of_range: 2건
거부 unknown_axis: 1건
```

**여기서 볼 것**

- **제한값 표는 `const`이고 `find`로 조회한다.** `maxPosition[cmd.axis]`로 썼다면?
  `const`가 아니었다면 `"z"`가 **최대 위치 0.0인 축으로 몰래 생겼을 것이다.** 표에 없는 축을 "거부"해야 하는데 엉뚱한 제한값이 생긴 셈이다.
  `const`로 만들어 두면 `[]` 자체가 **컴파일 에러**가 나서 이 실수를 막아 준다. (해 보기 3)
- **에러 집계는 `[]`로 한다.** 처음 보는 사유면 0에서 시작해 +1. 여기서는 `[]`의 "새로 만들기"가 **원하는 동작**이다.
- **같은 `[]`라도 조회에는 위험하고, 집계에는 편리하다.** 7-3절 표를 다시 보자.
- `continue` : 반복문의 **이번 회차만 건너뛰고** 다음 원소로 넘어간다. (Python과 같다)

> **잠깐 확인 ③**
> 거부 사유가 `out_of_range`, `unknown_axis` 순서로(알파벳순) 출력되는 이유는? `rejectCount`를 `unordered_map`으로 바꾸면 순서는 어떻게 될까?

---

## 9. 단계별 실습

실행은 모두 이렇게 한다.

```bash
cd ~/robot-practice/cpp-study
cmake --build build --target 07_stl_containers
./build/07_stl_containers
```

연습 코드는 새 파일 `07_stl_containers/practice.cpp`에 쓰고 이렇게 빌드한다.

```bash
cd ~/robot-practice/cpp-study/07_stl_containers
g++ -std=c++17 -Wall -Wextra practice.cpp -o practice && ./practice
```

### 해 보기 0: 실행 전 예상하기 (10분)

`main.cpp`를 보고 아래를 먼저 적는다.

| 질문 | 예상 |
|---|---|
| `[sort + 람다]`에서 출력 순서는? | |
| `[remove_if + erase]` 후 남는 값들은? | |
| `[map]`에서 `limit`과 `timeout` 중 무엇이 먼저 나오나? 값은? | |

<details>
<summary>실제 결과 (예상을 다 쓴 뒤에 펼친다)</summary>

```
[vector]
  size = 5
  3  1.5  7.2  0.4  5

[array]
  축 개수 = 2

[sort + 람다]
  0: Stop
  1: Home
  2: MoveTo

[find_if]
  찾음: Home

[remove_if + erase: 조건에 맞는 원소 지우기]
  3  1.5  0.4  5  (5.0 초과 제거)

[map: 정렬된 사전]
  limit = 2
  timeout = 2

[unordered_map: 조회]
  x_max = 100
  z_max 없음 (find/count로 확인하면 키가 새로 생기지 않는다)
```

</details>

### 해 보기 1: count_if로 세기 (10분) — 쉬움, 연습 과제 1번

`main.cpp`의 `[find_if]` 구간 바로 뒤에, `queue`에서 **priority가 1 이하인 명령의 개수**를 세서 출력한다.

```cpp
auto urgent = std::count_if(queue.begin(), queue.end(),
                            [](______ c) { return ______; });
std::cout << "  급한 명령: " << urgent << "개\n";
```

- `std::count_if`는 `find_if`와 쓰는 법이 같다. 다른 점은 **첫 원소의 위치** 대신 **true인 원소의 개수**를 돌려준다는 것뿐이다.
- 빈칸 1: 인자 타입. 5-4절 `find_if`의 람다를 참고한다.
- 빈칸 2: "priority가 1 이하"를 식으로.

### 해 보기 2: `[]` vs `at()` (10분) — 쉬움

`main.cpp`의 `[vector]` 구간 끝에 한 줄 추가하고 실행한다.

```cpp
std::cout << positions.at(10) << '\n';
```

```
terminate called after throwing an instance of 'std::out_of_range'
  what():  vector::_M_range_check: __n (which is 10) >= this->size() (which is 5)
```

1. 메시지를 읽어 본다. `__n (which is 10) >= this->size() (which is 5)` = "10번째를 달라고 했는데 크기가 5다".
2. 왜 `terminate`로 끝났을까? 04 해 보기 3을 떠올려 본다. (아무도 `catch`하지 않았다)
3. (도전) `try { ... } catch (const std::out_of_range& e) { ... }`로 감싸서, 프로그램이 죽지 않고 에러 메시지를 출력하게 만든다.

> `positions[10]`으로도 바꿔서 실행해 볼 수는 있다. 하지만 **결과를 믿으면 안 된다.** 이상한 숫자가 나와도, 아무 일 없어 보여도 버그다.

확인했으면 추가한 줄을 지운다.

### 해 보기 3: `[]`가 키를 몰래 만드는 것 확인 (10분)

`practice.cpp`에 작성한다.

```cpp
#include <iostream>
#include <string>
#include <unordered_map>

int main() {
    std::unordered_map<std::string, double> limits = {{"x_max", 100.0}, {"y_max", 50.0}};
    std::cout << "조회 전 size = " << limits.size() << '\n';

    double z = limits["z_max"];
    std::cout << "z = " << z << ", 조회 후 size = " << limits.size() << '\n';
}
```

1. 실행 전 예상: 조회 후 size는?
2. 실행해서 확인한다. `z_max`라는 키를 넣은 적이 없는데 size가 늘었다.
3. `limits` 선언 앞에 `const`를 붙이고 다시 빌드한다. 어떤 에러가 나나?
   ```
   error: passing 'const std::unordered_map<...>' as 'this' argument discards qualifiers
   ```
   01 Q3에서 본 에러와 같은 종류다. "`const` 객체에 **바꾸는** 함수(`[]`)를 부르려 했다"는 뜻이다. `[]`가 사전을 바꿀 수 있는 함수라는 증거다.
4. `[]` 대신 `find`를 써서, `z_max`가 없으면 `"z_max 설정 없음"`을 출력하도록 고친다. (5-7절 참고)

### 해 보기 4: 센서별 평균 (30분) — 연습 과제 2번

센서 이름마다 최근 측정값을 모으고, 센서별 평균을 출력한다.

```cpp
std::map<std::string, std::vector<double>> history;
```

이 타입을 먼저 읽어 본다: **키는 센서 이름(string), 값은 측정값 목록(vector<double>)**인 사전. 상자 안에 상자가 들어 있다.

**단계**

1. 측정값을 넣는다. `history["temp"]`는 무슨 타입일까? 그럼 거기에 무엇을 할 수 있을까?
   ```cpp
   history["temp"].push_back(36.5);
   history["temp"].push_back(37.1);
   history["pressure"].push_back(101.3);
   // ... 센서마다 몇 개씩 더 넣는다
   ```
   (처음 보는 센서 이름이면 `[]`가 **빈 vector**를 새로 만들어 준다. 여기서는 그게 원하는 동작이다)
2. 구조적 바인딩(5-6절)으로 센서마다 반복한다. `for (const auto& [name, values] : history)`
3. 평균을 구한다. 직접 `for`로 더해도 되고, `<numeric>`의 `std::accumulate`를 써도 된다.
   ```cpp
   double sum = std::accumulate(values.begin(), values.end(), 0.0);   // 0.0부터 시작해서 전부 더함
   ```
   **주의:** 세 번째 인자를 `0`(int)으로 쓰면 결과가 **int로 계산되어 소수점이 잘린다.** 반드시 `0.0`.
4. `name: 평균` 형식으로 출력한다.

**완료 조건**
- [ ] 센서가 이름순으로 출력된다. (왜 그런지 설명할 수 있다)
- [ ] `accumulate`의 `0.0`을 `0`으로 바꾸면 결과가 어떻게 달라지는지 확인했다.
- [ ] (도전) 값이 10개를 넘으면 가장 오래된 값을 지워서 **최근 10개만** 남긴다. (힌트: `values.erase(values.begin())` — 맨 앞 원소 지우기. 단, 반복 중인 `const auto&`로는 못 지운다. 넣을 때 처리한다)

### 해 보기 5: 반복자 무효화 (20분) — 연습 과제 3번

**(1) 눈으로 확인하기** — `practice.cpp`에 작성한다.

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<double> v = {1.0};
    std::cout << "capacity=" << v.capacity() << "  &v[0]=" << &v[0] << '\n';
    for (int i = 0; i < 4; ++i) {
        v.push_back(2.0);
        std::cout << "capacity=" << v.capacity() << "  &v[0]=" << &v[0] << '\n';
    }
}
```

- `&v[0]` : 0번째 원소의 **주소**. (02)
- capacity가 바뀌는 줄마다 `&v[0]`도 바뀌는지 본다. 6절 ② 그림의 "새 자리로 옮긴다"가 실제로 일어나는 것이다.
- capacity가 그대로인 줄에서는 주소가 그대로인지도 본다.

**(2) 진짜 버그 만들고 잡기** — 아래 코드는 **틀린 코드**다.

```cpp
std::vector<int> v;
v.push_back(1);
int& first = v[0];           // 0번째 원소의 참조(별명)
for (int i = 0; i < 100; ++i) {
    v.push_back(i);          // 재할당이 일어난다
}
std::cout << first << '\n';  // first는 무엇의 별명인가?
```

그냥 실행하면 **멀쩡해 보일 수도 있다.** (UB라서) 그래서 **AddressSanitizer**라는 검사 도구를 켜고 빌드한다.

```bash
g++ -std=c++17 -g -fsanitize=address practice.cpp -o practice && ./practice
```

```
ERROR: AddressSanitizer: heap-use-after-free ...
```

- `heap-use-after-free` = "**이미 돌려준 힙 메모리를 썼다**".
- 03 해 보기 4의 **댕글링 참조**와 같은 문제다. 원본(옛 자리)이 사라졌는데 별명(`first`)이 남아 있다.
- 고치는 방법 두 가지를 생각해 본다. (힌트 1: 인덱스는 재할당 후에도 유효하다 — 7-4절 표 / 힌트 2: `reserve`, 06)

> `-fsanitize=address`는 메모리 버그를 실행 중에 잡아 주는 컴파일 옵션이다. 앞으로 "이상한데 재현이 안 되는" 버그가 의심되면 먼저 켜 보자.

### 해 보기 6: 명령 대기열 처리기 (40분) — 실전

8절 예제를 **보지 않고** 다시 만들되, 기능을 하나씩 늘린다.

1. `MoveCommand{axis, target}`의 vector를 만들고, 8절처럼 제한값 검사 후 실행/거부를 출력한다.
2. `MoveCommand`에 `int priority`를 추가하고, 처리 **전에** 우선순위순으로 정렬한다. (5-3절)
3. 거부된 명령은 출력만 하지 말고 대기열에서 **지운다.** erase-remove 관용구를 쓴다. (5-5절)
   람다에서 제한값 표를 써야 하니 캡처가 필요하다. 표를 **복사**하지 않으려면 `[&maxPosition]`처럼 앞에 `&`를 붙여 **참조로** 캡처한다.
4. 마지막에 남은(실행할) 명령 목록과, 사유별 거부 횟수를 출력한다.

**완료 조건**
- [ ] 제한값 표는 `const`이고, `[]`를 한 번도 쓰지 않았다.
- [ ] 표에 없는 축, 음수 목표, 최대값 초과를 모두 거부한다.
- [ ] 정렬 → 거부 → 출력 순서가 지켜진다.

---

## 10. 이해도 확인 질문 3개

1. `std::remove_if`만 호출하고 `erase`를 안 하면 vector는 어떤 상태인가?
2. 설정 표에서 값을 **조회**할 때 `limits["x_max"]` 대신 `find`를 써야 하는 이유는?
3. vector의 원소를 참조로 들고 있는 동안 `push_back`을 하면 왜 위험한가? 6절 ② 그림의 몇 번 단계 때문인가?

---

## 11. 핵심 정리

- **기본 컨테이너는 `std::vector`.** 개수가 절대 안 바뀌면 `std::array`.
- 이름으로 찾으려면 사전. 정렬이 필요하면 `std::map`, 조회만 하면 `std::unordered_map`.
- STL 알고리즘은 **반복자 두 개(`begin()`, `end()`)로 범위**를 받는다. `end()`는 마지막 원소의 **다음**이다.
- 람다는 **즉석 함수**다. `[캡처](인자) { 본문 }`. 바깥 변수를 쓰려면 캡처에 적는다.
- `find`/`find_if`는 못 찾으면 `end()`를 돌려준다. **쓰기 전에 항상 `end()`와 비교한다.**
- 원소 삭제는 **erase-remove 관용구**. `remove_if`는 지우지 않고 모으기만 한다.
- 컨테이너는 모두 RAII라서 `delete`가 필요 없다. 06의 이동도 지원한다.

---

## 12. 특히 주의할 점

1. **vector의 `[]`는 범위 검사를 안 한다.** 범위 밖은 UB. 확실하지 않으면 `at()`.
2. **map의 `[]`는 없는 키를 만든다.** 조회는 `find` / `count`. 설정·제한값 표는 `const`로 만들어 실수를 컴파일 에러로 바꾼다.
3. **`end()`는 원소가 아니다.** `*queue.end()`나 `queue.end()->name`은 UB다.
4. **재할당되면 참조·포인터·반복자가 모두 무효가 된다.** 원소를 붙잡고 있는 동안 `push_back`/`erase` 하지 않는다. 특히 **반복문으로 돌면서 같은 vector에 넣거나 지우지 않는다.**
5. **`unordered_map`의 출력 순서를 믿지 않는다.** 실행 환경이 바뀌면 순서도 바뀔 수 있다.
6. **`std::accumulate`의 시작값 타입이 결과 타입이다.** double을 더하려면 `0.0`.
7. **람다 캡처 `[x]`는 복사, `[&x]`는 참조다.** 참조로 캡처한 람다를 원본보다 오래 살려 두면 댕글링 참조가 된다. (03)

---

## 13. 내 답과 메모

> 아래에 직접 적는다.

- 해 보기 0에서 틀린 곳:
- 잠깐 확인 ② (큰 숫자부터) 답:
- 질문 1 답:
- 질문 2 답:
- 질문 3 답:
- 해 보기 3에서 조회 후 size:
- 해 보기 4에서 0.0 → 0으로 바꾼 결과:
- 해 보기 5 고치는 방법 두 가지:
- 막혔던 곳 / 에러 메시지:
- 새로 이해한 것:

---

## 14. 질문과 답

> 공부하다 생긴 질문을 Claude에게 물어보고, 답을 여기에 정리한다.
