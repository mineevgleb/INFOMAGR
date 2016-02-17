#include "Material.h"

namespace AGR
{
	float Material::getAmbientIntensity() const
	{
		return m_ambientIntensity;
	}

	void Material::setAmbientIntensity(float ambientIntensity)
	{
		m_ambientIntensity = ambientIntensity;
	}

	Sampler* const& Material::getAmbientColor() const
	{
		return m_ambientColor;
	}

	void Material::setAmbientColor(Sampler* ambientColor)
	{
		m_ambientColor = ambientColor;
		updateRequireTex();
	}

	float Material::getDiffuseIntensity() const
	{
		return m_diffuseIntensity;
	}

	void Material::setDiffuseIntensity(float diffuseIntensity)
	{
		m_diffuseIntensity = diffuseIntensity;
	}

	Sampler* const& Material::getDiffuseColor() const
	{
		return m_diffuseColor;
	}

	void Material::setDiffuseColor(Sampler* diffuseColor)
	{
		m_diffuseColor = diffuseColor;
		updateRequireTex();
	}

	bool Material::isTexCoordRequired() const
	{
		return m_isRequiresTexCoord;
	}

	void Material::updateRequireTex()
	{
		m_isRequiresTexCoord =
			((m_ambientColor != nullptr && m_ambientColor->isRequiresTexture()) ||
				(m_diffuseColor != nullptr && m_diffuseColor->isRequiresTexture()));
	}
}
