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
				traceRay(r, 1.0, 1, c);
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

	void Renderer::traceRay(const Ray& ray, float energy, int depth, glm::vec3& color)
	{
		if (energy < 1.0 / 255 || depth > maxRecursionDepth)
		{
			color = glm::vec3();
			return;
		}
		color = m_backgroundColor;
		Intersection closestHit;
		if (getClosestIntersection(ray, closestHit)) {
			glm::vec3 colorSelf;
			gatherLight(closestHit, ray, colorSelf);

			const Material* m = closestHit.p_object->getMaterial();
			glm::vec3 colorRefracted;
			float realReflection = 0.0f;
			float realRefraction = 0.0f;
			if (m->refractionIntensity > FLT_EPSILON) {
				realReflection = calcReflectionComponent(ray.directon,
					closestHit.normal, 1.0f, m->refractionCoeficient);
				realRefraction = (1 - realReflection) * m->refractionIntensity;
				realReflection *= m->refractionIntensity;
				traceRefraction(closestHit, ray, colorRefracted,
					energy * realRefraction, depth + 1);
			}
			glm::vec3 colorReflected;
			realReflection += m->reflectionIntensity;
			realReflection = glm::clamp(realReflection, 0.0f, 1.0f);
			if (realReflection > FLT_EPSILON) {
				Ray reflectedRay;
				reflectedRay.origin = closestHit.hitPt + closestHit.normal * shiftValue;
				calcReflectedRay(ray.directon, closestHit.normal, reflectedRay.directon);
				traceRay(reflectedRay, energy * realReflection, depth + 1, colorReflected);
			}
			if (realReflection > 0.3) {
				color = glm::vec3(1, 0, 0);
			}
			color = colorSelf + 
				colorReflected * realReflection +
				colorRefracted * realRefraction;
			color.r = glm::clamp(color.r, 0.0f, 1.0f);
			color.g = glm::clamp(color.g, 0.0f, 1.0f);
			color.b = glm::clamp(color.b, 0.0f, 1.0f);
			return;
		}
		color = m_backgroundColor;
	}

	bool Renderer::getClosestIntersection(const Ray& ray, Intersection& intersect)
	{
		intersect.ray_length = FLT_MAX;
		bool wasHit = false;
		for (auto r : m_renderables) {
			Intersection cur;
			if (r->intersect(ray, cur)) {
				wasHit = true;
				if (cur.ray_length > 0 && cur.ray_length < intersect.ray_length) {
					intersect = cur;
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
		glm::vec3 colorInPt;
		if (m->texture && (m->ambientIntensity > FLT_EPSILON || m->diffuseIntensity > FLT_EPSILON)) {
			glm::vec2 texCoord;
			if (m->isTexCoordRequired()) {
				hit.p_object->getTexCoord(hit.hitPt, texCoord);
			}
			m->texture->getColor(texCoord, colorInPt);
		}
		color += colorInPt * m->ambientIntensity;
		if (m->specularIntensity > 0 || glm::length(colorInPt) > 0) {
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
				if (!isIntersect || intersect.ray_length > distToLight.length) {
					float diffuseScaler =
						glm::clamp(glm::dot(hit.normal, rayToLight.directon), 0.0f, 1.0f);
					color += colorInPt * lightCol * diffuseScaler * m->diffuseIntensity;
				}
				if (m->specularIntensity > FLT_EPSILON) {
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

	void Renderer::traceRefraction(const Intersection& hit, const Ray& ray, glm::vec3& color, 
		float energy, int depth)
	{
		if (energy < 1.0 / 255 || depth > maxRecursionDepth) {
			color = glm::vec3();
			return;
		}
		const Material* m = hit.p_object->getMaterial();
		Ray refractedRay;
		refractedRay.origin = hit.hitPt - hit.normal * shiftValue;
		if (!calcRefractedRay(ray.directon, hit.normal, 1.0f, 
			m->refractionCoeficient, refractedRay.directon)) {
			color = glm::vec3();
			return;
		}
		Intersection exitPt;
		hit.p_object->intersect(refractedRay, exitPt);
		exitPt.hitPt =
			refractedRay.origin + refractedRay.directon * exitPt.ray_length;
		Ray refractedRay2;
		refractedRay2.origin = 
			exitPt.hitPt + exitPt.normal * shiftValue;
		if (!calcRefractedRay(refractedRay.directon, -exitPt.normal,
			m->refractionCoeficient, 1.0f, refractedRay2.directon)) {
			color = m->innerColor;
			return;
		}
		float distanceTraveled = glm::length(hit.hitPt - exitPt.hitPt);
		float remainedIntensity = glm::exp(-glm::log(m->absorption) * distanceTraveled);
		glm::vec3 furtherTraceColor;
		traceRay(refractedRay2, remainedIntensity * energy, depth, furtherTraceColor);
		color = furtherTraceColor * remainedIntensity + m->innerColor * (1 - remainedIntensity);
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
		float angleCos = glm::dot(-incomingRay, normal);
		return r0 + (1 - r0) * glm::pow(1 - angleCos, 5.0f);
	}
}
