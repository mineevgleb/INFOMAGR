#include "Scene.h"
#include <limits>

namespace AGR {
	Scene::Scene(Camera& c, unsigned long backgroundColor, const glm::vec2 & resolution)
		: m_camera(&c),
		m_backgroundColor(backgroundColor),
		m_resolution(resolution)
	{
		m_image = new unsigned long[m_resolution.x * m_resolution.y];
	}

	Scene::~Scene()
	{
		delete[] m_image;
	}

	void Scene::addRenderable(Renderable& r)
	{
		r.m_idx = m_renderables.size();
		m_renderables.push_back(&r);
	}

	void Scene::removeRenderable(Renderable& r)
	{
		if (r.m_idx > -1 && r.m_idx < m_renderables.size()) {
			m_renderables[r.m_idx] = *m_renderables.rbegin();
			m_renderables.pop_back();
			r.m_idx = 0;
		}
	}

	void Scene::render()
	{
		render(m_resolution);
	}

	void Scene::render(const glm::uvec2 & resolution)
	{
		if (resolution != m_resolution) {
			delete[] m_image;
			m_image = new unsigned long[m_resolution.x * m_resolution.y];
			m_resolution = resolution;
		}
		for (int y = 0; y < m_resolution.y; ++y) {
			for (int x = 0; x < resolution.x; ++x) {
				Ray r;
				glm::vec2 curPixel(static_cast<float>(x) / (m_resolution.x - 1),
					static_cast<float>(y) / (m_resolution.y - 1));
				m_camera->produceRay(curPixel, r);
				m_image[x + y * resolution.x] = traceRay(r);
			}
		}
	}

	const glm::uvec2 & Scene::getResolution() const
	{
		return m_resolution;
	}

	const unsigned long * Scene::getImage() const
	{
		return m_image;
	}

	unsigned long Scene::traceRay(const Ray & ray)
	{
		Intersection min;
		bool wasHit = false;
		for (auto r : m_renderables) {
			std::vector<Intersection> cur;
			r->intersect(ray, cur);
			for (const auto &i : cur) {
				wasHit = true;
				if (i.ray_length > 0 && i.ray_length < min.ray_length) {
					min = i;
				}
			}
		}
		unsigned long color = m_backgroundColor;
		if (wasHit) {
			float normalized = glm::min(((min.ray_length - 6) / 5) * 255, 255.0f);
			color = 255 - static_cast<unsigned long>(round(normalized));
			color = color + (color << 8) + (color << 16);
		}
		return color;
	}
}