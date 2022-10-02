/*
shared data type & function
*/

#include "Precompiled.h"
#include <malloc.h>
#include <stdarg.h>
#include <chrono>

void frustum_plane_s::Setup(int viewport_width, int viewport_height, const camera_s &cam) {

	float tan1 = tanf(cam.mFovy * 0.5f * PI / 180.0f);
	float d = (float)viewport_height * 0.5f / tan1;
	float tan2 = (float)viewport_width * 0.5f / d;
	float fovx = ((atanf(tan2) * 180.0f) / PI) * 2.0f;

	mat4 m;
	vec3 forward = normalize(cam.mTarget - cam.mPos);

	float deg = -(90.0f - fovx * 0.5f);
	m = rotate(mat4(1.0f), deg * PI / 180.0f, cam.mUp);
	mLeftPlane = m * vec4(forward, 0.0f);
	mLeftPlane.w = -dot(cam.mPos, vec3(mLeftPlane));

	deg = (90.0f - fovx * 0.5f);
	m = rotate(mat4(1.0f), deg * PI / 180.0f, cam.mUp);
	mRightPlane = m * vec4(forward, 0.0f);
	mRightPlane.w = -dot(cam.mPos, vec3(mRightPlane));
}

bool frustum_plane_s::CullHorizontalCircle(const vec3 &center, float radius) const {
	float dist = dot(vec3(mLeftPlane), center) + mLeftPlane.w;
	if (dist < -radius) {
		return true;
	}

	dist = dot(vec3(mRightPlane), center) + mRightPlane.w;
	if (dist < -radius) {
		return true;
	}

	return false;
}

/*
================================================================================
helper
================================================================================
*/
uint32_t ToggleFlags(uint32_t flags, uint32_t bit) {
	if (flags & bit) {
		return flags & (~bit);
	}
	else {
		return flags | bit;
	}
}

/*
================================================================================
math
================================================================================
*/
float ClampYaw(float yaw) {
	while (yaw < 0.0f) {
		yaw += 360.0f;
	}

	while (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	return yaw;
}

float ClampPitch(float pitch) {
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}

	return pitch;
}

bool IsPowerOf2(int n) {
	return n && !(n & (n - 1));
}

/*
================================================================================
string
================================================================================
*/
static char * FindDoubleDots(char *s) {
	char * pc = strstr(s, "..\\");
	if (!pc) {
		pc = strstr(s, "../");
	}
	return pc;
}

static char * RFindSlash(char *s) {
	char * pc = strrchr(s, '\\');
	if (!pc) {
		pc = strrchr(s, '/');
	}
	return pc;
}

void Str_EraseDoubleDots(char *s) {
	char sl[2][MAX_PATH];

	strcpy_(sl[0], s);
	strcpy_(sl[1], s);

	int srcidx = 0;
	int dstidx = 1 - srcidx;

	char * pc = FindDoubleDots(sl[srcidx]);
	while (pc) {
		*pc = 0;

		char * pc1 = RFindSlash(sl[srcidx]);
		if (pc1) {
			*pc1 = 0;

			char * pc2 = RFindSlash(sl[srcidx]);
			if (pc2) {
				pc2[1] = 0;
				strcpy_(sl[dstidx], sl[srcidx]);
				strcat_(sl[dstidx], pc + 3);

				srcidx = 1 - srcidx;
				dstidx = 1 - srcidx;

				pc = FindDoubleDots(sl[srcidx]);
			}
			else {
				break;
			}

		}
		else {
			break;
		}
	}

#if defined(_MSC_VER)
	strcpy_s(s, MAX_PATH, sl[srcidx]);
#endif

#if defined(__GNUC__)
	strcpy(s, sl[srcidx]);
#endif
}

void Str_ExtractDirSelf(char *s) {
	char * pc = strrchr(s, '\\');
	if (!pc) {
		pc = strrchr(s, '/');
	}

	if (pc) {
		*pc = 0;
	}
}

