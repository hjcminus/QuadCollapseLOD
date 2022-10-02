/*
shared data type & function
*/

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)

// memory
#ifdef _DEBUG
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
# define	NEW__				new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
# define	NEW__				new
#endif

# define	PATH_SEPERATOR		"\\"

#endif

#if defined(__linux__)
# define	NEW__				new
# define	PATH_SEPERATOR		"/"
#endif

#if defined(_MSC_VER)
# define	strcpy_				strcpy_s
# define	strcat_				strcat_s
# define	sprintf_			sprintf_s
# define	vsprintf_			vsprintf_s
# define	fprintf_			fprintf_s
#endif

#if defined(__GNUC__)
# define	strcpy_				strcpy
# define	strcat_				strcat
# define	sprintf_			sprintf
# define	vsprintf_			vsprintf
# define	fprintf_			fprintf
#endif


// OpenGL
#include <GL/glew.h>

// GL math
#define GLM_SWIZZLE	// enable swizzle
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

/*
================================================================================
macros
================================================================================
*/
#define		VIEW_WIDTH			800
#define		VIEW_HEIGHT			600
#define		Z_NEAR				1.0f
#define		Z_FAR				4096.0f
#define		FOVY				70.0f
#define		PI					3.14159265358979323846f

#ifndef		MAX_PATH 
# define	MAX_PATH			260
#endif

#define		START_FORWARD		vec3(0.0f, 1.0f, 0.0f)
#define		START_RIGHT			vec3(1.0f, 0.0f, 0.0f)
#define		START_UP			vec3(0.0f, 0.0f, 1.0f)

#define		MAX_PRINT_TEXT_LEN	1024

#define		arraysize(A)		(sizeof(A) / sizeof(A[0]))

enum draw_flag_s {
	DF_SKYBOX = 1,
	DF_SOLID_TERRAIN = 2,
	DF_WIREFRAME_TERRAIN = 4
};

/*
================================================================================
types
================================================================================
*/
typedef		uint8_t				byte;
typedef		uint16_t			word;
typedef		uint32_t			dword;

// demo configuration
struct config_s {
	int							mViewWidth;
	int							mViewHeight;
	float						mZNear;
	float						mZFar;
	float						mFovy;
	vec3						mCameraPos;
	float						mCameraYaw;
	float						mCameraPitch;
	const char *				mResDir;
	const char *				mSkyboxDir;
	const char *				mTerrainHeight;
	float						mTerrainZScale;
	const char *				mTerrainBase;
	const char *				mTerrainDetail;
	vec3						mBackgroundColor;
	vec3						mWireframeColor;
	vec3						mFogColor;
	float						mFogDensity;
	vec3						mFontColor;

	config_s() {
		mViewWidth = VIEW_WIDTH;
		mViewHeight = VIEW_HEIGHT;
		mZNear = Z_NEAR;
		mZFar = Z_FAR;
		mFovy = FOVY;
		mCameraPos = vec3(0.0f);
		mCameraYaw = 0.0f;
		mCameraPitch = 0.0f;
		mResDir = nullptr;
		mSkyboxDir = nullptr;
		mTerrainHeight = nullptr;
		mTerrainZScale = 1.0f;
		mTerrainBase = nullptr;
		mTerrainDetail = nullptr;
		mBackgroundColor = vec3(0.0f);
		mWireframeColor = vec3(0.0f);
		mFogColor = vec3(0.0f);
		mFogDensity = 0.005f;
		mFogColor = vec3(0.0f);
	}
};

struct vertex_s {
	vec3						mPos;
	vec2						mTexCoord;
};

struct camera_s {
	vec3						mPos;
	vec3						mTarget;
	vec3						mUp;
	float						mZNear;
	float						mZFar;
	float						mFovy;

	camera_s() {
		mPos = vec3(0.0f);
		mTarget = START_FORWARD;
		mUp = START_UP;
		mZNear = Z_NEAR;
		mZFar = Z_FAR;
		mFovy = FOVY;
	}
};

struct frustum_plane_s {
	vec4						mLeftPlane;
	vec4						mRightPlane;

	void						Setup(int viewport_width, int viewport_height, const camera_s &cam);
	bool						CullHorizontalCircle(const vec3 &center, float radius) const;
};

enum movement_key_t : int {
	MK_NONE,
	MK_FORWARD,
	MK_BACKWARD,
	MK_LEFT,
	MK_RIGHT,
	MK_STRAIGHT_UP,
	MK_STRAIGHT_DOWN,
};

struct input_s {
	movement_key_t				mMovementKey;
	int							mMouseDeltaX;
	int							mMouseDeltaY;
};

// RGBA image
struct image32_s {
	int							mWidth;
	int							mHeight;
	byte *						mData;

	image32_s() :
		mWidth(0),
		mHeight(0),
		mData(nullptr)
	{
	}

	~image32_s() {
		if (mData) {
			free(mData);
		}
	}

	void						AllocDataSpace(int pixels);
};

struct height_field_s {
	int							mWidth;
	int							mHeight;
	byte *						mData;

	height_field_s() :
		mWidth(0),
		mHeight(0),
		mData(nullptr)
	{
	}

	~height_field_s() {
		if (mData) {
			free(mData);
		}
	}

