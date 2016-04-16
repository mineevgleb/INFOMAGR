#pragma once
#include <vector>
#include <map>
#include "renederables/Primitive.h" 
#include "renederables/Mesh.h"
#include "Camera.h"
#include "BVH.h"
#include "renederables/Sphere.h"

namespace AGR {

	class Renderer {
	public:
		Renderer(const Camera &c, Sampler *skydomeTex, const glm::vec2& resolution,
			bool useAntialiasing);
		virtual ~Renderer();
		void addRenderable(Primitive &r);
		void removeRenderable(Primitive &r);
		void addRenderable(Mesh& m);
		void removeRenderable(Mesh& m);
		void render();
		void render(const glm::uvec2 &resolution);
		void setSkydomeAngle(float angle);
		void testRay(int x, int y);
		void setGammaCorrection(bool correct, float gamma = 2.2f, float exposure = 1.0f);
		void setSepia(bool enabled);
		void setVignetting(bool enabled, float alpha = 1.0f);
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage();
	protected:
		virtual void traceRays(std::vector<Ray> &rays) = 0;
		virtual void combineImg(std::vector<glm::vec3> &buf) = 0;
		bool calcRefractedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2, glm::vec3& refracted) const;
		void calcReflectedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			glm::vec3& reflected) const;
		float calcReflectionComponent(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2) const;

		std::vector<Primitive *> m_primitives;
		std::map<Primitive *, int> m_primitivesIndices;
		std::vector<unsigned long> m_image;
		std::vector<glm::vec3> m_highpImage;
		glm::uvec2 m_resolution;
		const Camera *m_camera;
		BVH m_bvh;
		bool m_useAntialiasing;
		const float shiftValue = FLT_EPSILON * 500;

		bool m_correctGamma = false;
		float m_gamma = 2.2f;
		float m_exposureScaler = 1.0f;

		bool m_sepia = false;

		bool m_vignetting = false;
		float m_vignettingAlpha = 1.0f;

		Sphere *m_skydome;
		Material m_skyMat;
	};

}
