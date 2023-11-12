#pragma once

#include <vector>
#include <memory>

struct Signature {
    std::vector<uint8_t> key;
    std::vector<uint8_t> signature;
};

std::unique_ptr<Signature> CreateDigitalSignature(const std::vector<uint8_t>& data);