	void						AllocDataSpace(int size);
};

struct triangle_mesh_s {
	const vec3 *				mVertices;
	int							mNumTriangles;

	triangle_mesh_s() {
		mVertices = nullptr;
		mNumTriangles = 0;
	}
};

template<class T, int INIT_CAPACITY>
class ItemArray {
public:

	ItemArray();
	~ItemArray();

	void						Reset();
	void						Add(const T &item);

	int							GetCount() const;
	T *							GetItems();
	const T *					GetItems() const;

private:

	T *							mBuffer;
	int							mCapacity;
	int							mSize;
};

template<class T, int INIT_CAPACITY>
ItemArray<T, INIT_CAPACITY>::ItemArray() {
	mBuffer = (T*)malloc(sizeof(T) * INIT_CAPACITY);
	mCapacity = INIT_CAPACITY;
	mSize = 0;
}

template<class T, int INIT_CAPACITY>
ItemArray<T, INIT_CAPACITY>::~ItemArray() {
	if (mBuffer) {
		free(mBuffer);
	}
}

template<class T, int INIT_CAPACITY>
void ItemArray<T, INIT_CAPACITY>::Reset() {
	mSize = 0;
}

template<class T, int INIT_CAPACITY>
void ItemArray<T, INIT_CAPACITY>::Add(const T &item) {
	if (mSize == mCapacity) {
		mCapacity += (mCapacity >> 1);
		mBuffer = (T*)realloc(mBuffer, sizeof(T) * mCapacity);
	}

	mBuffer[mSize++] = item;
}

template<class T, int INIT_CAPACITY>
int ItemArray<T, INIT_CAPACITY>::GetCount() const {
	return mSize;
}

template<class T, int INIT_CAPACITY>
T * ItemArray<T, INIT_CAPACITY>::GetItems() {
	return mBuffer;
}

template<class T, int INIT_CAPACITY>
const T * ItemArray<T, INIT_CAPACITY>::GetItems() const {
	return mBuffer;
}

/*
================================================================================
helper
================================================================================
*/
uint32_t	ToggleFlags(uint32_t flags, uint32_t bit);

/*
================================================================================
math
================================================================================
*/
float	ClampYaw(float yaw);
float	ClampPitch(float pitch);
bool	IsPowerOf2(int n);

/*
================================================================================
string
================================================================================
*/
void	Str_EraseDoubleDots(char *s);
void	Str_ExtractDirSelf(char *s);
void	Str_ExtractExeDir(const char *exe, char *dir, int dir_size);

/*
================================================================================
hints
================================================================================
*/

#if defined(_MSC_VER)
# define	SYS_ERROR(fmt, ...)		Sys_Error(__FILE__, __LINE__, fmt, __VA_ARGS__)
#endif

#if defined(__GNUC__)
# define	SYS_ERROR(fmt, ...)		Sys_Error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

void	Sys_Error(const char *file, int line, const char *fmt, ...);

// override default error output proc
typedef void(*sys_error_output_proc_t)(const char *text);
void	Sys_SetErrorOutputProc(sys_error_output_proc_t proc);

/*
================================================================================
file
================================================================================
*/
FILE *	File_Open(const char * filename, const char * mod);
int		File_GetSize(const char * filename);
int		File_LoadText(const char * filename, char *buffer, int buffer_size);
int		File_LoadBinary(const char * filename, char *buffer, int buffer_size);
bool	File_LoadBMP(const char * filename, image32_s &image);
bool	File_LoadRawHeightFieldFrom8BitBMP(const char * filename, height_field_s &hf);

/*
================================================================================
timer
================================================================================
*/
void	Sys_InitTimer();
double	Sys_GetRelativeTime();	// seconds

/*
================================================================================
GL Helper
================================================================================
*/
bool	GL_Init();

enum gl_shader_t {
	VERTEX_SHADER,
	TESSELLATION_CONTROL_SHADER,
	TESSELLATION_EVALUATION_SHADER,
	GEOMETRY_SHADER,
	FRAGMENT_SHADER,

	SUPPORT_SHADER_COUNT
};

struct gl_program_s {
	GLuint						mProgram;
	GLuint						mShaders[SUPPORT_SHADER_COUNT];

	gl_program_s():mProgram(0) {
		memset(mShaders, 0, sizeof(mShaders));
	}
};

struct gl_shader_desc_s {
	const char *				mFiles[SUPPORT_SHADER_COUNT];

	gl_shader_desc_s() {
		memset(mFiles, 0, sizeof(mFiles));
	}
};

bool	GL_CreateProgram(const gl_shader_desc_s &desc, gl_program_s &program);
bool	GL_CreateProgram(const char *res_dir, const char * vs, const char *fs, gl_program_s &prog);
void	GL_DeleteProgram(gl_program_s &program);

// support BMP file for now
struct gl_texture2d_desc_s {
	const char *				mFileName;
	bool						mMipMap;
	bool						mRepeat;
};

GLuint		GL_CreateTexture2D(const gl_texture2d_desc_s &desc);
GLuint		GL_CreateTexture2D(const char *res_dir, const char * filename, bool mipmap, bool repeat);

GLuint		GL_CreateUniformBuffer(size_t size);
int			GL_CheckError();
