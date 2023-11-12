#pragma once

#include <functional>

class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()>&& onExit) noexcept;
    explicit ScopeGuard(const ScopeGuard& other) = delete;
    ScopeGuard& operator=(const ScopeGuard& other) = delete;
    ~ScopeGuard() noexcept;

    void Cancel() noexcept;

private:
    std::function<void()> _onExit;
    bool _cancelled = false;
};

