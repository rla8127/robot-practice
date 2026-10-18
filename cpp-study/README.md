# C++ 개념별 예제 (C++17)

로드맵의 Phase 1(1~4주)과 동시성(10주) 개념을 하나씩 실행해 보는 예제 모음이다.
각 폴더에 `main.cpp`가 하나씩 있다. 파일 맨 위 주석에서 개념을 설명하고, 맨 아래에 **연습 과제**가 있다.

## 학습 순서

| 폴더 | 개념 | 로드맵 |
|---|---|---|
| `01_build_basics` | 전처리·컴파일·링크, 헤더/소스 분리 | 1주 |
| `02_references_pointers` | 값 / 참조 / const 참조 / 포인터 | 1주 |
| `03_object_lifetime` | 생성자·소멸자 호출 시점과 순서 | 2주 |
| `04_raii` | 자원을 객체 수명에 묶기 | 2주 |
| `05_smart_pointers` | unique_ptr, shared_ptr, weak_ptr, 소유권 | 2주 |
| `06_copy_move` | 복사 vs 이동, std::move, Rule of Five/Zero | 2주 |
| `07_stl_containers` | vector, map, 알고리즘, 람다 | 3주 |
| `08_const_optional_errors` | const, optional, 예외 vs 오류 값 반환 | 3주 |
| `09_polymorphism` | 인터페이스, 가상 함수, Mock, 의존성 주입 | 4주 |
| `10_threads_mutex` | thread, data race, mutex, lock_guard | 10주 |
| `11_atomic_condvar` | atomic, condition_variable, 명령 큐 | 10주 |

> **02 → 03 → 04 → 05 → 06**은 하나로 이어지는 흐름이다.
> "참조는 원본을 가리킨다 → 원본은 언제 사라지나 → 사라질 때 자원을 정리하자 → 메모리에 적용 → 자원을 옮기자"

## 처음 한 번: 도구 설치 (Ubuntu / WSL2)

```bash
sudo apt update
sudo apt install -y build-essential cmake gdb
```

## 빌드와 실행

```bash
cd ~/robot-practice/cpp-study
cmake -S . -B build          # 빌드 설정 생성 (처음 한 번, CMakeLists.txt를 고친 뒤)
cmake --build build          # 전체 빌드
./build/03_object_lifetime   # 원하는 예제 실행
```

예제 하나만 다시 빌드하려면 `cmake --build build --target 04_raii`를 실행한다.

## 공부하는 방법

1. 코드를 **실행하기 전에** 출력을 먼저 예상해 본다.
2. 실행하고 예상과 다른 부분을 찾는다. 그 부분이 바로 배울 점이다.
3. 주석에 `← 주석을 풀면 컴파일 에러`라고 표시된 줄을 직접 풀어 보고, 에러 메시지를 읽는다.
4. 맨 아래 연습 과제는 **새 파일에 처음부터** 작성한다. 예제 코드를 복사하지 않는다.
5. 막히면 Claude에게 코드를 보여 주고 리뷰를 받는다.