void Str_ExtractExeDir(const char *exe, char *dir, int dir_size) {
#if defined(_MSC_VER)
	strcpy_s(dir, dir_size, exe);
#endif

#if defined(__GNUC__)
	strcpy(dir, exe);
#endif

	Str_EraseDoubleDots(dir);
	Str_ExtractDirSelf(dir);
}

/*
================================================================================
hints
================================================================================
*/
static sys_error_output_proc_t gErrorOutput;

void Sys_Error(const char *file, int line, const char *fmt, ...) {
	char buffer[4096];

	sprintf_(buffer, "file: %s\nline: %d\n", file, line);
	int len = (int)strlen(buffer);

	va_list argptr;
	va_start(argptr, fmt);
#if defined(_MSC_VER)
	vsprintf_s(buffer + len, 4096 - len, fmt, argptr);
#endif

#if defined(__GNUC__)
	vsprintf(buffer + len, fmt, argptr);
#endif
	va_end(argptr);

	if (gErrorOutput) {
		gErrorOutput(buffer);
	}
	else {
#if defined(_MSC_VER)
		fprintf_s(stderr, "%s", buffer);
#endif

#if defined(__GNUC__)
	fprintf(stderr, "%s", buffer);
#endif
	}
}

void Sys_SetErrorOutputProc(sys_error_output_proc_t proc) {
	gErrorOutput = proc;
}

/*
================================================================================
file
================================================================================
*/
int FileSize(FILE * f) {
	int pos = ftell(f);
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, pos, SEEK_SET);
	
	return size;
}

FILE * File_Open(const char * filename, const char * mod) {
#if defined(_MSC_VER)
	FILE * f = nullptr;
	if (fopen_s(&f, filename, mod) == 0) {
		return f;
	}
	else {
		return nullptr;
	}
#endif

#if defined(__GNUC__)
	return fopen(filename, mod);
#endif
}

int	File_GetSize(const char * filename) {
	int size = 0;

	FILE * f = File_Open(filename, "rb");
	if (f) {
		size = FileSize(f);
		fclose(f);
	}

	return size;
}

int	File_LoadText(const char * filename, char *buffer, int buffer_size) {
	int read = 0;

	FILE * f = File_Open(filename, "rb");
	if (f) {
		int size = FileSize(f);
		if ((size + 1) <= buffer_size) {
			read = (int)fread(buffer, 1, (size_t)size, f);
			buffer[read] = 0;
		}
		fclose(f);
	}

	return read;
}

int	File_LoadBinary(const char * filename, char *buffer, int buffer_size) {
	int read = 0;

	FILE * f = File_Open(filename, "rb");
	if (f) {
		int size = FileSize(f);
		if (size <= buffer_size) {
			read = (int)fread(buffer, 1, (size_t)size, f);
		}
		fclose(f);
	}

	return read;
}

void image32_s::AllocDataSpace(int pixels) {
	if (mData) {
		free(mData);
	}

	mData = (byte*)malloc(4 * pixels);
}

void height_field_s::AllocDataSpace(int size) {
	if (mData) {
		free(mData);
	}

	mData = (byte*)malloc(size);
}

