#include "Triangle.h"
#include "../util.h"

namespace AGR
{
	Triangle::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
		Material& m, bool useTextureCoords, bool useVertNormals, bool limit):
		Primitive(m),
		m_useTextureCoords(useTextureCoords),
		m_useVertNormals(useVertNormals),
		m_limit(limit)
	{
		m_vert[0] = v1;
		m_vert[1] = v2;
		m_vert[2] = v3;
		commitTransformations();
	}

	bool Triangle::intersect(Intersection& intersect) const
	{
		float denom = glm::dot(intersect.ray.direction, m_normal);
		if (glm::abs(denom) < FLT_EPSILON) return false;
		glm::vec3 tvec = m_vert[0].position - intersect.ray.origin;
		intersect.ray_length = glm::dot(tvec, m_normal) / denom;
		if (intersect.ray_length < 0) return false;
		intersect.hitPt = intersect.ray.origin + 
			intersect.ray.direction * intersect.ray_length;
		glm::vec3 baryc;
		if (!calcBarycentricCoord(intersect.hitPt, baryc, m_limit)) return false;
		intersect.p_object = this;
		return true;
	}

	void Triangle::getTexCoordAndNormal(Intersection& intersect) const
	{
		glm::vec3 baryc;
		calcBarycentricCoord(intersect.hitPt, baryc, false);
		
		if (m_material->isTexCoordRequired()) {
			intersect.texCoord = m_vert[0].texCoord * baryc.x +
				m_vert[1].texCoord * baryc.y +
				m_vert[2].texCoord * baryc.z;
		}

		if (m_useVertNormals) {
			intersect.normal = glm::normalize(m_vert[0].normal * baryc.x +
				m_vert[1].normal * baryc.y +
				m_vert[2].normal * baryc.z);
			if (glm::dot(-intersect.ray.direction, m_normal) < 0) {
				intersect.normal *= -1;
			}
			if (m_vert[0].alpha > 0) {
				//some black magic with consistent normals calculation
				//algorithm from "Consistent Normal Interpolation", Reshetov et al.
				float alphaAtPt = m_vert[0].alpha * baryc.x +
					m_vert[1].alpha * baryc.y +
					m_vert[2].alpha * baryc.z;
				float q = 1.0f - (2.0f / M_PI) * alphaAtPt;
				q *= q;
				q /= 1.0f + 2.0f * (1.0f - (2.0f / M_PI) * alphaAtPt);
				float b = glm::dot(-intersect.ray.direction, intersect.normal);
				float g = 1.0f + q * (b - 1);
				float p = glm::sqrt((q * (1 + g)) / (1 + b));
				glm::vec3 refl = (g + p * b) * intersect.normal 
					+ p * intersect.ray.direction;
				intersect.normal = glm::normalize(refl - intersect.ray.direction);
			}
		}
		else {
			intersect.normal = m_normal;
			if (glm::dot(-intersect.ray.direction, m_normal) < 0) {
				intersect.normal *= -1;
			}
		}
	}

	void Triangle::setVertex(int num, const Vertex& val)
	{
		m_vert[num] = val;
	}

	const Vertex& Triangle::getVertex(int num) const
	{
		return m_vert[num];
	}

	void Triangle::setUseTextureCoords(bool val)
	{
		m_useTextureCoords = val;
	}

	bool Triangle::getUseTextureCoords() const
	{
		return m_useTextureCoords;
	}

	void Triangle::setUseVertNormals(bool val)
	{
		m_useVertNormals = val;
	}

	bool Triangle::getUseVertNormals() const
	{
		return m_useVertNormals;
	}

	const glm::vec3& Triangle::getFaceNormal() const
	{
		return m_normal;
	}

	bool Triangle::calcBarycentricCoord(const glm::vec3& pt, glm::vec3& out, bool limit) const 
	{
		glm::vec3 v0pt = pt - m_vert[0].position;
		float d20 = glm::dot(v0pt, m_v0v1);
		float d21 = glm::dot(v0pt, m_v0v2);
		out.y = (m_d11 * d20 - m_d01 * d21) * m_invdenom;
		if (limit && (out.y < 0 || out.y > 1)) return false;
		out.z = (m_d00 * d21 - m_d01 * d20) * m_invdenom;
		if (limit && (out.z < 0 || out.z + out.y > 1)) return false;
		out.x = 1.0f - out.y - out.z;
		return true;
	}

	void Triangle::commitTransformations()
	{
		m_v0v1 = m_vert[1].position - m_vert[0].position;
		m_v0v2 = m_vert[2].position - m_vert[0].position;
		glm::vec3 crossproduct = glm::cross(m_v0v1, m_v0v2);
		m_normal = glm::normalize(crossproduct);
		m_d00 = glm::dot(m_v0v1, m_v0v1);
		m_d01 = glm::dot(m_v0v1, m_v0v2);
		m_d11 = glm::dot(m_v0v2, m_v0v2);
		m_invdenom = 1.0f / (m_d00 * m_d11 - m_d01 * m_d01);
		glm::vec3 minPt;
		minPt.x = fmin(fmin(m_vert[0].position.x, m_vert[1].position.x), m_vert[2].position.x);
		minPt.y = fmin(fmin(m_vert[0].position.y, m_vert[1].position.y), m_vert[2].position.y);
		minPt.z = fmin(fmin(m_vert[0].position.z, m_vert[1].position.z), m_vert[2].position.z);
		glm::vec3 maxPt;
		maxPt.x = fmax(fmax(m_vert[0].position.x, m_vert[1].position.x), m_vert[2].position.x);
		maxPt.y = fmax(fmax(m_vert[0].position.y, m_vert[1].position.y), m_vert[2].position.y);
		maxPt.z = fmax(fmax(m_vert[0].position.z, m_vert[1].position.z), m_vert[2].position.z);
		m_aabb = AABB(minPt, maxPt);
	}
}
