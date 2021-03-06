#include "Pathtracer.h"
#include <random>
#include "util.h"
#include <ppl.h>

namespace AGR
{
	Pathtracer::Pathtracer(const Camera& c, Sampler *skydomeTex,
		const glm::vec2& resolution) :
		Renderer(c, skydomeTex, resolution, true),
		m_isSceneUpdated(true)
	{}

	void Pathtracer::Sample(Ray& r, int d)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		if (d > MIN_PATH_LEN) {
			if (d > MAX_PATH_LEN) return;
			std::uniform_real_distribution<> dis0to1(0.0f, 1.0f);
			float probability = r.energy.x;
			probability = r.energy.y > probability ? r.energy.y : probability;
			probability = r.energy.z > probability ? r.energy.z : probability;
			probability *= (1.0f - float(d));
			float randNum = dis0to1(gen);
			if (randNum > probability)
				return;
			r.energy /= probability;
		}
		Intersection hit;
		glm::vec2 texCoord;
		glm::vec3 normal;
		if (!m_bvh.Traverse(r, hit, -1.0f)) {
			r.origin = glm::vec3();
			float dist = m_skydome->intersect(r);
			glm::vec2 texcoord;
			glm::vec3 n;
			m_skydome->getTexCoordAndNormal(r, dist, texcoord, n);
			glm::vec3 color;
			m_skydome->getMaterial()->texture->getHDRColor(texcoord, color);
			float maxComp = color.r > color.g && color.r != INFINITY ? color.r : color.g;
			maxComp = maxComp > color.b && maxComp != INFINITY ? maxComp : color.b;
			const float maxEdge = 80.0f;
			if (maxComp == INFINITY) maxComp = maxEdge;
			if (color.r == INFINITY || color.g == INFINITY || color.b == INFINITY) {
				if (color.r == INFINITY) color.r = maxComp;
				if (color.g == INFINITY) color.g = maxComp;
				if (color.b == INFINITY) color.b = maxComp;
			}
			if (maxComp > maxEdge) color = color / maxComp * maxEdge;
			if (r.surroundMaterial) {
				*r.pixel += r.energy * r.surroundMaterial->innerColor * color;
				return;
			}
			
			*r.pixel += color * r.energy;
			return;
		}
		if (r.surroundMaterial) {
			const Material *m = r.surroundMaterial;
			float remainedIntensity = glm::exp(-glm::log(m->absorption) * hit.ray_length);
			r.energy -= r.energy * (1.0f - remainedIntensity) * (1.0f - m->innerColor);
		}
		const Material *m = hit.p_object->getMaterial();
		hit.p_object->getTexCoordAndNormal(r, hit.ray_length, texCoord, normal);
		std::uniform_real_distribution<> dis(0.0f, 
			m->diffuseIntensity + m->reflectionIntensity + m->refractionIntensity);
		float materialType = dis(gen);
		glm::vec3 result = glm::vec3();
		glm::vec3 color = glm::vec3(1, 1, 1);
		if (m->texture)
			m->texture->getColor(texCoord, color);
		if (!m->isMicrofacet) {
			if (materialType < m->diffuseIntensity) {
				glm::vec3 brdf = color / M_PI;
				Ray next;
				next.origin = hit.ray_length * r.direction + r.origin + normal * shiftValue;
				next.direction = diffuseReflection(normal);
				next.pixel = r.pixel;
				next.energy = r.energy * brdf * M_PI;
				next.surroundMaterial = r.surroundMaterial;
				float lPdf;
				glm::vec3 directColor = SampleDirect(next.origin, r.direction, normal, color, &lPdf, m);
				float brdfPdf = glm::dot(next.direction, normal) / M_PI;
				*r.pixel += directColor * r.energy * (lPdf / (lPdf + brdfPdf));
				*r.pixel += color * m->glowIntensity * r.energy * (brdfPdf / (lPdf + brdfPdf));
				Sample(next, d + 1);
				return;
			}
			materialType -= m->diffuseIntensity;
			float trueReflection = m->reflectionIntensity;
			float trueRefraction = m->refractionIntensity;
			if (materialType < m->refractionIntensity) {
				Ray next;
				next.surroundMaterial = r.surroundMaterial ? nullptr : m;
				float refrCoef1 = r.surroundMaterial ? m->refractionCoeficient : 1.0f;
				float refrCoef2 = r.surroundMaterial ? 1.0f : m->refractionCoeficient;
				next.origin = hit.ray_length * r.direction + r.origin - normal * shiftValue;
				if (calcRefractedRay(r.direction, normal, refrCoef1, refrCoef2, next.direction)) {
					float frenselRefl;
					if (r.surroundMaterial) {
						frenselRefl = calcReflectionComponent(next.direction,
							normal, refrCoef1, refrCoef2);
					}
					else {
						frenselRefl = calcReflectionComponent(r.direction,
							normal, refrCoef1, refrCoef2);
					}
					trueRefraction -= frenselRefl;
					trueReflection += frenselRefl;
					if (materialType < trueRefraction) {
						next.energy = r.energy;
						next.pixel = r.pixel;
						Sample(next, d + 1);
						return;
					}
				}
				else {
					trueReflection += m->refractionIntensity;
				}
			}
			materialType -= trueRefraction;
			if (materialType < trueReflection) {
				Ray next;
				next.origin = hit.ray_length * r.direction + r.origin + normal * shiftValue;
				calcReflectedRay(r.direction, normal, next.direction);
				next.energy = r.energy * color;
				next.pixel = r.pixel;
				next.surroundMaterial = r.surroundMaterial;
				Sample(next, d + 1);
				return;
			}
			*r.pixel += color * m->glowIntensity * r.energy;
		} else {
			Ray next;
			next.origin = hit.ray_length * r.direction + r.origin + normal * shiftValue;
			float brdfPdf = 0.0f;
			next.direction = microfacetReflection(normal, r.direction, m->microfacetAlpha, &brdfPdf);
			if (glm::dot(next.direction, normal) < 0) return;
			next.pixel = r.pixel;
			glm::vec3 brdf = calcMicrofacetBrdf(m, r.direction, next.direction, normal, color);
			if (brdf.x < FLT_EPSILON && brdf.y < FLT_EPSILON && brdf.z < FLT_EPSILON) return;
			next.energy = r.energy * brdf * glm::dot(normal, next.direction) / brdfPdf;
			next.surroundMaterial = r.surroundMaterial;
			float lPdf;
			glm::vec3 directColor = SampleDirect(next.origin, r.direction, normal, color, &lPdf, m);
			*r.pixel += directColor * r.energy * (lPdf / (lPdf + brdfPdf));
			*r.pixel += color * m->glowIntensity * r.energy * (brdfPdf / (lPdf + brdfPdf));
			if (isnan(r.pixel->x))
				r.pixel->x = 1;
			Sample(next, d + 1);
		}
	}

	glm::vec3 Pathtracer::SampleDirect(glm::vec3& pt, glm::vec3& incoming,
		glm::vec3& normal, glm::vec3& color,
		float *outPdf, const Material *m)
	{
		if (m_lightsForSampling.size() == 0) return glm::vec3();
		const int attempts = 5;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, m_lightsForSampling.size() - 1);
		int lightIdx = dist(gen);
		Primitive *light = m_lightsForSampling[lightIdx];
		glm::vec3 ptOnLight = light->getRandomPoint();
		Ray r;
		float distToLight = glm::distance(ptOnLight, pt);


		r.origin = pt;
		r.direction = (ptOnLight - pt) / distToLight;
		Intersection hit;
		glm::vec2 texCoord;
		glm::vec3 lightNormal;
		light->getTexCoordAndNormal(r, distToLight, texCoord, lightNormal);
		if (glm::dot(normal, r.direction) > 0 && glm::dot(lightNormal, -r.direction) > 0) {
			if (!m_bvh.Traverse(r, hit, distToLight - 1.0) || hit.p_object == light) {
				glm::vec3 brdf;
				if (m->isMicrofacet) {
					brdf = calcMicrofacetBrdf(m, incoming, r.direction, normal, color);
				} else {
					brdf = color / M_PI;
				}
				float solidAngle = light->calcSolidAngle(pt);
				const Material *m = light->getMaterial();
				glm::vec3 col;
				m->texture->getColor(texCoord, col);
				glm::vec3 color = m->glowIntensity * col * brdf * solidAngle * glm::dot(r.direction, normal) /
					(m_lightProbs[lightIdx] / m_lightProbs.size());
				*outPdf = 1.0f / solidAngle;
				return m->glowIntensity * col * brdf * solidAngle * glm::dot(r.direction, normal) /
					(m_lightProbs[lightIdx] / m_lightProbs.size());
			}
		}
		return glm::vec3(0);
	}

	void Pathtracer::traceRays(std::vector<Ray>& rays)
	{
		if (m_isSceneUpdated) {
			m_bvh.construct(m_primitives);
			updateLightsProbs();
			m_isSceneUpdated = false;
		}
		concurrency::parallel_for(0, (int)rays.size(), 1, [this, &rays](int i) {
		//for (int i = 0; i < rays.size(); ++i) {
			Sample(rays[i], 0);
		});
	}

	void Pathtracer::combineImg(std::vector<glm::vec3>& buf)
	{
		float divisor = 1.0f / (m_amountOfIterations + 1);
		for (int i = 0; i < buf.size(); ++i) {
			glm::vec3 asd = m_highpImage[i];
			asd += glm::vec3();
			m_highpImage[i] = m_highpImage[i] * 
				static_cast<float>(m_amountOfIterations) + buf[i];
			m_highpImage[i] *= divisor;
		}
		m_amountOfIterations++;
	}

	void Pathtracer::updateLightsProbs()
	{
		float maxProb = 0.0f;
		static std::vector<Primitive *> lights;
		static std::vector<float> probs;
		lights.reserve(m_primitives.size());
		lights.resize(0);
		probs.reserve(m_primitives.size());
		probs.resize(0);
		for (int i = 0; i < m_primitives.size(); ++i) {
			const Material *m = m_primitives[i]->getMaterial();
			if (m->glowIntensity > 0.01) {
				lights.push_back(m_primitives[i]);
				float prob = m->glowIntensity * m_primitives[i]->getArea();
				probs.push_back(prob);
				if (prob > maxProb) maxProb = prob;
			}
		}
		if (lights.size() == 0) return;
		m_lightsForSampling.resize(0);
		m_lightProbs.resize(0);
		m_lightsForSampling.reserve(lights.size() * 200);
		m_lightProbs.reserve(lights.size() * 200);
		for (int i = 0; i < lights.size(); ++i) {
			float probFrac = probs[i] / maxProb;
			int slotsToOccupy = glm::ceil(probFrac * 200);
			for (int j = 0; j < slotsToOccupy; ++j) {
				m_lightsForSampling.push_back(lights[i]);
				m_lightProbs.push_back(slotsToOccupy);
			}
		}
	}

	glm::vec3 Pathtracer::diffuseReflection(const glm::vec3& normal) const
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> dis(0.0f, 1.0f);
		float r1 = dis(gen) * 2 * M_PI, r2 = dis(gen), r2s = sqrt(r2);
		glm::vec3 side1 = glm::normalize(glm::cross(
			glm::abs(normal.x) > FLT_EPSILON ?
			glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0),
			normal));
		glm::vec3 side2 = glm::normalize(glm::cross(side1, normal));
		return glm::normalize(
			side1 * sin(r1) * r2s + side2 * cos(r1) * r2s + normal * sqrt(1.0f - r2)
		);
	}

	glm::vec3 Pathtracer::diffuseUniformReflection(const glm::vec3& normal) const
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::normal_distribution<> dis;
		glm::vec3 local = glm::normalize(glm::vec3(dis(gen), dis(gen), dis(gen)));
		local.z = glm::abs(local.z);
		glm::vec3 side1 = glm::normalize(glm::cross(
			glm::abs(normal.x) > FLT_EPSILON ?
			glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0),
			normal));
		glm::vec3 side2 = glm::normalize(glm::cross(side1, normal));
		return glm::normalize(
			side1 * local.x + side2 * local.y + normal * local.z
			);
	}

	glm::vec3 Pathtracer::microfacetReflection(const glm::vec3& normal, const glm::vec3& incoming, 
		float alpha, float* outPDF) const
	{
		glm::vec3 result;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0f, 1.0f);
		float r0 = dis(gen);
		float phi = dis(gen) * 2 * M_PI;
		float costheta = glm::pow(r0, 1.0f / (1.0f + alpha));
		float sintheta = glm::sqrt(1.0f - costheta * costheta);
		glm::vec3 halfwayLocal = glm::vec3(sintheta * glm::cos(phi),
			sintheta * glm::sin(phi), costheta);
		glm::vec3 side1 = glm::normalize(glm::cross(
			glm::abs(normal.x) > FLT_EPSILON ?
			glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0),
			normal));
		glm::vec3 side2 = glm::normalize(glm::cross(side1, normal));
		glm::vec3 halfwayGlobal = glm::normalize(
			side1 * halfwayLocal.x + side2 * halfwayLocal.y + normal * halfwayLocal.z
			);
		*outPDF = (alpha + 2.0f) * glm::pow(glm::dot(halfwayGlobal, normal), alpha + 1) /
			(8 * M_PI * glm::dot(-incoming, halfwayGlobal));
		result = 2 * glm::dot(-incoming, halfwayGlobal) * halfwayGlobal + incoming;
		return result;
	}

	glm::vec3 Pathtracer::calcMicrofacetBrdf(const Material* m, const glm::vec3& incoming, 
		const glm::vec3& outgoing, const glm::vec3& normal, const glm::vec3& specCoef)
	{
		glm::vec3 halfway = glm::normalize(outgoing - incoming);
		float NdotH = glm::dot(normal, halfway);
		float VdotH = glm::dot(-incoming, halfway);
		float NdotL = glm::dot(normal, outgoing);
		float NdotV = glm::dot(normal, -incoming);
		float distrib = ((m->microfacetAlpha + 2) / (2 * M_PI)) *
			glm::pow(NdotH, m->microfacetAlpha);

		float geometryTerm = glm::min(1.0f,
			glm::min(
				2 * NdotH * NdotV / VdotH,
				2 * NdotH * NdotL / VdotH
				)
			);
		float scaler = 1 - glm::dot(outgoing, halfway);
		float scalerPow5 = scaler * scaler;
		scalerPow5 *= scalerPow5;
		scalerPow5 *= scaler;
		glm::vec3 frenselTerm = specCoef + (1.0f - specCoef) * scalerPow5;
		glm::vec3 brdf = frenselTerm * geometryTerm * distrib / (4 * NdotL * NdotH);
		return brdf;
	}
}