bool File_LoadBMP(const char * filename, image32_s &image) {
#pragma pack(push, 1)

	struct bmpfilehead_s {
		word		bfType;
		dword		bfSize;
		word		bfReserved1;
		word		bfReserved2;
		dword		bfOffBits;
	};

	struct bmpinfohead_s {
		dword		biSize;
		int32_t		biWidth;
		int32_t		biHeight;
		word		biPlanes;
		word		biBitCount;
		dword		biCompression;
		dword		biSizeImage;
		int32_t		biXPelsPerMeter;
		int32_t		biYPelsPerMeter;
		dword		biClrUsed;
		dword		biClrImportant;
	};

#pragma pack(pop)

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

	int size = File_GetSize(filename);
	char * buf = (char*)malloc(size);
	bool error = false;

	File_LoadBinary(filename, buf, size);

	bmpfilehead_s * filehead = (bmpfilehead_s*)buf;
	bmpinfohead_s * infohead = (bmpinfohead_s*)(filehead + 1);

	if (infohead->biBitCount == 8) {

		if (infohead->biCompression == BI_RGB) { // uncompressed
			int src_line_len = (infohead->biWidth + 3) & ~3;

			int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + 4 * 256 + src_line_len * infohead->biHeight;
			if (valid_size != size) {
				SYS_ERROR("bad size\n");
				error = true;
			}
			else {
				image.mWidth = infohead->biWidth;
				image.mHeight = infohead->biHeight;
				image.AllocDataSpace(infohead->biWidth * 4 * infohead->biHeight);

				byte * palette = (byte*)(infohead + 1);
				byte * src = palette + 4 * 256;

				int dst_line_len = infohead->biWidth * 4;

				for (int h = 0; h < infohead->biHeight; ++h) {
					byte * src_line = src + src_line_len * h;
					byte * dst_line = image.mData + dst_line_len * h;

					for (int w = 0; w < infohead->biWidth; ++w) {
						byte idx = src_line[w];

						byte * src_clr = palette + idx * 4;
						byte * dst_clr = dst_line + w * 4;

						dst_clr[0] = src_clr[2]; // red
						dst_clr[1] = src_clr[1]; // green
						dst_clr[2] = src_clr[0]; // blue
						dst_clr[3] = 255; // opaque
					}
				}
			}
			
		}
		else if (infohead->biCompression == BI_RLE8) {
			image.mWidth = infohead->biWidth;
			image.mHeight = infohead->biHeight;
			image.AllocDataSpace(infohead->biWidth * 4 * infohead->biHeight);

			memset(image.mData, 0xff, infohead->biWidth * 4 * infohead->biHeight);

			byte * palette = (byte*)(infohead + 1);
			byte * src = palette + 4 * 256;

			int dst_line_len = infohead->biWidth * 4;

			byte * s = src;

			int line = 0;
			int clrxpos = 0;
			byte * dst_line = image.mData;

			bool breakloop = false;
			while (!breakloop) {
				byte byte1 = s[0];
				byte byte2 = s[1];
				s += 2;

				if (byte1 > 0) {
					byte runlen = byte1;
					byte clridx = byte2;

					byte * src_clr = palette + clridx * 4;

					for (int i = 0; i < (int)runlen; ++i) {
						byte * dst_clr = dst_line + clrxpos * 4;

						dst_clr[0] = src_clr[2]; // red
						dst_clr[1] = src_clr[1]; // green
						dst_clr[2] = src_clr[0]; // blue
						dst_clr[3] = 255; // opaque

						clrxpos++;
					}
				}
				else { // 0 == byte1
					if (byte2 >= 0x03) {
						byte clrlen = byte2;
						for (int i = 0; i < (int)clrlen; ++i) {
							byte clridx = s[i];

							byte * src_clr = palette + clridx * 4;
							byte * dst_clr = dst_line + clrxpos * 4;

							dst_clr[0] = src_clr[2]; // red
							dst_clr[1] = src_clr[1]; // green
							dst_clr[2] = src_clr[0]; // blue
							dst_clr[3] = 255; // opaque

							clrxpos++;
						}
						s += clrlen;
					}
					else {
						switch (byte2) {
						case 0: // end of line
							line++;
							dst_line = image.mData + dst_line_len * line;
							if (clrxpos != infohead->biWidth) {
								SYS_ERROR("bad data\n");
								error = true;
								breakloop = true;
							}
							clrxpos = 0;
							break;
						case 1: // end of bitmap
							breakloop = true;
							break;
						case 2: 
							// delta.The 2 bytes following the escape contain unsigned values indicating the horizontal 
							// and vertical offsets of the next pixel from the current position.
							SYS_ERROR("did not know how to handle delta yet\n");
							error = true;
							breakloop = true;
							break;
						default:
							SYS_ERROR("bad data\n");
							error = true;
							breakloop = true;
							break;
						}
					}
				}
			}
		}
		else {
			error = true;
		}

	}
	else if (infohead->biBitCount == 16) {
		int src_line_len = (infohead->biWidth * 2 + 3) & ~3;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > size) {
			SYS_ERROR("bad size\n");
			error = true;
		}
		else {
			image.mWidth = infohead->biWidth;
			image.mHeight = infohead->biHeight;
			image.AllocDataSpace(infohead->biWidth * 4 * infohead->biHeight);

			byte * src = (byte*)(infohead + 1);

			int dst_line_len = infohead->biWidth * 4;

			for (int h = 0; h < infohead->biHeight; ++h) {//OpenGL store bottom row first, same as BMP
				byte * src_line = src + src_line_len * h;
				byte * dst_line = image.mData + dst_line_len * h;

				for (int w = 0; w < infohead->biWidth; ++w) {
					word src_clr = *(word*)(src_line + w * 2);
					byte * dst_clr = dst_line + w * 4;

					byte src_b = (byte) (src_clr & 0x001f);
					byte src_g = (byte)((src_clr & 0x07e0) >> 5);
					byte src_r = (byte)((src_clr & 0xf800) >> 11);

					dst_clr[0] = src_r; // red
					dst_clr[1] = src_g; // green
					dst_clr[2] = src_b; // blue
					dst_clr[3] = 255; // opaque
				}
			}
		}		
	}
	else if (infohead->biBitCount == 24) {
		int src_line_len = (infohead->biWidth * 3 + 3) & ~3;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > size) {
			SYS_ERROR("bad size\n");
			error = true;
		}
		else {
			image.mWidth = infohead->biWidth;
			image.mHeight = infohead->biHeight;
			image.AllocDataSpace(infohead->biWidth * 4 * infohead->biHeight);

			byte * src = (byte*)(infohead + 1);

			int dst_line_len = infohead->biWidth * 4;

			for (int h = 0; h < infohead->biHeight; ++h) { //OpenGL store bottom row first, same as BMP
				byte * src_line = src + src_line_len * h;
				byte * dst_line = image.mData + dst_line_len * h;

				for (int w = 0; w < infohead->biWidth; ++w) {
					byte * src_clr = src_line + w * 3;
					byte * dst_clr = dst_line + w * 4;

					dst_clr[0] = src_clr[2]; // red
					dst_clr[1] = src_clr[1]; // green
					dst_clr[2] = src_clr[0]; // blue
					dst_clr[3] = 255; // opaque
				}
			}
		}
	}
	else if (infohead->biBitCount == 32) {
		int src_line_len = infohead->biWidth * 4;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > size) {
			SYS_ERROR("bad size\n");
			error = true;
		}
		else {
			image.mWidth = infohead->biWidth;
			image.mHeight = infohead->biHeight;
			image.AllocDataSpace(infohead->biWidth * 4 * infohead->biHeight);

			byte * src = (byte*)(infohead + 1);

			int dst_line_len = infohead->biWidth * 4;

			for (int h = 0; h < infohead->biHeight; ++h) {
				byte * src_line = src + src_line_len * h;
				byte * dst_line = image.mData + dst_line_len * h;

				for (int w = 0; w < infohead->biWidth; ++w) {
					byte * src_clr = src_line + w * 4;
					byte * dst_clr = dst_line + w * 4;
					dst_clr[0] = src_clr[2]; // red
					dst_clr[1] = src_clr[1]; // green
					dst_clr[2] = src_clr[0]; // blue
					dst_clr[3] = src_clr[3]; // alpha
				}
			}
		}
	}
	else {
		error = true;
	}

	free(buf);

	return !error;
}

