#pragma once
#include <vector>
#include "renederables\Renderable.h"
#include "Camera.h"

namespace AGR {

	class Renderer {
	public:
		Renderer(const Camera &c, unsigned long backgroundColor, const glm::vec2 &resolution);
		~Renderer();
		void addRenderable(Renderable &r);
		void removeRenderable(Renderable &r);
		void render();
		void render(const glm::uvec2 &resolution);
		const glm::uvec2 & getResolution() const;
		const unsigned long *getImage() const;
	private:
		unsigned long traceRay(const Ray& ray);

		std::vector<const Renderable *> m_renderables;
		unsigned long *m_image;
		glm::uvec2 m_resolution;
		const Camera *m_camera;
		unsigned long m_backgroundColor;
	};

}