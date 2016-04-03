#include "Pathtracer.h"
#include <random>
#include "util.h"
#include <ppl.h>

namespace AGR
{
	Pathtracer::Pathtracer(const Camera& c, const glm::vec3& backgroundColor, 
		const glm::vec2& resolution) : Renderer(c, backgroundColor, resolution)
	{}

	glm::vec3 Pathtracer::Sample(Ray& r, int d)
	{
		if (d > 2) return glm::vec3();
		Intersection hit;
		glm::vec2 texCoord;
		glm::vec3 normal;
		if (!m_bvh.Traverse(r, hit, -1.0f)) return m_backgroundColor;
		const Material *m = hit.p_object->getMaterial();
		hit.p_object->getTexCoordAndNormal(r, hit.ray_length, texCoord, normal);
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0f, 
			m->diffuseIntensity + m->reflectionIntensity + m->refractionIntensity);
		float materialType = dis(gen);
		glm::vec3 result = glm::vec3();
		glm::vec3 color;
		m->texture->getColor(texCoord, color);
		if (materialType < m->diffuseIntensity) {
			Ray next;
			next.origin = hit.ray_length * r.direction + r.origin + normal * shiftValue;
			next.direction = diffuseReflection(normal);
			result += Sample(next, d + 1) * glm::dot(next.direction, normal) * color * 2.0f;
		}
		return result + color * m->glowIntensity;
	}

	void Pathtracer::traceRays(std::vector<Ray>& rays)
	{
		concurrency::parallel_for(0, (int)rays.size(), 1, [this, &rays](int i) {
		//for (int i = 0; i < rays.size(); ++i) {
			*rays[i].pixel = Sample(rays[i], 0);
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
