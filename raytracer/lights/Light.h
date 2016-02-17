#pragma once
#include <glm/glm.hpp>

namespace AGR{
	class Light
	{
	public:
		virtual void getDirectionToTheLight(const glm::vec3 &pt, glm::vec3& dir) = 0;
		virtual void getIntensityAtThePoint(const glm::vec3 &pt, glm::vec3& col) = 0;
		virtual ~Light(){}
	};
}