// 06. 복사(copy)와 이동(move)
//
// 복사: 원본은 그대로 두고 똑같은 것을 하나 더 만든다. (큰 데이터면 비싸다)
// 이동: 원본의 자원(메모리 등)을 "훔쳐" 온다. 원본은 비어 있지만 유효한 상태가 된다.
//
// std::move(x)는 실제로 아무것도 옮기지 않는다.
// "x를 이동해도 된다"고 표시(rvalue로 캐스팅)할 뿐이고, 실제 이동은 이동 생성자가 한다.
//
// Rule of Five: 소멸자/복사 생성자/복사 대입/이동 생성자/이동 대입 중 하나를 직접 만들면
//               나머지도 신경 써야 한다.
// Rule of Zero: 가능하면 std::vector, std::unique_ptr 같은 멤버를 써서 아무것도 직접 만들지 않는다.
//               (실무에서는 이쪽이 기본이다. 아래 Buffer는 동작 원리를 보기 위한 예시다.)
//
// 자세한 설명은 같은 폴더의 README.md를 본다.

#include <cstddef>   // std::size_t
#include <iostream>
#include <utility>   // std::move
#include <vector>

// 센서 데이터를 담는 버퍼. 일부러 new[]/delete[]를 써서 복사/이동을 직접 구현한다.
//
// Buffer 객체 하나의 모양:
//
//   Buffer (스택)                힙
//   ┌──────────────┐           ┌───┬───┬───┬─────┬───┐
//   │ size_ = 1000 │           │ 0 │ 0 │ 0 │ ... │ 0 │  ← int 1000개
//   │ data_ ───────┼──────────▶└───┴───┴───┴─────┴───┘
//   └──────────────┘
//
// 객체 자체는 작다(숫자 하나 + 주소 하나). 무거운 건 data_가 가리키는 힙 쪽이다.
// "복사"는 힙 쪽까지 통째로 새로 만들고, "이동"은 주소(data_)만 넘겨준다.
class Buffer {
public:
    // 생성자: 힙에 int를 size개 만든다.
    //   new int[size]   : int 배열을 힙에 size개 만들고, 그 시작 주소를 돌려준다.
    //   {}              : 배열의 모든 칸을 0으로 채운다. (없으면 쓰레기 값이 들어 있다)
    //   std::size_t     : "개수·크기"를 담는 부호 없는(음수가 없는) 정수 타입.
    // 멤버 선언 순서(size_ → data_)와 초기화 리스트 순서를 맞췄다. (03 해 보기 2 참고)
    explicit Buffer(std::size_t size) : size_(size), data_(new int[size]{}) {
        std::cout << "  생성자 (size=" << size_ << ")\n";
    }

    // 소멸자: 생성자에서 받은 힙 메모리를 돌려준다. (04 RAII)
    //   new[]로 만든 배열은 delete가 아니라 반드시 delete[]로 지운다.
    //   data_가 nullptr이면 delete[]는 아무것도 하지 않는다. (이동된 객체가 안전하게 소멸하는 이유)
    ~Buffer() {
        delete[] data_;
    }

    // 복사 생성자: 새 메모리를 할당하고 내용을 전부 복사 (깊은 복사)
    //   언제 불리나: Buffer b = a;  처럼 "이름 있는 a를 본떠 새 b를 만들 때"
    //   const Buffer& other : 원본(other)을 복사 없이 받되, 원본은 건드리지 않겠다는 약속 (02)
    //   other.size_         : other는 private 멤버인데 접근할 수 있다.
    //                         private는 "같은 클래스 코드 안에서는" 다른 객체 것이라도 볼 수 있다.
    //   data_(new int[other.size_]) : 원본과 "다른" 새 메모리를 받는다. ← 깊은 복사의 핵심
    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];  // 칸마다 값을 하나씩 베낀다. 1000개면 1000번.
        }
        std::cout << "  복사 생성자 (" << size_ << "개 복사)\n";
    }

    // 이동 생성자: 포인터만 가져오고 원본은 비운다. 할당/복사가 없어서 빠르다.
    //   언제 불리나: Buffer c = std::move(a);  또는  Buffer(10) 같은 임시 객체로 만들 때
    //   Buffer&& other : "곧 버려질 객체(rvalue)"만 받는 참조. &가 두 개다.
    //                    &  하나 = 이름 있는 객체(계속 쓸 수 있는 것)를 받는 참조
    //                    && 두 개 = 버려질 객체를 받는 참조 → 안의 자원을 가져가도 된다
    //   noexcept       : "이 함수는 절대 예외를 던지지 않는다"는 약속.
    //                    std::vector는 이 약속이 있어야 재할당할 때 복사 대신 이동을 쓴다. (README 해 보기 4)
    //   data_(other.data_) : 새 메모리를 받지 않고, 원본의 주소를 그대로 가져온다.
    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_) {
        // 원본을 비운다. 이걸 안 하면 원본과 나, 둘 다 같은 메모리를 delete[] 하게 된다. (두 번 해제 → 충돌)
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  이동 생성자 (포인터만 옮김)\n";
    }

    // 대입 연산자는 예제를 짧게 하려고 생략(삭제)했다.
    //   대입 = "이미 있는" 객체에 덮어쓰기.  b = a;
    //   생성 = "새로" 객체를 만들기.        Buffer b = a;   (= 기호가 있지만 생성이다!)
    //   = delete : 이 함수는 없는 것으로 친다. 쓰면 컴파일 에러. (04)
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    std::size_t size() const { return size_; }

