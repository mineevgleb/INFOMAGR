#pragma once
#include "Renderer.h"

namespace AGR {
	class Pathtracer :public Renderer
	{
	public:
		Pathtracer(const Camera &c, const glm::vec3& backgroundColor, const glm::vec2& resolution);
	private:
		void Sample(Ray &r, int d);
		void traceRays(std::vector<Ray> &rays) override;
		void combineImg(std::vector<glm::vec3> &buf) override;
		glm::vec3 diffuseReflection(const glm::vec3 & normal) const;
		int m_amountOfIterations = 0;
		const int MIN_PATH_LEN = 5;
	};
}