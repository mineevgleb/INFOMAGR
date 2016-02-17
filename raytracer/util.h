#pragma once

#define M_PI 3.14159265358979323846264338f

namespace AGR {

	void rotateMat(const glm::vec3& rotation, glm::mat4x4& out);

	void transformMat(const glm::vec3& translation, const glm::vec3& rotation,
		const glm::vec3& scale, glm::mat4x4& out);

	void lookAtToAngles(const glm::vec3& direction, glm::vec3& angles);
}
