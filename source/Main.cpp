/*
main entrance, base on GLUT framework
*/

#include "Precompiled.h"

// GLUT Framework
#include <GL/freeglut.h>

#if defined(_WIN32)
# include <GL/wglew.h>
#endif

#if defined(__linux__)
# include <GL/glew.h>
#endif

#include <chrono>

struct window_state_s {
	bool						mFullsceen;
	bool						mCursorVisible;
	int							mWindowedViewWidth;
	int							mWindowedViewHeight;
	int							mViewportWidth;
	int							mViewportHeight;
};

struct mouse_state_s {
	bool						mLeftButtonDown;
	int							mPriorX;
	int							mPriorY;
};

struct frame_s {
	double						mPriorTime;
};

static window_state_s			gWindowState;
static mouse_state_s			gMouseState;
static frame_s					gFrame;
static DemoApp					gDemoApp;
static input_s					gInput;
static uint32_t					gDrawFlags;
static float					gMoveSpeed;

// GLUT callback
static void OnReshape(int width, int height) {
	// adjust width and height
	gWindowState.mViewportWidth = max(1, width);
	gWindowState.mViewportHeight = max(1, height);

	if (!gWindowState.mFullsceen) {
		gWindowState.mWindowedViewWidth = gWindowState.mViewportWidth;
		gWindowState.mWindowedViewHeight = gWindowState.mViewportHeight;
	}

	gDemoApp.ResizeViewport(gWindowState.mViewportWidth, gWindowState.mViewportHeight);
}

static void OnMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (GLUT_DOWN == state) {
			gMouseState.mLeftButtonDown = true;
			gMouseState.mPriorX = x;
			gMouseState.mPriorY = y;
		}
		else if (GLUT_UP == state) {
			gMouseState.mLeftButtonDown = false;

			if (gWindowState.mCursorVisible) {
				gInput.mMouseDeltaX = 0;
				gInput.mMouseDeltaY = 0;
			}
		}
	}
}

static void OnMouseMove(int x, int y) {
	if (gWindowState.mCursorVisible) {
		if (gMouseState.mLeftButtonDown) {
			gInput.mMouseDeltaX = x - gMouseState.mPriorX;
			gInput.mMouseDeltaY = y - gMouseState.mPriorY;

			gMouseState.mPriorX = x;
			gMouseState.mPriorY = y;
		}
	}
	else {
		static bool skip_once = false;

		if (skip_once) {
			skip_once = false;
			return;
		}

		int center_x = gWindowState.mViewportWidth / 2;
		int center_y = gWindowState.mViewportHeight / 2;

		gInput.mMouseDeltaX += x - center_x;
		gInput.mMouseDeltaY += y - center_y;

		//printf("delta x: %d, delta y: %d\n", gInput.mMouseDeltaX, gInput.mMouseDeltaY);

		gMouseState.mPriorX = x;
		gMouseState.mPriorY = y;

		glutWarpPointer(center_x, center_y); // move cursor to view center
		skip_once = true; // do not process event cause by glutWarpPointer
	}
}

static void OnSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_F2) {
		gDrawFlags = ToggleFlags(gDrawFlags, DF_SKYBOX);
		return;
	}

	if (key == GLUT_KEY_F3) {
		gDrawFlags = ToggleFlags(gDrawFlags, DF_SOLID_TERRAIN);
		return;
	}

	if (key == GLUT_KEY_F4) {
		gDrawFlags = ToggleFlags(gDrawFlags, DF_WIREFRAME_TERRAIN);
		return;
	}

	if (key == GLUT_KEY_PAGE_UP) {
		gMoveSpeed += 1.0f;
		if (gMoveSpeed > 1024.0f) {
			gMoveSpeed = 1024.0f;
		}
		gDemoApp.SetMoveSpeed(gMoveSpeed);
		return;
	}

	if (key == GLUT_KEY_PAGE_DOWN) {
		gMoveSpeed -= 1.0f;
		if (gMoveSpeed < 0.0f) {
			gMoveSpeed = 0.0f;
		}
		gDemoApp.SetMoveSpeed(gMoveSpeed);
		return;
	}
}

struct movement_key_s {
	byte						mLowerCase;
	byte						mUpperCase;
	movement_key_t				mKeyFlag;
};

