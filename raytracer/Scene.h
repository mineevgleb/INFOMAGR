#pragma once
#include <vector>
#include "Renderable.h"
#include "Camera.h"

namespace AGR {

	class Scene {
	public:
		Scene(Camera &c, unsigned long backgroundColor, const glm::vec2 &resolution);
		~Scene();
		void addRenderable(Renderable &r);
		void removeRenderable(Renderable &r);
		void render();
		void setCamera(Camera &c);
		void render(const glm::uvec2 &resolution);
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage() const;
	private:
		unsigned long traceRay(const Ray& ray);

		std::vector<Renderable *> m_renderables;
		unsigned long *m_image;
		glm::uvec2 m_resolution;
		Camera *m_camera;
		unsigned long m_backgroundColor;
	};

}