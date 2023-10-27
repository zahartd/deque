#pragma once

#include <initializer_list>
#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <cstring>

namespace deque_settings {
    constexpr size_t kBlockSize = 512 / sizeof(int);
    constexpr size_t kBufferInitMaxSize = 1 << 4;
}  // namespace deque_settings

template<size_t BlockSize>
struct Block {
private:
    int data_[BlockSize];
    size_t size_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;

public:
    Block() = default;

    Block(size_t, int);

    Block(const Block &other);

    Block &operator=(const Block &other);

    Block(Block &&) noexcept;

    Block &operator=(Block &&other) noexcept;

    void Init(size_t);

    void PushBack(int);

    void PushFront(int);

    void PopBack();

    void PopFront();

    int Get(size_t) const;

    int &Get(size_t);

    size_t Size() const;

    size_t GetHead() const;

    size_t GetTail() const;

    bool IsEmpty() const;

    bool IsFull() const;

    bool IsHeadShifted() const;

    bool IsRightClose() const;
};

template<size_t BlockSize>
bool Block<BlockSize>::IsRightClose() const {
    return head_ + size_ >= BlockSize;
}

template<size_t BlockSize>
size_t Block<BlockSize>::GetTail() const {
    return tail_;
}

template<size_t BlockSize>
size_t Block<BlockSize>::GetHead() const {
    return head_;
}

template<size_t BlockSize>
void Block<BlockSize>::Init(size_t size) {
    size_ = size;
    head_ = 0;
    tail_ = size_ - 1;
    for (size_t i = head_; i <= tail_; ++i) {
        data_[i] = 0;
    }
}

template<size_t BlockSize>
Block<BlockSize>::Block(size_t size, int filler) {
    size_ = size;
    head_ = 0;
    tail_ = size_ - 1;
    for (size_t i = head_; i <= tail_; ++i) {
        data_[i] = filler;
    }
}

template<size_t BlockSize>
Block<BlockSize> &Block<BlockSize>::operator=(Block &&other) noexcept {
    if (this != &other) {
        data_ = std::move(other.data_);
        size_ = other.size_;
        head_ = other.head_;
        tail_ = other.tail_;

        other.data_ = {};
        other.size_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }
    return *this;
}

template<size_t BlockSize>
Block<BlockSize>::Block(Block &&other) noexcept
        : data_{std::move(other.data_)}, size_{other.size_}, head_{other.head_}, tail_{other.tail_} {
    other.data_ = {};
    other.size_ = 0;
    other.head_ = 0;
    other.tail_ = 0;
}

template<size_t BlockSize>
Block<BlockSize>::Block(const Block &other)
        : size_(other.size_), head_(other.head_), tail_(other.tail_), data_(other.data_) {
}

template<size_t BlockSize>
Block<BlockSize> &Block<BlockSize>::operator=(const Block &other) {
    if (this != &other) {
        size_ = other.size_;
        head_ = other.head_;
        tail_ = other.tail_;
        std::memcpy(&data_, &other.data_, sizeof(other.data_));
    }
    return *this;
}

template<size_t BlockSize>
void Block<BlockSize>::PushFront(int value) {
    if (head_ == tail_ and head_ == 0) {
        head_ = tail_ = BlockSize - 1;
        data_[head_] = value;
    } else {
        head_ -= 1;
        data_[head_] = value;
    }
    size_ += 1;
}

template<size_t BlockSize>
void Block<BlockSize>::PushBack(int value) {
    if (head_ == tail_ and tail_ == 0 and size_ == 0) {
        data_[tail_] = value;
    } else {
        tail_ += 1;
        data_[tail_] = value;
    }
    size_ += 1;
}

template<size_t BlockSize>
void Block<BlockSize>::PopFront() {
    data_[head_] = 0;
    head_ += 1;
    size_ -= 1;
}

template<size_t BlockSize>
void Block<BlockSize>::PopBack() {
    data_[tail_] = 0;
    tail_ -= 1;
    size_ -= 1;
}

template<size_t BlockSize>
int Block<BlockSize>::Get(size_t index) const {
    return data_[head_ + index];
}

template<size_t BlockSize>
int &Block<BlockSize>::Get(size_t index) {
    return data_[head_ + index];
}

