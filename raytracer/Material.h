#pragma once
#include "samplers\Sampler.h"

namespace AGR {
	class Material {
	public:
		float getAmbientIntensity() const;
		void setAmbientIntensity(float ambientIntensity);

		Sampler* const& getAmbientColor() const;
		void setAmbientColor(Sampler* ambientColor);

		float getDiffuseIntensity() const;
		void setDiffuseIntensity(float diffuseIntensity);

		Sampler* const& getDiffuseColor() const;
		void setDiffuseColor(Sampler* diffuseColor);

		bool isTexCoordRequired() const;
	private:
		void updateRequireTex();
		bool m_isRequiresTexCoord = false;
		float m_ambientIntensity = 0.0f;
		Sampler* m_ambientColor = nullptr;
		float m_diffuseIntensity = 0.0f;
		Sampler* m_diffuseColor = nullptr;
	};
}
