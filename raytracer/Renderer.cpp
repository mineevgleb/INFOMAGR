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

	void Renderer::addRenderable(Primitive& r)
	{
		r.m_idx = m_renderables.size();
		m_renderables.push_back(&r);
	}

	void Renderer::removeRenderable(Primitive& r)
	{
		if (r.m_idx > -1 && r.m_idx < m_renderables.size()) {
			m_renderables[r.m_idx] = *m_renderables.rbegin();
			m_renderables.pop_back();
			r.m_idx = -1;
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
		for (Triangle *t : m.m_triangles) {
			removeRenderable(*t);
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

	void Renderer::testRay(glm::vec2 &px, glm::vec3 &col)
	{
		glm::vec2 a(px.x / (m_resolution.x - 1), px.y / (m_resolution.y - 1));
		Ray r;
		m_camera->produceRay(a, r);
		glm::vec3 c;
		traceRay(r, 1.0, 1, c);
		col = c;
	}

	const glm::uvec2 & Renderer::getResolution() const
	{
		return m_resolution;
	}

	const unsigned long * Renderer::getImage() const
	{
		return m_image;
	}

	float Renderer::traceRay(const Ray& ray, float energy, int depth, glm::vec3& color)
	{
		color = m_backgroundColor;
		Intersection closestHit;
		bool isInObject = ray.surroundMaterial != nullptr;
		if (getClosestIntersection(ray, closestHit)) {
			glm::vec3 colorSelf;
			gatherLight(closestHit, ray, colorSelf);

			const Material* m = closestHit.p_object->getMaterial();
			glm::vec3 colorRefracted;
			float totalReflection = 0.0f;
			float totalRefraction = 0.0f;
			if (m->refractionIntensity > FLT_EPSILON) {
				if (energy > 1.0 / 255 && depth <= maxRecursionDepth) {
					totalRefraction = traceRefraction(closestHit, ray, colorRefracted,
						energy, depth, isInObject);
					if (!isInObject) {
						totalRefraction *= m->refractionIntensity;
						totalReflection = (1 - totalRefraction) * m->refractionIntensity;
					}
					else {
						totalReflection = (1 - totalRefraction);
					}
				} else {
					colorRefracted = m->innerColor;
				}
			}
			glm::vec3 colorReflected;
			totalReflection += m->reflectionIntensity;
			totalReflection = glm::clamp(totalReflection, 0.0f, 1.0f);
			if (totalReflection > FLT_EPSILON) {
				if (energy > 1.0 / 255 && depth <= maxRecursionDepth) {
					Ray reflectedRay;
					reflectedRay.origin = closestHit.hitPt + closestHit.normal * shiftValue;
					reflectedRay.surroundMaterial = ray.surroundMaterial;
					calcReflectedRay(ray.direction, closestHit.normal, reflectedRay.direction);
					traceRay(reflectedRay, energy * totalReflection, depth + 1, colorReflected);
					colorReflected *= m->reflectionColor;
				} else {
					colorReflected = m->reflectionColor;
				}
			}
			color = colorSelf + 
				colorReflected * totalReflection +
				colorRefracted * totalRefraction;
			color.r = glm::clamp(color.r, 0.0f, 1.0f);
			color.g = glm::clamp(color.g, 0.0f, 1.0f);
			color.b = glm::clamp(color.b, 0.0f, 1.0f);
			if (isInObject) {
				float remainedIntensity = glm::exp(-glm::log(m->absorption) * closestHit.ray_length);
				color *= remainedIntensity;
				color += ray.surroundMaterial->innerColor * (1 - remainedIntensity);
			}
			return closestHit.ray_length;
		}
		if (isInObject) {
			color = ray.surroundMaterial->innerColor;
		}
		else {
			color = m_backgroundColor;
		}
		return -1;
	}

	bool Renderer::getClosestIntersection(const Ray& ray, Intersection& intersect)
	{
		intersect.ray_length = FLT_MAX;
		bool wasHit = false;
		Intersection cur;
		cur.ray = ray;
		for (auto r : m_renderables) {
			if (r->intersect(cur)) {
				wasHit = true;
				if (cur.ray_length > 0 && cur.ray_length < intersect.ray_length) {
					intersect = cur;
				}
			}
		}
		if (wasHit)
		{
			intersect.p_object->getTexCoordAndNormal(intersect);
		}
		return wasHit;
	}

	void Renderer::gatherLight(const Intersection &hit, const Ray& ray, glm::vec3& color)
	{
		color = glm::vec3();
		const Material* m = hit.p_object->getMaterial();
		glm::vec3 colorInPt;
		if (m->texture && (m->ambientIntensity > FLT_EPSILON || m->diffuseIntensity > FLT_EPSILON)) {
			m->texture->getColor(hit.texCoord, colorInPt);
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
				rayToLight.direction = distToLight.direction;
				Intersection intersect;
				bool isIntersect = getClosestIntersection(rayToLight, intersect);
				if (!isIntersect || intersect.ray_length > distToLight.length) {
					float diffuseScaler =
						glm::clamp(glm::dot(hit.normal, rayToLight.direction), 0.0f, 1.0f);
					color += colorInPt * lightCol * diffuseScaler * m->diffuseIntensity;
				}
				if (m->specularIntensity > FLT_EPSILON) {
					glm::vec3 reflected =
						2 * glm::dot(hit.normal, rayToLight.direction) * hit.normal 
						- rayToLight.direction;
					float specularScaler = 
						glm::clamp(glm::dot(reflected, -ray.direction), 0.0f, 1.0f);
					color += lightCol *
						glm::pow(specularScaler, m->shininess) *
						m->specularIntensity;
				}
			}
		}	
	}

	//returns actual refraction component value (0 in case of TIR)
	float Renderer::traceRefraction(const Intersection& hit, const Ray& ray, glm::vec3& color, 
		float energy, int depth, bool isLeaving)
	{
		if (energy < 1.0 / 255 || depth > maxRecursionDepth) {
			color = glm::vec3();
			return 1;
		}
		Ray refractedRay;
		const Material* m = hit.p_object->getMaterial();
		float refrCoef1 = 1.0f, refrCoef2 = 1.0f;
		if (isLeaving) {
			refractedRay.surroundMaterial = nullptr;
			refrCoef1 = m->refractionCoeficient;
		} else {
			refrCoef2 = m->refractionCoeficient;
			refractedRay.surroundMaterial = m;
		}
		refractedRay.origin = hit.hitPt - hit.normal * shiftValue;
		float refractionComp;
		if (!calcRefractedRay(ray.direction, hit.normal, refrCoef1, 
			refrCoef2, refractedRay.direction)) {
			color = glm::vec3();
			return 0;
		} 
		if (isLeaving) {
			refractionComp = 1 - calcReflectionComponent(refractedRay.direction,
				hit.normal, refrCoef1, refrCoef2);
		} else {
			refractionComp = 1 - calcReflectionComponent(ray.direction,
				hit.normal, refrCoef1, refrCoef2);
		}
		traceRay(refractedRay, energy * refractionComp, depth + 1, color);
		return refractionComp;
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

	bool Renderer::isRayLeavingObject(const glm::vec3& ray, const glm::vec3& normal) const 
	{
		return (glm::dot(ray, normal) > 0);
	}
}
