#include "precomp.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	float aspectRatio = static_cast<float>(screen->GetWidth()) / screen->GetHeight();
	m_cam = new AGR::Camera(AGR::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(aspectRatio, 1, 0.5)));
	m_sph = new AGR::Sphere(AGR::Transform(glm::vec3(-1, 0, 10)));
	m_sph2 = new AGR::Sphere(AGR::Transform(glm::vec3(1, 0, 10)));
	m_sph3 = new AGR::Sphere(AGR::Transform(glm::vec3(0, -0.5, 11.3), glm::vec3(), glm::vec3(2)));
	m_scene = new AGR::Scene(*m_cam, 0x0000FF00, glm::vec2(screen->GetWidth(), screen->GetHeight()));
	m_scene->addRenderable(*m_sph);
	m_scene->addRenderable(*m_sph2);
	m_scene->addRenderable(*m_sph3);
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
}