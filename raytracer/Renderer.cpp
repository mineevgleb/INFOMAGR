#include "Renderer.h"
#include <random>

namespace AGR {
	Renderer::Renderer(const Camera& c, const glm::vec3& backgroundColor,
		const glm::vec2 & resolution, bool useAntialiasing) : m_image(resolution.x * resolution.y),
		m_highpImage(resolution.x * resolution.y),
		m_resolution(resolution),
		m_camera(&c),
		m_backgroundColor(backgroundColor),
		m_useAntialiasing(useAntialiasing)
	{}

	void Renderer::addRenderable(Primitive& r)
	{
		if (m_primitivesIndices.try_emplace(&r, m_primitives.size()).second) {
			m_primitives.push_back(&r);
		}
	}

	void Renderer::removeRenderable(Primitive& r)
	{
		auto it = m_primitivesIndices.find(&r);
		if (it != m_primitivesIndices.end()) {
			m_primitivesIndices.insert_or_assign(*m_primitives.rbegin(), it->second);
			m_primitives[it->second] = *m_primitives.rbegin();
			m_primitives.pop_back();
			m_primitivesIndices.erase(it);
		}
	}

	void Renderer::addRenderable(Mesh& m)
	{
		for (Triangle *t : m.m_triangles) {
			addRenderable(*t);
		}
	}

	void Renderer::removeRenderable(Mesh& m)
	{
		for (int i = 0; i < m.m_triangles.size(); ++i) {
			removeRenderable(*m.m_triangles[i]);
		}
	}

	void Renderer::render()
	{
		render(m_resolution);
	}

	void Renderer::render(const glm::uvec2 & resolution)
	{
		static std::vector<glm::vec3> tmp_buf(resolution.x * resolution.y);
		m_bvh.construct(m_primitives);
		if (resolution != m_resolution) {
			m_image.resize(m_resolution.x * m_resolution.y);
			m_highpImage.resize(m_resolution.x * m_resolution.y);
			tmp_buf.resize(m_resolution.x * m_resolution.y);
			m_resolution = resolution;
		}
		memset(&tmp_buf[0], 0, tmp_buf.size() * sizeof(tmp_buf[0]));
		static std::vector<Ray> raysToIntersect;
		raysToIntersect.resize(m_resolution.x * m_resolution.y);

		glm::vec2 pixelSize(1.0f / (m_resolution.x - 1), 1.0f / (m_resolution.y - 1));
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis0to1(0.0f, 1.0f);

		for (size_t y = 0; y < m_resolution.y; ++y) {
			for (size_t x = 0; x < resolution.x; ++x) {
				glm::vec2 curPixel(static_cast<float>(x) / (m_resolution.x - 1),
					static_cast<float>(y) / (m_resolution.y - 1));
				if (m_useAntialiasing) {
					curPixel.x += (dis0to1(gen) - 0.5f) * pixelSize.x;
					curPixel.y += (dis0to1(gen) - 0.5f) * pixelSize.y;
				}
				m_camera->produceRay(curPixel, raysToIntersect[x + y * m_resolution.x]);
				raysToIntersect[x + y * m_resolution.x].pixel = &tmp_buf[x + y * m_resolution.x];
				raysToIntersect[x + y * m_resolution.x].surroundMaterial = nullptr;
				raysToIntersect[x + y * m_resolution.x].energy = glm::vec3(1.0f);
			}
		}
		traceRays(raysToIntersect);
		combineImg(tmp_buf);
	}

	void Renderer::testRay(int x, int y)
	{
		std::vector<Ray> r(1);
		glm::vec2 curPixel(static_cast<float>(x) / (m_resolution.x - 1),
			static_cast<float>(y) / (m_resolution.y - 1));
		m_camera->produceRay(curPixel, r[0]);
		r[0].pixel = &m_highpImage[x + y * m_resolution.x];
		r[0].surroundMaterial = nullptr;
		r[0].energy = glm::vec3(1.0f);
		traceRays(r);
	}

	const glm::uvec2 & Renderer::getResolution() const
	{
		return m_resolution;
	}

	const unsigned long * Renderer::getImage()
	{
		for (int i = 0; i < m_highpImage.size(); ++i) {
			glm::vec3 c = m_highpImage[i];
			c = glm::clamp(c, 0.0f, 1.0f);
			c *= 255;
			m_image[i] =
				(unsigned(c.b)) + (unsigned(c.g) << 8) + (unsigned(c.r) << 16);
		}
		return &m_image[0];
	}

	bool Renderer::calcRefractedRay(const glm::vec3& incomingRay, const glm::vec3& normal,
		float n1, float n2, glm::vec3& refracted) const
	{
		float angleCos = glm::dot(normal, -incomingRay);
		float scaler = n1 / n2;
		float k = 1.0f - (scaler * scaler) * (1.0f - angleCos * angleCos);
		if (k < 0) {
			return false;
		}
		refracted =
			glm::normalize(scaler * incomingRay + normal * (scaler * angleCos - glm::sqrt(k)));
		return true;
	}

	void Renderer::calcReflectedRay(const glm::vec3& incomingRay, const glm::vec3& normal,
		glm::vec3& reflected) const
	{
		reflected =
			2 * glm::dot(normal, -incomingRay) * normal
			+ incomingRay;
	}

	float Renderer::calcReflectionComponent(const glm::vec3& incomingRay, const glm::vec3& normal,
		float n1, float n2) const
	{
		float r0 = (n1 - n2) / (n1 + n2);
		r0 *= r0;
		float angleCos = glm::abs(glm::dot(-incomingRay, normal));
		return r0 + (1 - r0) * glm::pow(1 - angleCos, 5.0f);
	}
}
