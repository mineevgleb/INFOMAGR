#pragma once
#include "raytracer\Renderer.h"
#include "raytracer\renederables\Sphere.h"

namespace Tmpl8 {

class Surface;
class Game
{
public:
	void SetTarget( Surface* _Surface ) { screen = _Surface; }
	void Init();
	void Shutdown();
	void Tick( float _DT );
	void MouseUp( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown(int _Button);
	void MouseMove(int _X, int _Y);
	void KeyUp( int _Key ) { /* implement if you want to handle keys */ }
	void KeyDown( int _Key ) { /* implement if you want to handle keys */ }
private:
	Surface* screen;
	std::vector<AGR::Primitive *> r;
	std::vector<AGR::Material *> m;
	std::vector<AGR::Sampler *> s;
	std::vector<AGR::Light *> l;
	AGR::Camera *m_cam;
	AGR::Renderer *m_scene;
	glm::vec2 m_mousePos;
};

}; // namespace Tmpl8