bool File_LoadRawHeightFieldFrom8BitBMP(const char * filename, height_field_s &hf) {
	image32_s temp_image;
	if (!File_LoadBMP(filename, temp_image)) {
		return false;
	}

	hf.mWidth = temp_image.mWidth;
	hf.mHeight = temp_image.mHeight;
	hf.AllocDataSpace(hf.mWidth * hf.mHeight);

	int src_line_len = temp_image.mWidth * 4;
	int dst_line_len = hf.mWidth;

	for (int h = 0; h < hf.mHeight; ++h) {
		byte * src_line = temp_image.mData + src_line_len * h;
		byte * dst_line = hf.mData + dst_line_len * h;

		for (int w = 0; w < hf.mWidth; ++w) {
			byte * src_clr = src_line + w * 4;
			byte * dst_z = dst_line + w;

			*dst_z = src_clr[0];
		}
	}

	return true;
}

/*
================================================================================
timer
================================================================================
*/
typedef std::chrono::high_resolution_clock sys_timer_t;
static sys_timer_t::time_point gBaseTime;

void Sys_InitTimer() {
	gBaseTime = sys_timer_t::now();
}

double Sys_GetRelativeTime() {
	sys_timer_t::time_point t = sys_timer_t::now();
	sys_timer_t::duration delta = t - gBaseTime;

	return delta.count() * 1E-9;
}

