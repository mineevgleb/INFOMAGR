#include "Pathtracer.h"
#include <random>
#include "util.h"
#include <ppl.h>

namespace AGR
{
	Pathtracer::Pathtracer(const Camera& c, const glm::vec3& backgroundColor, 
		const glm::vec2& resolution) : Renderer(c, backgroundColor, resolution, true)
	{}

	void Pathtracer::Sample(Ray& r, int d)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		if (d > MIN_PATH_LEN) {
			std::uniform_real_distribution<> dis0to1(0.0f, 1.0f);
			float probability = r.energy.x;
			probability = r.energy.y > probability ? r.energy.y : probability;
			probability = r.energy.z > probability ? r.energy.z : probability;
			probability *= (1.0 - float(d) / 200);
			float randNum = dis0to1(gen);
			if (randNum > probability)
				return;
			r.energy /= probability;
		}
		Intersection hit;
		glm::vec2 texCoord;
		glm::vec3 normal;
		if (!m_bvh.Traverse(r, hit, -1.0f)) {
			if (r.surroundMaterial) {
				*r.pixel += r.energy * r.surroundMaterial->innerColor;
				return;
			}
			*r.pixel += m_backgroundColor * r.energy;
			return;
		}
		if (r.surroundMaterial) {
			const Material *m = r.surroundMaterial;
			float remainedIntensity = glm::exp(-glm::log(m->absorption) * hit.ray_length);
			*r.pixel += r.energy * (1.0f - remainedIntensity) * m->innerColor;
			r.energy *= remainedIntensity;
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
		if (materialType < m->diffuseIntensity) {
			Ray next;
			next.origin = hit.ray_length * r.direction + r.origin + normal * shiftValue;
			next.direction = diffuseReflection(normal);
			next.pixel = r.pixel;
			next.energy = r.energy * glm::dot(next.direction, normal) * color * 2.0f;
			*r.pixel += color * m->glowIntensity * r.energy;
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
			} else {
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
			Sample(next, d + 1);
			return;
		}
		*r.pixel += color * m->glowIntensity * r.energy;
	}

	void Pathtracer::traceRays(std::vector<Ray>& rays)
	{
		concurrency::parallel_for(0, (int)rays.size(), 1, [this, &rays](int i) {
		//for (int i = 0; i < rays.size(); ++i) {
			Sample(rays[i], 0);
		});
	}

	void Pathtracer::combineImg(std::vector<glm::vec3>& buf)
	{
		float divisor = 1.0f / (m_amountOfIterations + 1);
		for (int i = 0; i < buf.size(); ++i) {
			m_highpImage[i] = m_highpImage[i] * 
				static_cast<float>(m_amountOfIterations) + buf[i];
			m_highpImage[i] *= divisor;
		}
		m_amountOfIterations++;
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
}
