#pragma once

#include <cstdint>
#include <CL/sycl.hpp>
#include <utility>  

// Container with subset of functionality of std::vector. Size is constant and
// determined at compile time to allow for use in SYCL kernels.
template<typename Type, uint32_t MaxSize>
class ConstSizeVector final
{
    using AccessorType = sycl::accessor<Type>;
public:
    explicit ConstSizeVector() = default;
    explicit ConstSizeVector(const ConstSizeVector<Type, MaxSize>& other);
    explicit ConstSizeVector(const AccessorType& accessor);
    ConstSizeVector(ConstSizeVector&& other) = delete;

    ~ConstSizeVector() = default;

    ConstSizeVector& operator=(const ConstSizeVector<Type, MaxSize>& other);
    ConstSizeVector& operator=(ConstSizeVector&& other) = delete;

    inline const Type& operator[](uint32_t index) const;
    inline Type& operator[](uint32_t index);

    inline const Type& front() const;
    inline Type& front();
    inline const Type& back() const;
    inline Type& back();

    inline uint32_t getSize() const;
    inline constexpr uint32_t getMaxSize() const;
    inline bool isEmpty() const;

    inline void clear();
    inline void pushBack(const Type& value);
    inline void pushBack(Type&& value);
    inline void popBack();

    void copyToAccessor(const AccessorType& accessor, uint32_t start, uint32_t end);
    void copyToAccessor(const AccessorType& accessor, uint32_t end);
    void copyToAccessor(const AccessorType& accessor);

private:
    uint32_t size = 0;
    Type elements[MaxSize];
};

template<typename Type, uint32_t MaxSize>
ConstSizeVector<Type, MaxSize>::ConstSizeVector(const ConstSizeVector<Type, MaxSize>& other)
: size(other.size)
{
    size = other.size;
    for(uint32_t i = 0; i < size; ++i)
    {
        elements[i] = other.elements[i];
    }
}

template<typename Type, uint32_t MaxSize>
ConstSizeVector<Type, MaxSize>::ConstSizeVector(const AccessorType& accessor)
{
    size = accessor.size();
    for(uint32_t i = 0; i < size; ++i)
    {
        elements[i] = accessor[i];
    }
}

template<typename Type, uint32_t MaxSize>
ConstSizeVector<Type, MaxSize>& ConstSizeVector<Type, MaxSize>::operator=(const ConstSizeVector<Type, MaxSize>& other)
{
    size = other.size;
    for(uint32_t i = 0; i < other.size; ++i)
    {
        elements[i] = other.elements[i];
    }

    return *this;
}

template<typename Type, uint32_t MaxSize>
inline const Type& ConstSizeVector<Type, MaxSize>::operator[](uint32_t index) const
{
    return elements[index];
}

template<typename Type, uint32_t MaxSize>
inline Type& ConstSizeVector<Type, MaxSize>::operator[](uint32_t index)
{
    return elements[index];
}

template<typename Type, uint32_t MaxSize>
inline const Type& ConstSizeVector<Type, MaxSize>::front() const
{
    return elements[0];
}

template<typename Type, uint32_t MaxSize>
inline Type& ConstSizeVector<Type, MaxSize>::front()
{
    return elements[0];
}

template<typename Type, uint32_t MaxSize>
inline const Type& ConstSizeVector<Type, MaxSize>::back() const
{
    return elements[size - 1];
}

template<typename Type, uint32_t MaxSize>
inline Type& ConstSizeVector<Type, MaxSize>::back()
{
    return elements[size - 1];
}

template<typename Type, uint32_t MaxSize>
inline uint32_t ConstSizeVector<Type, MaxSize>::getSize() const
{
    return size;
}

template<typename Type, uint32_t MaxSize>
inline constexpr uint32_t ConstSizeVector<Type, MaxSize>::getMaxSize() const
{
    return MaxSize;
}

template<typename Type, uint32_t MaxSize>
inline bool ConstSizeVector<Type, MaxSize>::isEmpty() const
{
    return size == 0;
}

template<typename Type, uint32_t MaxSize>
inline void ConstSizeVector<Type, MaxSize>::clear()
{
    size = 0;
}

template<typename Type, uint32_t MaxSize>
inline void ConstSizeVector<Type, MaxSize>::pushBack(const Type& value)
{
    elements[size] = value;
    ++size;
}

template<typename Type, uint32_t MaxSize>
inline void ConstSizeVector<Type, MaxSize>::pushBack(Type&& value)
{
    elements[size] = std::move(value);
    ++size;
}

template<typename Type, uint32_t MaxSize>
inline void ConstSizeVector<Type, MaxSize>::popBack()
{
    --size;
}

template<typename Type, uint32_t MaxSize>
void ConstSizeVector<Type, MaxSize>::copyToAccessor(const AccessorType& accessor, uint32_t start, uint32_t end)
{
    for(uint32_t i = 0; i < end - start; ++i)
    {
        accessor[i] = elements[i + start];
    }
}

template<typename Type, uint32_t MaxSize>
void ConstSizeVector<Type, MaxSize>::copyToAccessor(const AccessorType& accessor, uint32_t end)
{
    copyToAccessor(accessor, 0, end);
}

template<typename Type, uint32_t MaxSize>
void ConstSizeVector<Type, MaxSize>::copyToAccessor(const AccessorType& accessor)
{
    copyToAccessor(accessor, 0, size);
}