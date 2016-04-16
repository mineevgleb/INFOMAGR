#pragma once
#include "Renderer.h"

namespace AGR {
	class Pathtracer :public Renderer
	{
	public:
		Pathtracer(const Camera &c, Sampler *skydomeTex, const glm::vec2& resolution);
		void SceneUpdated() { m_isSceneUpdated = true; }
	private:
		void Sample(Ray &r, int d);
		glm::vec3 SampleDirect(glm::vec3& pt, glm::vec3& incoming, 
			glm::vec3& normal, glm::vec3& color, 
			float *outPdf, const Material *m);
		void traceRays(std::vector<Ray> &rays) override;
		void combineImg(std::vector<glm::vec3> &buf) override;
		void updateLightsProbs();
		glm::vec3 diffuseReflection(const glm::vec3 & normal) const;
		glm::vec3 diffuseUniformReflection(const glm::vec3 & normal) const;
		glm::vec3 microfacetReflection(const glm::vec3 & normal, 
			const glm::vec3 &incoming,
			float alpha, float *outPDF) const;
		glm::vec3 calcMicrofacetBrdf(const Material *m,
			const glm::vec3& incoming, const glm::vec3& outgoing,
			const glm::vec3& normal, const glm::vec3& specCoef);
		int m_amountOfIterations = 0;
		const int MIN_PATH_LEN = 5;
		const int MAX_PATH_LEN = 128;
		std::vector<Primitive *> m_lightsForSampling;
		std::vector<float> m_lightProbs;
		bool m_isSceneUpdated;
	};
}