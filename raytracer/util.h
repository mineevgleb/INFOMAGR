#pragma once
#include <glm/gtc/matrix_transform.inl>

#define M_PI 3.14159265358979323846264338f

namespace AGR {

	inline void rotateMat(const glm::vec3& rotation, glm::mat4x4& out) {
		out = glm::rotate(out, rotation.y, glm::vec3(0, 1, 0));
		out = glm::rotate(out, rotation.x, glm::vec3(1, 0, 0));
		out = glm::rotate(out, rotation.z, glm::vec3(0, 0, 1));
	}

	inline void transformMat(const glm::vec3& translation, const glm::vec3& rotation,
		const glm::vec3& scale, glm::mat4x4& out) {
		out = glm::translate(out, translation);
		rotateMat(rotation, out);
		out = glm::scale(out, scale);
	}

	inline void lookAtToAngles(const glm::vec3& direction, glm::vec3& angles) {
		angles.x = glm::atan(direction.z, direction.x);
		angles.y = glm::acos(direction.y / direction.length());
	}

	inline int lzcnt64(::uint64_t num)
	{
		int guess = 0;
		int step = 64;
		do {
			step = (step + 1) >> 1;
			int newGuess = guess + step;
			if (newGuess > 0 && (UINT64_C(1) << newGuess - 1) <= num)
				guess = newGuess;
		} while (step > 1);
		return sizeof(num) * 8 - guess;
	}

	inline int lzcnt32(::uint32_t num)
	{
		int guess = 0;
		int step = 64;
		do {
			step = (step + 1) >> 1;
			int newGuess = guess + step;
			if (newGuess > 0 && (UINT32_C(1) << newGuess - 1) <= num)
				guess = newGuess;
		} while (step > 1);
		return sizeof(num) * 8 - guess;
	}

	inline int setbitcnt(unsigned int i)
	{
		i = i - ((i >> 1) & 0x55555555);
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
		return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	inline void shiftRight(::uint64_t *buf, size_t size) {
		buf[size - 1] >>= 1;
		for (int i = size - 2; i >= 0; --i) {
			buf[i + 1] |= buf[i] << 63;
			buf[i] >>= 1;
		}
	}

	inline void shiftLeft(::uint64_t *buf, size_t size) {
		buf[0] <<= 1;
		for (int i = 1; i < size; ++i) {
			buf[i - 1] |= buf[i] >> 63;
			buf[i] <<= 1;
		}
	}

	constexpr int constpow(const int base, const int pow) {
		return pow == 1 ? base : (base * constpow(base, pow - 1));
	}

}