static const movement_key_s MOVEMENT_KEYS[] = {
	'w','W',MK_FORWARD,
	's','S',MK_BACKWARD,
	'a','A',MK_LEFT,
	'd','D',MK_RIGHT,
	'q','Q',MK_STRAIGHT_UP,
	'z','Z',MK_STRAIGHT_DOWN
};

static void OnKeyboard(unsigned char key, int x, int y) {
	if ((key == 13) && (glutGetModifiers() & GLUT_ACTIVE_ALT)) { // ALT + ENTER toggle full screen mode
		gWindowState.mFullsceen = !gWindowState.mFullsceen;

		if (gWindowState.mFullsceen) {
			glutFullScreen();
		}
		else {
			glutReshapeWindow(gWindowState.mWindowedViewWidth, gWindowState.mWindowedViewHeight);
		}
		return;
	}

	if (key == 27) { // escape
		glutExit();
		return;
	}

	if (key == ' ') { // space key to show/hide cursor
		gWindowState.mCursorVisible = !gWindowState.mCursorVisible;
		glutSetCursor(gWindowState.mCursorVisible ? GLUT_CURSOR_LEFT_ARROW : GLUT_CURSOR_NONE);

		if (!gWindowState.mCursorVisible) {
			glutWarpPointer(gWindowState.mViewportWidth / 2, gWindowState.mViewportHeight / 2);
			gInput.mMouseDeltaX = gInput.mMouseDeltaY = 0;
		}
		
		return;
	}

	for (int i = 0; i < arraysize(MOVEMENT_KEYS); ++i) {
		const movement_key_s & mk = MOVEMENT_KEYS[i];
		if (key == mk.mLowerCase || key == mk.mUpperCase) {
			gInput.mMovementKey = mk.mKeyFlag;
			return;
		}
	}
}

static void OnKeyboardUp(unsigned char key, int x, int y) {
	for (int i = 0; i < arraysize(MOVEMENT_KEYS); ++i) {
		const movement_key_s & mk = MOVEMENT_KEYS[i];
		if (key == mk.mLowerCase || key == mk.mUpperCase) {
			gInput.mMovementKey = MK_NONE;
			return;
		}
	}
}

static void OnDisplay() {
	gDemoApp.UpdateScreen(gDrawFlags);

#if defined(_WIN32)
	// v-sync
	if (wglSwapIntervalEXT) {
		wglSwapIntervalEXT(1);
	}
#endif

	glutSwapBuffers();
}

static const int	FPS = 60;
static const double FRAME_TIME = 1.0 / FPS;

static void OnIdle() {
	// run frame
	double t = Sys_GetRelativeTime();
	double delta = t - gFrame.mPriorTime;

	if (delta >= FRAME_TIME) {
		gDemoApp.ProcessInput((float)delta, gInput);

		gInput.mMouseDeltaX = 0;
		gInput.mMouseDeltaY = 0;

		glutPostRedisplay();
		gFrame.mPriorTime = t;
	}
}

static void OnClose() {
	gDemoApp.Shutdown();
}

static void ErrorOutput(const char *text) {
#if defined(_WIN32)
	MessageBoxA(NULL, text, "Error", MB_OK);
#endif

#if defined(__linux__)
	printf("%s\n", text);
#endif
}

