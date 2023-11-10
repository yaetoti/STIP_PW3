#include "ByteStream.h"

ByteStream::ByteStream(size_t capacity)
	: _buf(capacity) { };

const std::vector<uint8_t>& ByteStream::buf() const {
	return _buf;
}

std::vector<uint8_t>&& ByteStream::release() {
	return std::move(_buf);
}

void ByteStream::seek(size_t readPos) {
	_readPos = readPos;
}