template<size_t BlockSize>
size_t Block<BlockSize>::Size() const {
    return size_;
}

template<size_t BlockSize>
bool Block<BlockSize>::IsEmpty() const {
    return size_ == 0;
}

template<size_t BlockSize>
bool Block<BlockSize>::IsFull() const {
    return size_ == BlockSize;
}

template<size_t BlockSize>
bool Block<BlockSize>::IsHeadShifted() const {
    return head_ != 0;
}

template<size_t BlockSize>
class CircularBuffer {
private:
    using DataBlock = Block<BlockSize>;
    size_t size_ = 1;
    size_t max_size_ = deque_settings::kBufferInitMaxSize;
    size_t head_ = 0;
    size_t tail_ = 0;
    std::unique_ptr<std::unique_ptr<DataBlock>[]> buffer_;

public:
    CircularBuffer();

    CircularBuffer(size_t blocks_count);

    CircularBuffer(size_t blocks_count, int filler);

    CircularBuffer(const CircularBuffer &other);

    CircularBuffer(CircularBuffer &&other) noexcept;

    CircularBuffer &operator=(CircularBuffer &&other) noexcept;

    void Swap(CircularBuffer &);

    void AddTailDataBlock();

    void AddHeadDataBlock();

    void DeleteDataBlockFromTail();

    void DeleteDataBlockFromHead();

    bool IsFull() const;

    bool IsEmpty() const;

    bool IsTailDataBlockFull() const;

    bool IsHeadDataBlockFull() const;

    DataBlock *GetTailDataBlock();

    DataBlock *GetHeadDataBlock();

    int &GetElementByIndex(size_t);

    int GetElementByIndex(size_t) const;

    void ExpandBuffer();

    void Clear();

    // Check if block is head and like this [...[], [], []]
    bool TailIsUsedHead();

    // Check if head block is like as [] or ([[], []...] or [...[], []])
    bool IsHeadStartNotShifted();

private:
    static size_t GetBlocksCount(size_t elem_count) {
        if (elem_count == 0) {
            return 1;
        }
        return (elem_count - 1) / deque_settings::kBlockSize + 1;
    }
};

template<size_t BlockSize>
CircularBuffer<BlockSize>::CircularBuffer() : buffer_(new std::unique_ptr<DataBlock>[max_size_]) {
    buffer_[0] = std::make_unique<DataBlock>();
}

template<size_t BlockSize>
CircularBuffer<BlockSize>::CircularBuffer(size_t elem_count)
        : size_(GetBlocksCount(elem_count)),
          max_size_(std::max(size_, deque_settings::kBufferInitMaxSize)),
          buffer_(new std::unique_ptr<DataBlock>[max_size_]) {
    buffer_[0] = std::make_unique<DataBlock>();
};

template<size_t BlockSize>
CircularBuffer<BlockSize>::CircularBuffer(size_t elem_count, int filler)
        : size_(GetBlocksCount(elem_count)),
          max_size_(std::max(size_, deque_settings::kBufferInitMaxSize)),
          buffer_(new std::unique_ptr<DataBlock>[max_size_]) {
    for (size_t i = 0; i < size_; ++i) {
        if (i == size_ - 1) {
            buffer_[i] = std::make_unique<DataBlock>(elem_count % BlockSize, filler);
        } else {
            buffer_[i] = std::make_unique<DataBlock>(BlockSize, filler);
        }
    }
};

template<size_t BlockSize>
CircularBuffer<BlockSize>::CircularBuffer(CircularBuffer &&other) noexcept
        : size_{other.size_},
          max_size_{other.max_size_},
          head_{other.head_},
          tail_{other.tail_},
          buffer_{std::move(other.buffer_)} {
    other.buffer_.reset();
    other.size_ = 1;
    other.max_size_ = 0;
    other.head_ = 0;
    other.tail_ = 0;
}

template<size_t BlockSize>
CircularBuffer<BlockSize> &CircularBuffer<BlockSize>::operator=(CircularBuffer &&other) noexcept {
    if (this != &other) {
        size_ = other.size_;
        max_size_ = other.max_size_;
        head_ = other.head_;
        tail_ = other.tail_;
        buffer_ = std::move(other.buffer_);

        other.buffer_.reset();
        other.size_ = 1;
        other.max_size_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }
    return *this;
}

