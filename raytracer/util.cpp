#include <glm/gtc/matrix_transform.inl>
#include "util.h"

namespace AGR {

	void rotateMat(const glm::vec3& rotation, glm::mat4x4& out) {
		out = glm::rotate(out, rotation.y, glm::vec3(0, 1, 0));
		out = glm::rotate(out, rotation.x, glm::vec3(1, 0, 0));
		out = glm::rotate(out, rotation.z, glm::vec3(0, 0, 1));
	}

	void transformMat(const glm::vec3& translation, const glm::vec3& rotation,
		const glm::vec3& scale, glm::mat4x4& out) {
		out = glm::translate(out, translation);
		rotateMat(rotation, out);
		out = glm::scale(out, scale);
	}

	void lookAtToAngles(const glm::vec3& direction, glm::vec3& angles) {
		angles.x = glm::atan(direction.z, direction.x);
		angles.y = glm::acos(direction.y / direction.length());
	}

	int lzcnt(::uint64_t num)
	{
		int guess = 0;
		int step = 64;
		do {
			step = (step + 1) >> 1;
			int newGuess = guess + step;
			if (newGuess > 0 && (UINT64_C(1) << newGuess - 1) <= num)
				guess = newGuess;
		} while (step > 1);
		return 64 - guess;
	}
}
