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

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// 센서 데이터를 담는 버퍼. 일부러 new[]/delete[]를 써서 복사/이동을 직접 구현한다.
class Buffer {
public:
    explicit Buffer(std::size_t size) : size_(size), data_(new int[size]{}) {
        std::cout << "  생성자 (size=" << size_ << ")\n";
    }

    ~Buffer() {
        delete[] data_;
    }

    // 복사 생성자: 새 메모리를 할당하고 내용을 전부 복사 (깊은 복사)
    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
        std::cout << "  복사 생성자 (" << size_ << "개 복사)\n";
    }

    // 이동 생성자: 포인터만 가져오고 원본은 비운다. 할당/복사가 없어서 빠르다.
    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  이동 생성자 (포인터만 옮김)\n";
    }

    // 대입 연산자는 예제를 짧게 하려고 생략(삭제)했다.
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};

Buffer makeBuffer() {
    Buffer b(1000);
    return b;  // 컴파일러가 복사/이동 자체를 생략하는 경우가 많다 (RVO/NRVO)
}

int main() {
    std::cout << "[복사]\n";
    Buffer a(1000);
    Buffer b = a;  // 복사 생성자
    std::cout << "  a.size=" << a.size() << ", b.size=" << b.size() << '\n';

    std::cout << "\n[이동]\n";
    Buffer c = std::move(a);  // 이동 생성자
    std::cout << "  a.size=" << a.size() << " (비워짐), c.size=" << c.size() << '\n';
    // 이동된 a는 소멸은 안전하지만, 값을 다시 쓰지는 않는 게 원칙이다.

    std::cout << "\n[함수에서 반환]\n";
    Buffer d = makeBuffer();  // 대부분 "생성자"만 한 번 출력된다 (복사 생략)
    std::cout << "  d.size=" << d.size() << '\n';

    std::cout << "\n[vector에 넣기]\n";
    std::vector<Buffer> buffers;
    buffers.reserve(2);  // 미리 공간 확보 (재할당 시 생기는 추가 이동을 피함)
    buffers.push_back(Buffer(10));  // 임시 객체 → 이동
    buffers.emplace_back(20);       // vector 안에서 바로 생성 → 이동도 없음

    return 0;
}

// ─── 연습 과제 ───────────────────────────────────────────────
// 1. 이동 생성자를 지워 보자. Buffer c = std::move(a); 에서 무엇이 호출되는가?
// 2. reserve(2) 줄을 지우면 출력이 어떻게 바뀌는가? 왜 그런가?
// 3. Buffer의 int* data_를 std::vector<int>로 바꿔서 Rule of Zero 버전을 만들어 보자.
//    (복사/이동/소멸자를 전부 지워도 올바르게 동작해야 한다.)
