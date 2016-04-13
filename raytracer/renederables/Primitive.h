#pragma once
#include "../Material.h"
#include "../Intersection.h"
#include "../AABB.h"

namespace AGR {
	class Primitive {
	friend class Raytracer;
	friend class BVH;
	public:
		explicit Primitive(Material &m) :
			m_material(&m) {}
		virtual ~Primitive() {}
		virtual float intersect(const Ray &r) const = 0;
		virtual void getTexCoordAndNormal(const Ray& r, float dist, 
			glm::vec2& texCoord, glm::vec3& normal) const = 0;
		const AABB& getBoundingBox() const { return m_aabb; }
		const Material* getMaterial() const { return m_material; }
		virtual float getArea() = 0;
	protected:
		Material *m_material;
		AABB m_aabb;
	private:
		//used for BVH constructions
		::uint64_t m_mortonCode;
	};

}