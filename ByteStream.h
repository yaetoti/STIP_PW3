#pragma once

#include <string>
#include <vector>

class ByteStream final {
public:
	ByteStream() = default;
	explicit ByteStream(size_t capacity);
	explicit ByteStream(const std::vector<uint8_t>& data);
	explicit ByteStream(std::vector<uint8_t>&& data);

	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	ByteStream& operator<<(const std::basic_string<T, std::char_traits<T>, std::allocator<T>>& data) {
		*this << data.length();
		const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(data.c_str());
		_buf.insert(_buf.end(), dataPtr, dataPtr + sizeof(T) * data.length());
		return *this;
	}

	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	ByteStream& operator<<(const T& data) {
		const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&data);
		_buf.insert(_buf.end(), dataPtr, dataPtr + sizeof(data));
		return *this;
	}

	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	ByteStream& operator>>(std::basic_string<T, std::char_traits<T>, std::allocator<T>>& data) {
		size_t length = 0;
		*this >> length;
		if (_readPos + sizeof(T) * length <= _buf.size()) {
			data.assign(reinterpret_cast<const T*>(_buf.data() + _readPos), length);
			_readPos += sizeof(T) * length;
		}

		return *this;
	}

	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	ByteStream& operator>>(T& data) {
		if (_readPos + sizeof(T) <= _buf.size()) {
			std::memcpy(&data, _buf.data() + _readPos, sizeof(T));
			_readPos += sizeof(T);
		}

		return *this;
	}

	const std::vector<uint8_t>& buf() const;
	std::vector<uint8_t>&& release();
	void seek(size_t readPos);

private:
	std::vector<uint8_t> _buf;
	size_t _readPos = 0;
};