private:
    std::size_t size_;  // 칸 개수
    int* data_;         // 힙에 있는 int 배열의 시작 주소. 이 Buffer가 "소유"한다.
};

// 함수 안에서 만든 Buffer를 밖으로 돌려준다.
Buffer makeBuffer() {
    Buffer b(1000);
    return b;  // 컴파일러가 복사/이동 자체를 생략하는 경우가 많다 (RVO/NRVO)
               // 생략 = 처음부터 호출한 쪽의 변수(d) 자리에 b를 바로 만든다. 옮길 일이 없다.
}

int main() {
    std::cout << "[복사]\n";
    Buffer a(1000);  // 생성자. a가 힙 메모리 1000칸을 소유
    Buffer b = a;    // 복사 생성자. b는 "자기만의" 1000칸을 새로 받고 값을 베낀다
    std::cout << "  a.size=" << a.size() << ", b.size=" << b.size() << '\n';
    // 지금 힙에는 1000칸짜리가 2개 있다. a 것, b 것.

    std::cout << "\n[이동]\n";
    Buffer c = std::move(a);  // 이동 생성자
    // std::move(a) : a를 "버려도 되는 것"으로 표시만 한다. 이것 자체는 아무것도 옮기지 않는다.
    //                그 표시 덕분에 복사 생성자 대신 이동 생성자가 골라진다.
    // 결과: a의 1000칸을 c가 넘겨받고, a는 빈 상태(size 0, data_ nullptr)가 된다.
    std::cout << "  a.size=" << a.size() << " (비워짐), c.size=" << c.size() << '\n';
    // 이동된 a는 소멸은 안전하지만, 값을 다시 쓰지는 않는 게 원칙이다.

    std::cout << "\n[함수에서 반환]\n";
    Buffer d = makeBuffer();  // 대부분 "생성자"만 한 번 출력된다 (복사 생략)
    std::cout << "  d.size=" << d.size() << '\n';

    std::cout << "\n[vector에 넣기]\n";
    std::vector<Buffer> buffers;
    buffers.reserve(2);  // 미리 공간 확보 (재할당 시 생기는 추가 이동을 피함)
    // reserve(2) : "원소 2개가 들어갈 자리"를 미리 잡는다. 원소를 만들지는 않는다. (size는 여전히 0)

    buffers.push_back(Buffer(10));  // 임시 객체 → 이동
    // Buffer(10)은 이름 없는 임시 객체(03)다. 곧 버려지므로 std::move 없이도 이동 생성자가 골라진다.
    // 과정: ① 임시 Buffer 생성 → ② vector 안으로 이동 → ③ 빈 임시 객체는 ;에서 소멸

    buffers.emplace_back(20);       // vector 안에서 바로 생성 → 이동도 없음
    // emplace_back(20) : Buffer를 만들 "재료(20)"만 넘긴다. vector가 자기 자리에서 Buffer(20)을 바로 만든다.

    return 0;
}   // 여기서 buffers, d, c, b, a가 소멸한다. 각자 소유한 힙 메모리만 delete[] 하므로 두 번 해제가 없다.
    // (a는 이동되어 data_가 nullptr이라 delete[]가 아무것도 안 한다)

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 이동 생성자를 지워 보자. Buffer c = std::move(a); 에서 무엇이 호출되는가?
// 2. reserve(2) 줄을 지우면 출력이 어떻게 바뀌는가? 왜 그런가?
// 3. Buffer의 int* data_를 std::vector<int>로 바꿔서 Rule of Zero 버전을 만들어 보자.
//    (복사/이동/소멸자를 전부 지워도 올바르게 동작해야 한다.)
// → 단계별 실습은 README.md의 "9. 단계별 실습"을 따라간다.