/*
================================================================================
GL Helper
================================================================================
*/
bool GL_Init() {
	if (GLEW_OK != glewInit()) {
		SYS_ERROR("glewInit error\n");
		return false;
	}

	if (!glewIsSupported("GL_VERSION_4_3")) {
		SYS_ERROR("requre OpenGL 4.3 and above\n");
		return false;
	}

	return true;
}

static const int MAX_SHADER_FILE_SIZE = 65535;

static GLuint CreateShader(const char *filename, GLenum type) {
	int file_size = File_GetSize(filename);
	if (file_size <= 0) {
		return 0;
	}

	if (file_size > MAX_SHADER_FILE_SIZE) {
		return 0;
	}

	GLuint shader = 0;
	shader = glCreateShader(type);
	if (!shader) {
		return 0;
	}
	else {
		char buf[MAX_SHADER_FILE_SIZE];
		file_size = File_LoadText(filename, buf, MAX_SHADER_FILE_SIZE);

		const char * src = buf;
		glShaderSource(shader, 1, &src, 0);
		glCompileShader(shader);
	}

	// check compile status

	GLint compile_status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	// check compile result
	if (compile_status == 1) {
		return shader;
	}
	else {
		GLint log_len = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len) {
#if defined(_MSC_VER)
			char * buf = (char*)_alloca(log_len);
#endif

#if defined(__GNUC__)
			char * buf = (char*)malloc(log_len);
			memset(buf, 0, log_len);
#endif
			glGetShaderInfoLog(shader, log_len, &log_len, buf);
			SYS_ERROR("shader compile error: %s\n", buf);
		}
		return 0;
	}
}

static const GLenum TO_GL_SHADER[] = {
	GL_VERTEX_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER
};

bool GL_CreateProgram(const gl_shader_desc_s &desc, gl_program_s &program) {
	program.mProgram = glCreateProgram();

	for (int s = VERTEX_SHADER; s < SUPPORT_SHADER_COUNT; ++s) {
		const char * filename = desc.mFiles[s];
		if (filename) {
			GLuint shader = CreateShader(filename, TO_GL_SHADER[s]);
			if (!shader) {
				return false;
			}

			glAttachShader(program.mProgram, shader);

			program.mShaders[s] = shader;

		}
	}

	glLinkProgram(program.mProgram);

	// check link status
	GLint link_status = 0;
	glGetProgramiv(program.mProgram, GL_LINK_STATUS, &link_status);

	if (link_status == 1) {
		return true;
	}
	else {
		GLint log_len = 0;
		glGetProgramiv(program.mProgram, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len) {
#if defined(_MSC_VER)
			char * buf = (char*)_alloca(log_len);
#endif

#if defined(__GNUC__)
			char * buf = (char*)malloc(log_len);
			memset(buf, 0, log_len);
#endif
			glGetProgramInfoLog(program.mProgram, log_len, &log_len, buf);
			SYS_ERROR("program link error: %s\n", buf);
		}

		GL_DeleteProgram(program);
		return false;
	}
}

