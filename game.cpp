#include "precomp.h"
#include "raytracer\samplers\ImageSampler.h"
#include "raytracer\samplers\ColorSampler.h"
#include "raytracer/lights/GlobalLight.h"
#include "raytracer/lights/PointLight.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	float aspectRatio = static_cast<float>(screen->GetWidth()) / screen->GetHeight();
	m_cam = new AGR::Camera(aspectRatio, 80);
	s.push_back(new AGR::ImageSampler("earth.png"));
	s.push_back(new AGR::ColorSampler(glm::vec3(1, 1, 1)));
	m.push_back(new AGR::Material);
	m[0]->ambientIntensity = 0.1f;
	m[0]->diffuseIntensity = 0.9f;
	m[0]->specularIntensity = 0.0f;
	//m[0]->reflectionIntensity = 0.7f;
	m[0]->shininess = 3.0f;
	m[0]->texture = new AGR::ImageSampler("earth.png");
	m.push_back(new AGR::Material);
	m[1]->refractionIntensity = 1.0f;
	m[1]->reflectionIntensity = -1.0f;
	m[1]->texture = s[1];
	m[1]->ambientIntensity = 0.05f;
	r.push_back(new AGR::Sphere(*m[1], glm::vec3(-1, 0, 9)));
	r.push_back(new AGR::Sphere(*m[0], glm::vec3(1.3, 0, 8), 0.5f));
	r.push_back(new AGR::Sphere(*m[0], glm::vec3(0, -0.5, 11.3), 2));
	l.push_back(new AGR::PointLight(0.1, glm::vec3(0, 3, 8), glm::vec3(1000, 1000, 1000)));
	m_scene = new AGR::Renderer(*m_cam, glm::vec3(0, 0, 0), glm::vec2(screen->GetWidth(), screen->GetHeight()));
	m_scene->addRenderable(*r[0]);
	m_scene->addRenderable(*r[1]);
	m_scene->addRenderable(*r[2]);
	m_scene->addLight(*l[0]);
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
	delete m_scene;
	delete m_cam;
	for (auto cl : l) delete cl;
	for (auto cr : r) delete cr;
	for (auto cm : m) delete cm;
	for (auto cs : s) delete cs;
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float _DT )
{
	m_scene->render();
	memcpy(screen->GetBuffer(), m_scene->getImage(),
		m_scene->getResolution().x * m_scene->getResolution().y * sizeof(Pixel));
	glm::vec3 rot = ((AGR::Sphere *)r[2])->getRotation();
	rot.y += 5;
	((AGR::Sphere *)r[2])->setRotation(rot);
}