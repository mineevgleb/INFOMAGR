#pragma once
#include <vector>
#include <map>
#include <CL/cl.hpp>
#include "renederables/Primitive.h" 
#include "renederables/Mesh.h"
#include "lights/Light.h"
#include "Camera.h"
#include "BVH.h"

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
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage() const;
		void testRay(int x, int y);
	private:
		void traceRays(std::vector<Ray> &rays, bool test);
		void processMissedRays(std::vector<Ray> &rays, std::vector<Intersection> &intersections) const;
		void gatherLight(std::vector<Ray> &rays, std::vector<Intersection> &intersections,
			std::vector<glm::vec2> &texCoords, std::vector<glm::vec3> &normals);
		void produceSecondaryRays(Ray& r, Intersection& hit, glm::vec3& normal, 
			Ray& reflected, Ray& refracted) const;
		bool calcRefractedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2, glm::vec3& refracted) const;
		void calcReflectedRay(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			glm::vec3& reflected) const;
		float calcReflectionComponent(const glm::vec3 &incomingRay, const glm::vec3 &normal,
			float n1, float n2) const;

		std::vector<Primitive *> m_primitives;
		std::map<Primitive *, int> m_primitivesIndices;
		std::vector<const Light *> m_lights;
		unsigned long *m_image;
		glm::vec3 *m_highpImage;
		glm::uvec2 m_resolution;
		const Camera *m_camera;
		glm::vec3 m_backgroundColor;
		BVH *m_bvh;

		const int maxRecursionDepth = 10;
		const float shiftValue = FLT_EPSILON * 500;

		cl::Context *m_context;
		cl::Device m_deviceToUse;
	};

}