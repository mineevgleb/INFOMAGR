#include "Renderer.h"

namespace AGR {
	Renderer::Renderer(const Camera& c, const glm::vec3& backgroundColor, 
		const glm::vec2 & resolution) : m_resolution(resolution),
		m_camera(&c),
		m_backgroundColor(backgroundColor)
	{
		m_image = new unsigned long[m_resolution.x * m_resolution.y];
		m_highpImage = new glm::vec3[m_resolution.x * m_resolution.y];

		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0) {
			throw std::runtime_error("No OpenCL platforms found");
		}
		cl::Platform platformToUse = platforms[0];
		std::vector<cl::Device> devices;
		platformToUse.getDevices(CL_DEVICE_TYPE_GPU, &devices);
		if (devices.size() == 0) {
			throw std::runtime_error("No OpenCL GPU devices found for default platform");
		}
		m_deviceToUse = devices[0];
		m_context = new cl::Context({ m_deviceToUse });
		m_bvh = new BVH(m_context, &m_deviceToUse);
	}

	Renderer::~Renderer()
	{
		delete m_bvh;
		delete m_context;
		delete[] m_image;
	}

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

	void Renderer::addLight(Light& l)
	{
		l.m_idx = static_cast<int>(m_lights.size());
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
		m_bvh->constructAgglomerative(m_primitives);
		if (resolution != m_resolution) {
			delete[] m_image;
			delete[] m_highpImage;
			m_image = new unsigned long[m_resolution.x * m_resolution.y];
			m_highpImage = new glm::vec3[m_resolution.x * m_resolution.y];
			m_resolution = resolution;
		}
		static std::vector<Ray> raysToIntersect;
		raysToIntersect.resize(m_resolution.x * m_resolution.y);
		for (size_t y = 0; y < m_resolution.y; ++y) {
			for (size_t x = 0; x < resolution.x; ++x) {
				glm::vec2 curPixel(static_cast<float>(x) / (m_resolution.x - 1),
					static_cast<float>(y) / (m_resolution.y - 1));
				m_camera->produceRay(curPixel, raysToIntersect[x + y * m_resolution.x]);
				raysToIntersect[x + y * m_resolution.x].pixel = &m_highpImage[x + y * m_resolution.x];
				raysToIntersect[x + y * m_resolution.x].surroundMaterial = nullptr;
				raysToIntersect[x + y * m_resolution.x].energy = glm::vec3(1.0f);
				m_highpImage[x + y * m_resolution.x] = glm::vec3(0.0f);
			}
		}
		traceRays(raysToIntersect, false);
		for (size_t y = 0; y < m_resolution.y; ++y) {
			for (size_t x = 0; x < resolution.x; ++x) {
				glm::vec3& c = m_highpImage[x + y * resolution.x];
				c = glm::clamp(c, 0.0f, 1.0f);
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

	void Renderer::testRay(int x, int y)
	{
		std::vector<Ray> r(1);
		glm::vec2 curPixel(static_cast<float>(x) / (m_resolution.x - 1),
			static_cast<float>(y) / (m_resolution.y - 1));
		m_camera->produceRay(curPixel, r[0]);
		r[0].pixel = &m_highpImage[x + y * m_resolution.x];
		r[0].surroundMaterial = nullptr;
		r[0].energy = glm::vec3(1.0f);
		traceRays(r, true);
	}

	void Renderer::traceRays(std::vector<Ray>& rays, bool test)
	{
		static std::vector<Intersection> intersections;
		static std::vector<glm::vec3> normals;
		static std::vector<glm::vec2> texCoords;
		intersections.resize(rays.size());
		for (int i = 0; i < maxRecursionDepth && rays.size() > 0; ++i) {
			m_bvh->PacketTraverse(rays, intersections);
			processMissedRays(rays, intersections);
			normals.resize(intersections.size());
			texCoords.resize(intersections.size());
			for (int j = 0; j < intersections.size(); ++j) {
				intersections[j].p_object->getTexCoordAndNormal(rays[j],
					intersections[j].ray_length, texCoords[j], normals[j]);
				/*if (test && i > 0) {
					for (int k = 0; k <= 30; ++k) {
						Material* m = new Material();
						m->reflectionIntensity = 1;
						m->reflectionColor = glm::vec3(0);
						float itp = ((float)k) / 30;
						glm::vec3 pt = intersections[j].ray.origin * (1 - itp) + intersections[j].hitPt * itp; 
						float r = k == 0 ? 0.05f: 0.01f;
						Sphere *s = new Sphere(*m, pt, r);
						addRenderable(*s);
					}
				}*/
				Ray& r = rays[j];
				if (r.surroundMaterial) {
					const Material *m = r.surroundMaterial;
					float remainedIntensity = glm::exp(-glm::log(m->absorption) * intersections[j].ray_length);
					*r.pixel += r.energy * (1.0f - remainedIntensity) * m->innerColor;
					r.energy *= remainedIntensity;
				}
			}
			gatherLight(rays, intersections, texCoords, normals);
			int initialAm = rays.size();
			int curAm = 0;
			for (int j = 0; j < initialAm; ++j) {
				Ray refl, refr;
				produceSecondaryRays(rays[j], intersections[j], normals[j], refl, refr);
				if (refl.pixel) {
					rays[curAm++] = refl;
				}
				if (refr.pixel) {
					rays.push_back(refr);
				}
			}
			if (curAm != initialAm) {
				for (int j = initialAm; j < rays.size(); ++j) {
					rays[curAm++] = rays[j];
				}
				rays.resize(curAm);
			}
		}
	}

	void Renderer::processMissedRays(std::vector<Ray> &rays, std::vector<Intersection>& intersections) const
	{
		int amount = static_cast<int>(intersections.size());
		int i = 0;
		while (i < amount) {
			if (intersections[i].ray_length < 0) {
				Ray &r = rays[i];
				if (r.surroundMaterial) {
					*r.pixel += r.energy * r.surroundMaterial->innerColor;
				} else {
					*r.pixel += r.energy * m_backgroundColor;
				}
				intersections[i] = intersections[--amount];
				rays[i] = rays[--amount];
			} else {
				++i;
			}
		}
		intersections.resize(amount);
		rays.resize(amount);
	}

	void Renderer::gatherLight(std::vector<Ray> &rays, std::vector<Intersection>& intersections,
		std::vector<glm::vec2> &texCoords, std::vector<glm::vec3> &normals)
	{
		static std::vector<glm::vec3> surfaceColors;
		static std::vector<Ray> raysToLights;
		static std::vector<int> hitPtForRays;
		static std::vector<float> lengthsFromLights;
		static std::vector<bool> occlusionFlags;
		size_t raysAm = 0;
		surfaceColors.resize(intersections.size());
		normals.resize(intersections.size());
		raysToLights.resize(intersections.size() * m_lights.size());
		lengthsFromLights.resize(intersections.size() * m_lights.size());
		hitPtForRays.resize(intersections.size() * m_lights.size());

		for (int i = 0; i < intersections.size(); ++i) {
			Ray &r = rays[i];
			const Material *m = intersections[i].p_object->getMaterial();
			if (m->texture) {
				glm::vec2 texCoord;
				m->texture->getColor(texCoords[i], surfaceColors[i]);
				*r.pixel += r.energy * m->ambientIntensity * surfaceColors[i];
			} else {
				continue;
			}

			if (m->specularIntensity > FLT_EPSILON || m->diffuseIntensity > FLT_EPSILON) {
				for (int j = 0; j < m_lights.size(); ++j) {
					glm::vec3 lightAtPt;
					glm::vec3 hitPt = rays[i].direction * intersections[i].ray_length + rays[i].origin;
					m_lights[j]->getIntensityAtThePoint(hitPt, lightAtPt);
					if (lightAtPt.x > 1.0f / 255.0f || lightAtPt.y > 1.0f / 255.0f 
						|| lightAtPt.z > 1.0f / 255.0f) {
						LightRay lr;
						m_lights[j]->getDirectionToTheLight(hitPt, lr);
						raysToLights[raysAm].direction = lr.direction;
						raysToLights[raysAm].origin =
							hitPt + normals[i] * shiftValue;
						raysToLights[raysAm].energy = lightAtPt;
						hitPtForRays[raysAm] = i;
						lengthsFromLights[raysAm++] = lr.length;
					}
				}
			}

		}
		/*raysToLights.resize(raysAm);
		lengthsFromLights.resize(raysAm);
		occlusionFlags.resize(raysAm);
		m_bvh->PacketCheckOcclusions(raysToLights, lengthsFromLights, occlusionFlags);

		for (int i = 0; i < raysAm; ++i) {
			if (!occlusionFlags[i]) {
				Intersection& hit = intersections[hitPtForRays[i]];
				const Material *m = hit.p_object->getMaterial();
				float diffuseScaler =
					glm::clamp(glm::dot(hit.normal, raysToLights[i].direction), 0.0f, 1.0f);
				*hit.ray.pixel +=
					hit.ray.energy * diffuseScaler * m->diffuseIntensity *
					raysToLights[i].energy * surfaceColors[hitPtForRays[i]];
				if (m->specularIntensity > FLT_EPSILON) {
					glm::vec3 reflected =
						2 * glm::dot(hit.normal, raysToLights[i].direction) * hit.normal
						- raysToLights[i].direction;
					float specularScaler =
						glm::clamp(glm::dot(reflected, -hit.ray.direction), 0.0f, 1.0f);
					*hit.ray.pixel += hit.ray.energy *  raysToLights[i].energy *
						glm::pow(specularScaler, m->shininess) * m->specularIntensity;
				}
			}
		}*/
	}

	void Renderer::produceSecondaryRays(Ray& r, Intersection& hit, glm::vec3& normal,
		Ray& reflected, Ray& refracted) const
	{
		const Material* m = hit.p_object->getMaterial();
		glm::vec3 refrEnergy = m->refractionIntensity * r.energy;
		glm::vec3 reflEnergy = m->reflectionIntensity * r.energy;
		glm::vec3 hitPt = r.direction * hit.ray_length + r.origin;
		float frenselRefl = 0.0f;
		if(refrEnergy.x > 1.0f / 255.0f || refrEnergy.y > 1.0f / 255.0f 
			|| refrEnergy.z > 1.0f / 255.0f) {
			refracted.surroundMaterial = r.surroundMaterial ? nullptr : m;
			float refrCoef1 = r.surroundMaterial ? m->refractionCoeficient : 1.0f;
			float refrCoef2 = r.surroundMaterial ? 1.0f : m->refractionCoeficient;
			if (calcRefractedRay(r.direction, normal, refrCoef1, refrCoef2, refracted.direction)) {
				refracted.origin = hitPt - normal * shiftValue;
				if (r.surroundMaterial) {
					frenselRefl = calcReflectionComponent(refracted.direction,
						normal, refrCoef1, refrCoef2);
				}
				else {
					frenselRefl = calcReflectionComponent(r.direction,
						normal, refrCoef1, refrCoef2);
				}
				glm::vec3 pureRefrEnergy = refrEnergy * (1.0f - frenselRefl);
				if (pureRefrEnergy.x > 1.0f / 255.0f || pureRefrEnergy.y > 1.0f / 255.0f
					|| pureRefrEnergy.z > 1.0f / 255.0f) {
					refracted.pixel = r.pixel;
					refracted.energy = pureRefrEnergy;
				}
			} else {
				frenselRefl = 1.0f;
			}
		}
		reflEnergy += refrEnergy * frenselRefl;
		reflEnergy *= m->reflectionColor;
		if (reflEnergy.x > 1.0f / 255.0f || reflEnergy.y > 1.0f / 255.0f
			|| reflEnergy.z > 1.0f / 255.0f) {
			reflected.origin = hitPt + normal * shiftValue;
			reflected.surroundMaterial = r.surroundMaterial;
			calcReflectedRay(r.direction, normal, reflected.direction);
			reflected.energy = reflEnergy;
			reflected.pixel = r.pixel;
		}
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
