#include "ScopeGuard.h"

ScopeGuard::ScopeGuard(std::function<void()>&& onExit) noexcept
	: _onExit(std::move(onExit)) { }

ScopeGuard::~ScopeGuard() noexcept {
    if (!_cancelled) {
        _onExit();
    }
}

void ScopeGuard::Cancel() noexcept {
    _cancelled = true;
}
