#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>

#include "array_ptr.h"


class ReserveProxyObj {
public:    
    explicit ReserveProxyObj (size_t capacity_to_reserve)
        :capacity_(capacity_to_reserve)
    {    
    }
    
    size_t ReserveCapasity(){
        return capacity_;
    }
    
private:    
    size_t capacity_; 
 
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : vector_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(&vector_[0], &vector_[size_], 0);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : size_(size), capacity_(size)
    {
        ArrayPtr<Type> tmp(size_);
        std::fill(tmp.Get(), tmp.Get() + size_, value);
        vector_.swap(tmp);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    :   size_(init.size()), capacity_(init.size())
    {
        int i = 0;
        ArrayPtr<Type> tmp(size_);
        for (const Type& element: init) {
            tmp[i++] = element;
        }
        vector_.swap(tmp);
    }

    SimpleVector(const SimpleVector& other) 
    : size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        SimpleVector tmp(other.GetSize());

        std::copy(other.begin(), other.end(), tmp.begin());

        swap(tmp);
    }

    SimpleVector(SimpleVector&& other) {
        vector_ = std::move(other.vector_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector( ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.ReserveCapasity() );
    }

    SimpleVector& operator= (const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);

            this->swap(tmp);
        }

        return *this;
    }

    SimpleVector& operator= (SimpleVector&& rhs) = default;

    
    void PushBack(const Type& item) {
        this->Resize(size_ + 1);
        this->vector_[size_ - 1] = item;
    }

    void PushBack(Type&& item) {
        this->Resize(size_ + 1);
        this->vector_[size_ - 1] = std::move(item); 
    }

    void PopBack() noexcept {
        if (!IsEmpty())
            --size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        auto insertable = pos - cbegin();
        if (capacity_ > size_) {
            Insert(insertable, value);
            ++size_;
        } else {
            this->Resize(size_ + 1);
            Insert(insertable, value);
        }

        return &vector_[insertable];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        auto insertable = pos - cbegin();
        if (capacity_ > size_) {
            Insert(insertable, std::move(value));
            ++size_;
        } else {
            Resize(size_ + 1);
            Insert(insertable, std::move(value));
        }

        return &vector_[insertable];
    }

    Iterator Erase(ConstIterator pos) {
        if (size_) {
            auto* it = begin() + (pos - cbegin());
            std::move((it + 1), end(), it);
            --size_;
        }
        
        return &vector_[pos - begin()];
    }

    void swap(SimpleVector& other) noexcept {
        vector_.swap(other.vector_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }


    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index < size_) {
            return vector_[index]; 
        } else {
            throw std::out_of_range("");
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index < size_) {
            return vector_[index]; 
        } else {
            throw std::out_of_range("");
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            capacity_= std::max(new_size, capacity_ * 2);

            ArrayPtr<Type> tmp(capacity_);
            
            std::move(&vector_[0], &vector_[size_], tmp.Get());

            vector_.swap(tmp);

        } else {
            for (auto i = size_; i < new_size; ++i) {
                vector_[i] = std::move(Type{});
            }
        }

        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        
        ArrayPtr<Type> tmp(new_capacity);
        for (std::size_t i{}; i < size_; ++i){
            tmp[i] = std::move(vector_[i]);
        }
            vector_.swap(tmp);
            capacity_ = std::exchange(new_capacity, 0);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return vector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return vector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{vector_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{vector_.Get() + size_};

    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{vector_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{vector_.Get() + size_};
    }
    
private:
    void Insert(int index, const Type& value) {
        std::copy_backward(begin() + index, end(), end() + 1);
        vector_[index] = value;
    }
    
    void Insert(int index, Type&& value) {
        std::move_backward(begin() + index, end(), end() + 1);
        vector_[index] = std::move(value);
    }
    

    ArrayPtr<Type> vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
    
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
