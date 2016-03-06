#pragma once
#include <vector>
#include "renederables/Primitive.h"
#include "renederables/Mesh.h"
#include "lights/Light.h"
#include "Camera.h"

namespace AGR {

	class Renderer {
	public:
		Renderer(const Camera &c, const glm::vec3& backgroundColor, const glm::vec2& resolution);
		~Renderer();
		void addRenderable(Primitive &r);
		void removeRenderable(Primitive &r);
		void addRenderable(Mesh& m);
		void removeRenderable(Mesh& m);
		void addLight(Light &l);
		void removeLight(Light &l);
		void render();
		void render(const glm::uvec2 &resolution);
		void testRay(glm::vec2& px, glm::vec3 &col);
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage() const;
	private:
		float traceRay(const Ray& ray, float energy, int depth, glm::vec3& color);
		bool getClosestIntersection(const Ray& ray, Intersection& intersect);
		void gatherLight(const Intersection &hit, const Ray& ray, glm::vec3& color);
		float traceRefraction(const Intersection &hit, const Ray& ray, glm::vec3& color,
			float energy, int depth, bool isLeaving);
		bool calcRefractedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2, glm::vec3& refracted) const;
		void calcReflectedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			glm::vec3& reflected) const;
		float calcReflectionComponent(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2) const;
		bool isRayLeavingObject(const glm::vec3& ray, const glm::vec3& normal) const;

		std::vector<const Primitive *> m_renderables;
		std::vector<const Light *> m_lights;
		unsigned long *m_image;
		glm::uvec2 m_resolution;
		const Camera *m_camera;
		glm::vec3 m_backgroundColor;

		const int maxRecursionDepth = 30;
		const float shiftValue = FLT_EPSILON * 500;
	};

}