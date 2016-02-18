#pragma once
#include <vector>
#include "renederables/Renderable.h"
#include "lights/Light.h"
#include "Camera.h"

namespace AGR {

	class Renderer {
	public:
		Renderer(const Camera &c, const glm::vec3& backgroundColor, const glm::vec2& resolution);
		~Renderer();
		void addRenderable(Renderable &r);
		void removeRenderable(Renderable &r);
		void addLight(Light &l);
		void removeLight(Light &l);
		void render();
		void render(const glm::uvec2 &resolution);
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage() const;
	private:
		void traceRay(const Ray& ray, float energy, int depth, glm::vec3& color);
		bool getClosestIntersection(const Ray& ray, Intersection& intersect);
		void gatherLight(const Intersection &hit, const Ray& ray, glm::vec3& color);

		std::vector<const Renderable *> m_renderables;
		std::vector<const Light *> m_lights;
		unsigned long *m_image;
		glm::uvec2 m_resolution;
		const Camera *m_camera;
		glm::vec3 m_backgroundColor;

		const int maxRecursionDepth = 5;
		const float shiftValue = FLT_EPSILON * 500;
	};

}