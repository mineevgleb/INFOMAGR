#include "Renderer.h"

namespace AGR {
	Renderer::Renderer(const Camera& c, const glm::vec3& backgroundColor, 
		const glm::vec2 & resolution) : m_resolution(resolution),
		m_camera(&c),
		m_backgroundColor(backgroundColor)
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

	void Renderer::addLight(Light& l)
	{
		l.m_idx = m_lights.size();
		m_lights.push_back(&l);
	}

	void Renderer::removeLight(Light& l)
	{
		if (l.m_idx > -1 && l.m_idx < m_lights.size()) {
			m_lights[l.m_idx] = *m_lights.rbegin();
			m_lights.pop_back();
			l.m_idx = -1;
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
				glm::vec3 c;
				traceRay(r, c);
				c *= 255; 
				m_image[x + y * resolution.x] = 
					(unsigned(c.b)) + (unsigned(c.g) << 8) + (unsigned(c.r) << 16);
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

	void Renderer::traceRay(const Ray& ray, glm::vec3& color)
	{
		
		color = m_backgroundColor;
		Intersection closestHit;
		if (getClosestIntersection(ray, closestHit)) {
			glm::vec3 colorSelf;
			gatherLight(closestHit, ray, colorSelf);
			color = colorSelf;
			return;
		}
		color = m_backgroundColor;
	}

	bool Renderer::getClosestIntersection(const Ray& ray, Intersection& intersect)
	{
		intersect.ray_length = FLT_MAX;
		bool wasHit = false;
		for (auto r : m_renderables) {
			std::vector<Intersection> cur;
			r->intersect(ray, cur);
			for (const auto &i : cur) {
				wasHit = true;
				if (i.ray_length > 0 && i.ray_length < intersect.ray_length) {
					intersect = i;
				}
			}
		}
		if (wasHit)
		{
			intersect.hitPt = ray.origin + ray.directon * intersect.ray_length;
		}
		return wasHit;
	}

	void Renderer::gatherLight(const Intersection &hit, const Ray& ray, glm::vec3& color)
	{
		color = glm::vec3();
		const Material* m = hit.p_object->getMaterial();
		if (m->texture) {
			glm::vec3 colorInPt;
			if (m->ambientIntensity > FLT_EPSILON || m->diffuseIntensity > FLT_EPSILON)
			{
				glm::vec2 texCoord;
				if (m->isTexCoordRequired()) {
					hit.p_object->getTexCoord(hit.hitPt, texCoord);
				}
				m->texture->getColor(texCoord, colorInPt);
			}
			color += colorInPt * m->ambientIntensity;
			if (m->specularIntensity > 0 || glm::length(colorInPt) > 0)
			{
				const float shiftValue = FLT_EPSILON * 500;
				glm::vec3 shiftedPt = hit.hitPt + hit.normal * shiftValue;
				for (auto l : m_lights) {
					glm::vec3 lightCol;
					l->getIntensityAtThePoint(shiftedPt, lightCol);
					if (glm::length(lightCol) < FLT_EPSILON) continue;
					LightRay distToLight;
					l->getDirectionToTheLight(shiftedPt, distToLight);
					Ray rayToLight;
					rayToLight.origin = shiftedPt;
					rayToLight.directon = distToLight.direction;
					Intersection intersect;
					bool isIntersect = getClosestIntersection(rayToLight, intersect);
					if (!isIntersect || intersect.ray_length > distToLight.length)
					{
						float diffuseScaler =
							glm::clamp(glm::dot(hit.normal, rayToLight.directon), 0.0f, 1.0f);
						color += colorInPt * lightCol * diffuseScaler * m->diffuseIntensity;
					}
					if (m->specularIntensity > FLT_EPSILON)
					{
						glm::vec3 reflected =
							2 * glm::dot(hit.normal, rayToLight.directon) * hit.normal 
							- rayToLight.directon;
						float specularScaler = 
							glm::clamp(glm::dot(reflected, -ray.directon), 0.0f, 1.0f);
						color += lightCol *
							glm::pow(specularScaler, m->shininess) *
							m->specularIntensity;
					}
				}
			}	
		}
		color.r = glm::clamp(color.r, 0.0f, 1.0f);
		color.g = glm::clamp(color.g, 0.0f, 1.0f);
		color.b = glm::clamp(color.b, 0.0f, 1.0f);
	}
}
