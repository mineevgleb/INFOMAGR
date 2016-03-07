#pragma once
#include "../Material.h"
#include "../Intersection.h"
#include "../AABB.h"

namespace AGR {
	class Primitive {
	friend class Renderer;
	public:
		explicit Primitive(Material &m) :
			m_material(&m), m_idx(-1) {}
		virtual ~Primitive() {}
		virtual bool intersect(Intersection &intersect) const = 0;
		virtual void getTexCoordAndNormal(Intersection &intersect) const = 0;
		const AABB& getBoundingBox() const { return m_aabb; }
		const Material* getMaterial() const { return m_material; }
	protected:
		Material *m_material;
		AABB m_aabb;
	private:
		//index of an object in the Renderer's vector
		int m_idx;
	};

}