bool GL_CreateProgram(const char *res_dir, const char * vs, const char *fs, gl_program_s &prog) {
	char vsfilename[MAX_PATH];
	char fsfilename[MAX_PATH];

	sprintf_(vsfilename, "%s%sshaders%s%s", res_dir, PATH_SEPERATOR, PATH_SEPERATOR, vs);
	sprintf_(fsfilename, "%s%sshaders%s%s", res_dir, PATH_SEPERATOR, PATH_SEPERATOR, fs);

	gl_shader_desc_s shader_desc;

	shader_desc.mFiles[VERTEX_SHADER] = vsfilename;
	shader_desc.mFiles[FRAGMENT_SHADER] = fsfilename;

	return GL_CreateProgram(shader_desc, prog);
}

void GL_DeleteProgram(gl_program_s &program) {
	if (program.mProgram) {
		for (int s = VERTEX_SHADER; s < SUPPORT_SHADER_COUNT; ++s) {
			if (program.mShaders[s]) {
				glDetachShader(program.mProgram, program.mShaders[s]);
				glDeleteShader(program.mShaders[s]);
				program.mShaders[s] = 0;
			}
		}

		glDeleteProgram(program.mProgram);
		program.mProgram = 0;
	}
}

GLuint GL_CreateTexture2D(const gl_texture2d_desc_s &desc) {
	image32_s image;
	if (!File_LoadBMP(desc.mFileName, image)) {
		return 0;
	}

	GLuint t = 0;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);

	GLenum wrap_mode = desc.mRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);

	GLenum min_filter = desc.mMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST;
	GLenum mag_filter = desc.mMipMap ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.mWidth, image.mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.mData);
	if (desc.mMipMap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	return t;
}

GLuint GL_CreateTexture2D(const char *res_dir, const char * filename, bool mipmap, bool repeat) {
	char fullfilename[MAX_PATH];
	sprintf_(fullfilename, "%s%s%s", res_dir, PATH_SEPERATOR, filename);

	gl_texture2d_desc_s tex2d_desc;

	tex2d_desc.mFileName = fullfilename;
	tex2d_desc.mMipMap = mipmap;
	tex2d_desc.mRepeat = repeat;

	return GL_CreateTexture2D(tex2d_desc);
}

GLuint GL_CreateUniformBuffer(size_t size) {
	GLuint ubo = 0;

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

	return ubo;
}

int GL_CheckError() {
	int error_count = 0;
	GLenum r = GL_NO_ERROR;
	do {
		r = glGetError();
		if (r != GL_NO_ERROR) {
			error_count++;
			switch (r) {
			case GL_INVALID_ENUM:
				SYS_ERROR("GL_INVALID_ENUM\n");
				break;
			case GL_INVALID_VALUE:
				SYS_ERROR("GL_INVALID_VALUE\n");
				break;
			case GL_INVALID_OPERATION:
				SYS_ERROR("GL_INVALID_OPERATION\n");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				SYS_ERROR("GL_INVALID_FRAMEBUFFER_OPERATION\n");
				break;
			case GL_OUT_OF_MEMORY:
				SYS_ERROR("GL_OUT_OF_MEMORY\n");
				break;
			case GL_STACK_UNDERFLOW:
				SYS_ERROR("GL_STACK_UNDERFLOW\n");
				break;
			case GL_STACK_OVERFLOW:
				SYS_ERROR("GL_STACK_OVERFLOW\n");
				break;
			default:
				SYS_ERROR("unknown gl error: %d\n", r);
				break;
			}
		}

	} while (r != GL_NO_ERROR);

	return error_count;
}