int main(int argc, char **argv) {

#if defined(_WIN32)

# ifdef _DEBUG // detect memory leak
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
# endif

#endif

	Sys_SetErrorOutputProc(ErrorOutput);
	Sys_InitTimer();

	// get res directory path
	char res_dir[MAX_PATH];
	Str_ExtractExeDir(argv[0], res_dir, MAX_PATH);
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	Str_ExtractDirSelf(res_dir);
	strcat_(res_dir, PATH_SEPERATOR);
	strcat_(res_dir, "res");

	Config config_file;
	config_file.Load(res_dir);

	config_s cfg;

	cfg.mViewWidth = config_file.GetAsInteger("ViewWidth", 800);
	cfg.mViewHeight = config_file.GetAsInteger("ViewHeight", 600);
	cfg.mZNear = config_file.GetAsFloat("ZNear", Z_NEAR);
	cfg.mZFar = config_file.GetAsFloat("ZFar", Z_FAR);
	cfg.mFovy = config_file.GetAsFloat("Fovy", FOVY);
	cfg.mCameraPos = config_file.GetAsVec3("CameraPos", vec3(0.0f));
	cfg.mCameraYaw = config_file.GetAsFloat("CameraYaw", 0.0f);
	cfg.mCameraPitch = config_file.GetAsFloat("CameraPitch", 0.0f);
	cfg.mResDir = res_dir;
	cfg.mSkyboxDir = config_file.GetAsString("SkyboxDir", "skybox");
	cfg.mTerrainHeight = config_file.GetAsString("TerrainHeight", "terrain" PATH_SEPERATOR "gcanyon_height_2k2k.bmp");
	cfg.mTerrainZScale = config_file.GetAsFloat("TerrainZScale", 1.0f);
	cfg.mTerrainBase = config_file.GetAsString("TerrainBase", "terrain" PATH_SEPERATOR "gcanyon_color_2k2k.bmp");
	cfg.mTerrainDetail = config_file.GetAsString("TerrainDetail", "terrain" PATH_SEPERATOR "detail.bmp");
	cfg.mBackgroundColor = config_file.GetAsVec3("BackgroundColor", vec3(0.0f));
	cfg.mWireframeColor = config_file.GetAsVec3("WireframeColor", vec3(0.0f));
	cfg.mFogColor = config_file.GetAsVec3("FogColor", vec3(0.0f));
	cfg.mFogDensity = config_file.GetAsFloat("FogDensity", 0.005f);
	cfg.mFontColor = config_file.GetAsVec3("FontColor", vec3(0.0f));
	gMoveSpeed = config_file.GetAsFloat("MoveSpeed", 10.0f);

	int draw_skybox = config_file.GetAsInteger("DrawSkyBox", 0);
	int draw_solid_terrain = config_file.GetAsInteger("DrawSolidTerrain", 0);
	int draw_wireframe_terrain = config_file.GetAsInteger("DrawWireframeTerrain", 0);

	gDrawFlags = 0;
	if (draw_skybox) gDrawFlags |= DF_SKYBOX;
	if (draw_solid_terrain) gDrawFlags |= DF_SOLID_TERRAIN;
	if (draw_wireframe_terrain) gDrawFlags |= DF_WIREFRAME_TERRAIN;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	int screen_cx = glutGet(GLUT_SCREEN_WIDTH);
	int screen_cy = glutGet(GLUT_SCREEN_HEIGHT);
	int pos_x = (screen_cx - cfg.mViewWidth) >> 1;
	int pos_y = (screen_cy - cfg.mViewHeight) >> 1;
	glutInitWindowPosition(pos_x, pos_y);
	glutInitWindowSize(cfg.mViewWidth, cfg.mViewHeight);
	glutCreateWindow("QuadCollapseLOD");

	if (!GL_Init()) {
		return 1;
	}
	
	if (!gDemoApp.Init(cfg)) {
		gDemoApp.Shutdown();
		return 1;
	}

	gDemoApp.SetMoveSpeed(gMoveSpeed);

	gWindowState.mCursorVisible = true;

	glutReshapeFunc(OnReshape);
	glutMouseFunc(OnMouse);
	glutMotionFunc(OnMouseMove);
	glutPassiveMotionFunc(OnMouseMove); // trace cursor movement even cursor outside the viewport
	glutSpecialFunc(OnSpecial);
	glutKeyboardFunc(OnKeyboard);
	glutKeyboardUpFunc(OnKeyboardUp);
	glutDisplayFunc(OnDisplay);
	glutIdleFunc(OnIdle);
	glutCloseFunc(OnClose);

	// print hint message
	const char * HINT =
		"---------- keyboard operation ----------\n"
		"\"F2\" to toggle draw skybox\n"
		"\"F3\" to toggle draw solid terrain\n"
		"\"F4\" to toggle draw wireframe terrain\n"
		"\"ALT + ENTER\" to toggle fullscreen mode\n"
		"\"space\" to hide/show cursor\n"
		"\"W,S,A,D,Q,Z\" to move around\n"
		"\"Page Up\" to increase move speed\n"
		"\"Page Down\" to decrease move speed\n"
		"\"ESC\" can quit program\n";

	printf("%s", HINT);

	gFrame.mPriorTime = Sys_GetRelativeTime();

	// enter main loop
	glutMainLoop();

	// never reach here
	return 0;
}
