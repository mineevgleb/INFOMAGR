#pragma once
#include <glm\glm.hpp>
#include "Intersection.h"

namespace AGR {

	class Camera {
	public:
		Camera(float aspectRatio, float horFOV, const glm::vec3& position = glm::vec3(),
		       const glm::vec3& rotation = glm::vec3());

		//Ray through the pixel with coordinates X and Y. Coordinates are in range [0, 1].
		void produceRay(const glm::vec2 position, Ray &out) const;


		void lookAt(glm::vec3 point, float rollAngle);

		float getAspectRatio() const;
		void setAspectRatio(float aspect_ratio);

		float getHorFOV() const;
		void setHorFOV(float horFOV);

		const glm::vec3& getPosition() const;
		void setPosition(const glm::vec3& position);

		const glm::vec3& getRotation() const;
		void setRotation(const glm::vec3& rotation);

		void setLensParams(float focalDist, float apertureSize, 
			int edgesAm = -1, float roatation = 0.0f);

	private:
		void updateVectors();

		float m_aspectRatio;
		float m_horFov;
		glm::vec3 m_position;
		glm::vec3 m_rotation;

		glm::vec3 m_frontVec;
		glm::vec3 m_rightVec;
		glm::vec3 m_upVec;

		float m_focalDist = 1.0f;
		float m_apertureSize = 0.0f;
		int m_edgesAm = -1;
		float m_lensRotation;
	};

}
