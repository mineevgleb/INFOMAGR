#pragma once
#include <glm/glm.hpp>

namespace AGR{
	struct LightRay
	{
		glm::vec3 direction;
		float length;
	};

	class Light
	{
	friend class Renderer;
	public:
		Light(): m_idx(-1){}
		virtual void getDirectionToTheLight(const glm::vec3 &pt, LightRay& dir) const = 0;
		virtual void getIntensityAtThePoint(const glm::vec3 &pt, glm::vec3& col) const = 0;
		virtual ~Light(){}
	private:
		//index of an object in the Renderer's vector
		int m_idx;
	};
}