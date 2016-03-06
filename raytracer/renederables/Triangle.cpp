#include "Triangle.h"
#include "../util.h"

namespace AGR
{
	Triangle::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, 
		Material& m, bool invertNormals, bool useTextureCoords, bool useVertNormals):
		Renderable(m, invertNormals),
		m_useTextureCoords(useTextureCoords),
		m_useVertNormals(useVertNormals)
	{
		m_vert[0] = v1;
		m_vert[1] = v2;
		m_vert[2] = v3;
		recalcInternalInfo();
	}

	bool Triangle::intersect(const Ray& r, Intersection& out) const
	{
		float denom = glm::dot(r.directon, m_normal);
		if (glm::abs(denom) < FLT_EPSILON) return false;
		glm::vec3 tvec = m_vert[0].position - r.origin;
		out.ray_length = glm::dot(tvec, m_normal) / denom;
		if (out.ray_length < 0) return false;
		out.hitPt = r.origin + r.directon * out.ray_length;
		glm::vec3 baryc;
		if (!calcBarycentricCoord(out.hitPt, baryc)) return false;
		if (m_useVertNormals) {
			out.normal = glm::normalize(m_vert[0].normal * baryc.x +
				m_vert[1].normal * baryc.y +
				m_vert[2].normal * baryc.z);
			if (m_vert[0].alpha > 0) {
				//some black magic with consistent normals calculation
				float alphaAtPt = m_vert[0].alpha * baryc.x +
					m_vert[1].alpha * baryc.y +
					m_vert[2].alpha * baryc.z;
				float q = 1.0f - (2.0f / M_PI) * alphaAtPt;
				q *= q;
				q /= 1.0f + 2.0f * (1.0f - (2.0f / M_PI) * alphaAtPt);
				if (glm::dot(-r.directon, m_normal) < 0) {
					out.normal *= -1;
				}
				float b = glm::dot(-r.directon, out.normal);
				float g = 1.0f + q * (b - 1);
				float p = glm::sqrt((q * (1 + g)) / (1 + b));
				glm::vec3 refl = (g + p * b) * out.normal + p * r.directon;
				out.normal = glm::normalize(refl - r.directon);
			}
		} else {
			out.normal = m_normal;
		}
		out.p_object = this;
		return true;
	}

	void Triangle::getTexCoord(glm::vec3 pt, glm::vec2& out) const
	{
		glm::vec3 weights;
		if (!m_useTextureCoords || calcBarycentricCoord(pt, weights)) {
			out = glm::vec2();
		}
		out = m_vert[0].texCoord * weights.x +
			m_vert[1].texCoord * weights.y +
			m_vert[2].texCoord * weights.z;
	}

	void Triangle::setVertex(int num, const Vertex& val)
	{
		m_vert[num] = val;
		recalcInternalInfo();
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

	bool Triangle::calcBarycentricCoord(const glm::vec3& pt, glm::vec3& out) const 
	{
		glm::vec3 v0pt = pt - m_vert[0].position;
		float d20 = glm::dot(v0pt, m_v0v1);
		float d21 = glm::dot(v0pt, m_v0v2);
		out.y = (m_d11 * d20 - m_d01 * d21) * m_invdenom;
		if (out.y < 0 || out.y > 1) return false;
		out.z = (m_d00 * d21 - m_d01 * d20) * m_invdenom;
		if (out.z < 0 || out.z + out.y > 1) return false;
		out.x = 1.0f - out.y - out.z;
		return true;
	}

	void Triangle::recalcInternalInfo()
	{
		m_v0v1 = m_vert[1].position - m_vert[0].position;
		m_v0v2 = m_vert[2].position - m_vert[0].position;
		m_normal = glm::normalize(glm::cross(m_v0v1, m_v0v2));
		m_d00 = glm::dot(m_v0v1, m_v0v1);
		m_d01 = glm::dot(m_v0v1, m_v0v2);
		m_d11 = glm::dot(m_v0v2, m_v0v2);
		m_invdenom = 1.0f / (m_d00 * m_d11 - m_d01 * m_d01);
		if (m_invertNormals) {
			m_normal *= -1;
			for (int i = 0; i < 3; i++)
				m_vert[i].normal *= -1;
		}
	}
}
