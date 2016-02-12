#define SCRWIDTH		800
#define SCRHEIGHT		512
// #define FULLSCREEN
#define GLM_FORCE_RADIANS
// #define OLDTEMPLATESTYLE

#include <inttypes.h>
extern "C" 
{ 
#include "glew.h" 
}
#include "gl.h"
#include "io.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "wglext.h"
#include "template.h"
#include "counters.h"
#include "surface.h"

using namespace std;
using namespace Tmpl8;				// to use template classes
using namespace glm;				// to use glm vector stuff

#include <vector>
#include "game.h"
#include "fcntl.h"
#include "threads.h"
#include <glm/gtc/matrix_transform.hpp>
#include "freeimage.h"