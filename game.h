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
	void MouseDown( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int _X, int _Y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int _Key ) { /* implement if you want to handle keys */ }
	void KeyDown( int _Key ) { /* implement if you want to handle keys */ }
private:
	Surface* screen;
	AGR::Camera *m_cam;
	AGR::Sphere *m_sph;
	AGR::Sphere *m_sph2;
	AGR::Sphere *m_sph3;
	AGR::Renderer *m_scene;
	AGR::Material *m_mat;
};

}; // namespace Tmpl8