#include "Renderer.h"
#include <limits>

namespace AGR {
	Renderer::Renderer(const Camera& c, unsigned long backgroundColor, const glm::vec2 & resolution)
		: m_camera(&c),
		m_backgroundColor(backgroundColor),
		m_resolution(resolution)
	{
		m_image = new unsigned long[m_resolution.x * m_resolution.y];
	}

	Renderer::~Renderer()
	{
		delete[] m_image;
	}

	void Renderer::addRenderable(Renderable& r)
	{
		r.m_idx = m_renderables.size();
		m_renderables.push_back(&r);
	}

	void Renderer::removeRenderable(Renderable& r)
	{
		if (r.m_idx > -1 && r.m_idx < m_renderables.size()) {
			m_renderables[r.m_idx] = *m_renderables.rbegin();
			m_renderables.pop_back();
			r.m_idx = -1;
		}
	}

	void Renderer::render()
	{
		render(m_resolution);
	}

	void Renderer::render(const glm::uvec2 & resolution)
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

	const glm::uvec2 & Renderer::getResolution() const
	{
		return m_resolution;
	}

	const unsigned long * Renderer::getImage() const
	{
		return m_image;
	}

	unsigned long Renderer::traceRay(const Ray & ray)
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
			glm::vec3 hitPt = ray.origin + ray.directon * min.ray_length;
			glm::vec2 texCoord;
			if (min.p_object->getMaterial()->isTexCoordRequired()) {
				min.p_object->getTexCoord(hitPt, texCoord);
			}
			glm::vec3 ambientCol;
			min.p_object->getMaterial()->getAmbientColor()->getColor(texCoord, ambientCol);
			ambientCol *= min.p_object->getMaterial()->getAmbientIntensity() * 255;
			color = unsigned(ambientCol.b) + (unsigned(ambientCol.g) << 8) + (unsigned(ambientCol.r) << 16);
		}
		return color;
	}
}