template<size_t BlockSize>
CircularBuffer<BlockSize>::CircularBuffer(const CircularBuffer &other)
        : size_{other.size_},
          max_size_{other.max_size_},
          head_{other.head_},
          tail_{other.tail_},
          buffer_(new std::unique_ptr<DataBlock>[max_size_]) {
    for (size_t i = 0; i < other.size_; ++i) {
        buffer_[i] = std::make_unique<DataBlock>();
        *buffer_[i] = *other.buffer_[i];
    }
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::Swap(CircularBuffer &other) {
    std::swap(buffer_, other.buffer_);
    std::swap(size_, other.size_);
    std::swap(max_size_, other.max_size_);
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::AddTailDataBlock() {
    if (tail_ == max_size_ - 1) {
        tail_ = 0;
    } else {
        tail_ += 1;
    }
    buffer_[tail_] = std::make_unique<DataBlock>();
    size_ += 1;
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::AddHeadDataBlock() {
    if (head_ == 0) {
        head_ = max_size_ - 1;
    } else {
        head_ -= 1;
    }
    buffer_[head_] = std::make_unique<DataBlock>();
    size_ += 1;
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::DeleteDataBlockFromTail() {
    buffer_[tail_].reset();
    if (tail_ == head_) {
        tail_ = 0;
        head_ = 0;
        size_ = 0;
    } else {
        if (tail_ == 0) {
            tail_ = max_size_ - 1;
        } else {
            tail_ -= 1;
        }
        size_ -= 1;
    }
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::DeleteDataBlockFromHead() {
    buffer_[head_].reset();
    if (head_ == tail_) {
        tail_ = 0;
        head_ = 0;
        size_ = 0;
    } else {
        if (head_ == max_size_ - 1) {
            head_ = 0;
        } else {
            head_ += 1;
        }
        size_ -= 1;
    }
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::IsFull() const {
    return size_ == max_size_;
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::IsEmpty() const {
    return size_ == 1 and buffer_[0]->IsEmpty();
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::IsTailDataBlockFull() const {
    return buffer_[tail_]->IsFull();
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::IsHeadDataBlockFull() const {
    return buffer_[head_]->IsFull();
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::TailIsUsedHead() {
    return buffer_[tail_]->IsRightClose();
}

template<size_t BlockSize>
bool CircularBuffer<BlockSize>::IsHeadStartNotShifted() {
    return !buffer_[head_]->IsHeadShifted();
}

template<size_t BlockSize>
typename CircularBuffer<BlockSize>::DataBlock *CircularBuffer<BlockSize>::GetTailDataBlock() {
    if (!buffer_[tail_]) {
        buffer_[tail_] = std::make_unique<DataBlock>();
    }
    return buffer_[tail_].get();
}

template<size_t BlockSize>
typename CircularBuffer<BlockSize>::DataBlock *CircularBuffer<BlockSize>::GetHeadDataBlock() {
    return buffer_[head_].get();
}

template<size_t BlockSize>
int &CircularBuffer<BlockSize>::GetElementByIndex(size_t index) {
    size_t head_size = buffer_[head_]->Size();
    if (index < head_size) {
        return buffer_[head_]->Get(index);
    }
    index -= head_size;
    size_t target_block_index = (head_ + index / BlockSize + 1) % max_size_;
    size_t target_elem_in_block_index = index % BlockSize;
    return buffer_[target_block_index]->Get(target_elem_in_block_index);
}

template<size_t BlockSize>
int CircularBuffer<BlockSize>::GetElementByIndex(size_t index) const {
    size_t head_size = buffer_[head_]->Size();
    if (index < head_size) {
        return buffer_[head_]->Get(index);
    }
    index -= head_size;
    size_t target_block_index = (head_ + index / BlockSize + 1) % max_size_;
    size_t target_elem_in_block_index = index % BlockSize;
    return buffer_[target_block_index]->Get(target_elem_in_block_index);
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::ExpandBuffer() {
    size_t new_size = max_size_ * 2;
    std::unique_ptr<std::unique_ptr<DataBlock>[]> new_buffer(
            new std::unique_ptr<DataBlock>[new_size]);
    size_t cnt = 0;
    for (size_t i = head_; i != tail_; i = (i + 1) % max_size_) {
        new_buffer[cnt++] = std::move(buffer_[i]);
    }
    new_buffer[cnt] = std::move(buffer_[tail_]);
    head_ = 0;
    tail_ = size_ - 1;
    buffer_ = std::move(new_buffer);
    max_size_ = new_size;
}

template<size_t BlockSize>
void CircularBuffer<BlockSize>::Clear() {
    for (size_t i = 0; i < size_; ++i) {
        buffer_[i].reset();
    }
    buffer_[0] = std::make_unique<DataBlock>();
    size_ = 0;
    tail_ = 0;
    head_ = 0;
    max_size_ = deque_settings::kBufferInitMaxSize;
}

class Deque {
public:
    Deque() = default;

    Deque(const Deque &rhs) = default;

    Deque(Deque &&rhs) = default;

    explicit Deque(size_t size);

    Deque(std::initializer_list<int> list);

    Deque &operator=(Deque rhs);

    void Swap(Deque &rhs);

    void PushBack(int value);

    void PopBack();

    void PushFront(int value);

    void PopFront();

    int &operator[](size_t ind);

    int operator[](size_t ind) const;

    size_t Size() const;

    void Clear();

private:
    using DataBlock = Block<deque_settings::kBlockSize>;
    CircularBuffer<deque_settings::kBlockSize> data_proxy_;
    size_t size_ = 0;
};

Deque::Deque(size_t size) : data_proxy_(size, 0), size_(size) {
}

Deque::Deque(std::initializer_list<int> list) : data_proxy_(list.size()) {
    for (int el: list) {
        PushBack(el);
    }
}

Deque &Deque::operator=(Deque rhs) {
    size_ = std::move(rhs.size_);
    data_proxy_ = std::move(rhs.data_proxy_);
    return *this;
}

void Deque::Swap(Deque &rhs) {
    std::swap(size_, rhs.size_);
    data_proxy_.Swap(rhs.data_proxy_);
}

void Deque::PushBack(int value) {
    if (data_proxy_.IsFull() and data_proxy_.IsTailDataBlockFull()) {
        data_proxy_.ExpandBuffer();
    }
    if (data_proxy_.IsTailDataBlockFull() or data_proxy_.TailIsUsedHead()) {
        data_proxy_.AddTailDataBlock();
    }
    DataBlock *tail_data_block = data_proxy_.GetTailDataBlock();
    // tail_data_block is guaranteed valid block to standard PushBack
    tail_data_block->PushBack(value);
    ++size_;
}

void Deque::PopBack() {
    DataBlock *tail_data_block = data_proxy_.GetTailDataBlock();
    tail_data_block->PopBack();
    if (tail_data_block->IsEmpty() and size_ > 1) {
        data_proxy_.DeleteDataBlockFromTail();
    }
    --size_;
}

void Deque::PushFront(int value) {
    if (data_proxy_.IsFull() and data_proxy_.IsHeadDataBlockFull()) {
        data_proxy_.ExpandBuffer();
    }
    if (!data_proxy_.IsEmpty() and data_proxy_.IsHeadStartNotShifted()) {
        data_proxy_.AddHeadDataBlock();
    }
    DataBlock *head_data_block = data_proxy_.GetHeadDataBlock();
    // head_data_block is guaranteed valid block to standard PushFront
    head_data_block->PushFront(value);
    ++size_;
}

void Deque::PopFront() {
    DataBlock *head_data_block = data_proxy_.GetHeadDataBlock();
    head_data_block->PopFront();
    if (head_data_block->IsEmpty() and size_ > 1) {
        data_proxy_.DeleteDataBlockFromHead();
    }
    --size_;
}

int &Deque::operator[](size_t ind) {
    return data_proxy_.GetElementByIndex(ind);
}

int Deque::operator[](size_t ind) const {
    return data_proxy_.GetElementByIndex(ind);
}

size_t Deque::Size() const {
    return size_;
}

void Deque::Clear() {
    data_proxy_.Clear();
    size_ = 0;
}
