#include "precomp.h"
#include "raytracer\samplers\ImageSampler.h"
#include "raytracer\samplers\CheckboardSampler.h"
#include "raytracer\samplers\ColorSampler.h"
#include "raytracer/lights/PointLight.h"
#include "raytracer/renederables/Triangle.h"
#include "raytracer/renederables/Mesh.h"
#include "raytracer/renederables/Sphere.h"

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
AGR::Mesh *hum1;
AGR::Mesh *hum2;
AGR::Mesh *arm;
AGR::Mesh *teap;

void Game::Init()
{
	float aspectRatio = static_cast<float>(screen->GetWidth()) / screen->GetHeight();
	m_cam = new AGR::Camera(aspectRatio, 80, glm::vec3(0, 1.5, -5));
	s.push_back(new AGR::CheckboardSampler(glm::vec2(10, 5), glm::vec3(1, 1, 1), glm::vec3(0.5, 0.5, 0.5)));
	s.push_back(new AGR::ColorSampler(glm::vec3(1, 0.5, 0.5)));
	s.push_back(new AGR::ColorSampler(glm::vec3(0.5, 0.5, 0.5)));
	s.push_back(new AGR::ImageSampler("pier.jpg"));
	s.push_back(new AGR::ColorSampler(glm::vec3(0.5, 0.5, 1.0)));
	m.push_back(new AGR::Material);
	//m[0]->specularIntensity = 1.0f;
	m[0]->diffuseIntensity = 1.0f;
	//m[0]->reflectionIntensity = 0.9f;
	m[0]->glowIntensity = 0.0f;
	//m[0]->reflectionIntensity = 1.0f;
	//m[0]->reflectionColor = glm::vec3(1, 0.8, 1);
	m[0]->texture = s[2];
	m.push_back(new AGR::Material);
	m[1]->refractionIntensity = 1.0f;
	m[1]->refractionCoeficient = 1.71f;
	m[1]->innerColor = glm::vec3(0.5, 1.0, 0.5);
	m[1]->reflectionIntensity = 0.0f;
	m[1]->absorption = 1.3f;
	m.push_back(new AGR::Material);
	m[2]->texture = s[3];
	//m[2]->diffuseIntensity = 1.0f;
	m[2]->glowIntensity = 1.0f;
	m.push_back(new AGR::Material);
	m[3]->texture = s[1];
	m[3]->glowIntensity = 5.5f;
	m[3]->diffuseIntensity = 0.2f;
	m.push_back(new AGR::Material);
	m[4]->texture = s[4];
	m[4]->reflectionIntensity = 1.0;
	r.push_back(new AGR::Sphere(*m[2], glm::vec3(), 400, glm::vec3(0, 180 ,0)));
	r.push_back(new AGR::Sphere(*m[1], glm::vec3(2, 0.5, 2), 0.5, glm::vec3(0, 180, 0)));
	r.push_back(new AGR::Sphere(*m[4], glm::vec3(-2, 0.5, 2), 0.5, glm::vec3(0, 180, 0)));
	r.push_back(new AGR::Triangle(
		AGR::Vertex(glm::vec3(0, 0, 40), glm::vec2(0, 0)),
		AGR::Vertex(glm::vec3(10, 0, 50), glm::vec2(0, 1)),
		AGR::Vertex(glm::vec3(-10, 0, 50), glm::vec2(1, 0)), 
		*m[0], true, false, false));
	hum1 = new AGR::Mesh(*m[0]);
	hum1->load("cube.obj", AGR::CONSISTENT);
	hum1->setRotation(glm::vec3(0, 90, 0));
	hum1->setScale(glm::vec3(9.0f));
	hum1->setPosition(glm::vec3(3.0f, -9.0f, 16.0f));
	hum1->commitTransformations();
	hum2 = new AGR::Mesh(*m[3]);
	hum2->load("cube.obj", AGR::CONSISTENT);
	//hum2->setRotation(glm::vec3(0, -90, 0));
	hum2->setScale(glm::vec3(0.5f));
	hum2->setPosition(glm::vec3(2.1, 0.4, 2.0));
	hum2->commitTransformations();
	arm = new AGR::Mesh(*m[0]);
	arm->load("bunny.obj", AGR::CONSISTENT);
	arm->setPosition(glm::vec3(0, 0, 0.5));
	arm->setScale(glm::vec3(0.5));
	arm->setRotation(glm::vec3(0, 0, 0));
	arm->commitTransformations();

	l.push_back(new AGR::PointLight(0.1, glm::vec3(0, 3, -3), glm::vec3(1000, 1000, 1000)));
	l.push_back(new AGR::PointLight(0.5, glm::vec3(0, 1, 40), glm::vec3(1000, 1000, 1000)));
	m_scene = new AGR::Pathtracer(*m_cam, glm::vec3(0, 0, 0), glm::vec2(screen->GetWidth(), screen->GetHeight()));
	m_scene->addRenderable(*r[0]);
	m_scene->addRenderable(*r[1]);m_scene->addRenderable(*r[2]);
	m_scene->addRenderable(*hum1);
	m_scene->addRenderable(*hum2);
	m_scene->addRenderable(*arm);
	//m_scene->addLight(*l[0]);
	//m_scene->addLight(*l[1]);
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
	DWORD before = GetTickCount();
	//static int i = 0;
	//if (!i++) 
	m_scene->render();
	DWORD after = GetTickCount();
	memcpy(screen->GetBuffer(), m_scene->getImage(),
		m_scene->getResolution().x * m_scene->getResolution().y * sizeof(Pixel));
	screen->Print(std::to_string(after - before).c_str(), 10, 10, 0xFF0000);
	static float rot = 0;
	rot += 10;
	//((AGR::Sphere *)r[2])->setRotation(rot);
	//arm->setRotation(glm::vec3(0, rot, 0));
	//arm->commitTransformations();
}

void Game::MouseDown(int _Button)
{
	glm::vec3 col;
	int x, y;
	SDL_GetMouseState(&x, &y);
	m_mousePos.x = x;
	m_mousePos.y = y;
	m_scene->testRay(x, y);
}

void Game::MouseMove(int _X, int _Y)
{
	m_mousePos = glm::vec2(_X, _Y);
}
