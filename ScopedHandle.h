#pragma once

#include <type_traits>
#include <functional>

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
class ScopedHandle final {
public:
    ScopedHandle(T defaultValue, std::function<void(T)>&& onExit) noexcept
        : _value(defaultValue), _defaultValue(defaultValue), _onExit(std::move(onExit)) {}
    ScopedHandle(T value, T defaultValue, std::function<void(T)>&& onExit) noexcept
        : _value(value), _defaultValue(defaultValue), _onExit(std::move(onExit)) {}
    ScopedHandle(const ScopedHandle&) = delete;
    ~ScopedHandle() {
        Release();
    }

    ScopedHandle& operator=(const ScopedHandle&) = delete;

    inline std::add_pointer_t<T> Address() {
        return std::addressof(_value);
    }

    inline T Get() {
        return _value;
    }

    inline void Release() {
        if (_value != _defaultValue) {
            _onExit(_value);
        }

        _value = _defaultValue;
    }

private:
    T _value;
    T _defaultValue;
    std::function<void(T)> _onExit;
};
