#include "precomp.h"
#include "raytracer\samplers\ImageSampler.h"
#include "raytracer/lights/GlobalLight.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	float aspectRatio = static_cast<float>(screen->GetWidth()) / screen->GetHeight();
	m_cam = new AGR::Camera(aspectRatio, 80);
	m_mat = new AGR::Material;
	m_mat->ambientIntensity = 0.2f;
	m_mat->diffuseIntensity = 0.8f;
	m_mat->specularIntensity = 0.8f;
	m_mat->shininess = 3.0f;
	m_mat->texture = new AGR::ImageSampler("earth.png");
	m_sph = new AGR::Sphere(*m_mat, glm::vec3(-1, 0, 10), 1.0f, glm::vec3(0, 0, 0));
	m_sph2 = new AGR::Sphere(*m_mat, glm::vec3(1, 0, 10));
	m_sph3 = new AGR::Sphere(*m_mat, glm::vec3(0, -0.5, 11.3), 2);
	m_light = new AGR::GlobalLight(glm::vec3(0, -1, 1));
	m_scene = new AGR::Renderer(*m_cam, glm::vec3(0, 0, 0), glm::vec2(screen->GetWidth(), screen->GetHeight()));
	m_scene->addRenderable(*m_sph);
	m_scene->addRenderable(*m_sph2);
	m_scene->addRenderable(*m_sph3);
	m_scene->addLight(*m_light);
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	delete m_scene;
	delete m_sph;
	delete m_sph2;
	delete m_cam;
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float _DT )
{
	m_scene->render();
	memcpy(screen->GetBuffer(), m_scene->getImage(),
		m_scene->getResolution().x * m_scene->getResolution().y * sizeof(Pixel));
	glm::vec3 rot = m_sph3->getRotation();
	rot.y += 5;
	m_sph3->setRotation(rot);
}