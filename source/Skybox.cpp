/*
skybox
*/

#include "Precompiled.h"

Skybox::Skybox():
	mVAO(0),
	mVBO(0)
{
	memset(mTextures, 0, sizeof(mTextures));
}

Skybox::~Skybox() {
	glDeleteTextures(6, mTextures);
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);
	GL_DeleteProgram(mProgram_SkyBox);
}

bool Skybox::Init(const config_s &cfg) {
	// create program
	if (!GL_CreateProgram(cfg.mResDir, "skybox.vert", "skybox.frag", mProgram_SkyBox)) {
		return false;
	}

	// create VAO & VBO
	vec3 unique_vertices[8];

	unique_vertices[0] = vec3( 1.0f,  1.0f,  1.0f);
	unique_vertices[1] = vec3(-1.0f,  1.0f,  1.0f);
	unique_vertices[2] = vec3(-1.0f, -1.0f,  1.0f);
	unique_vertices[3] = vec3( 1.0f, -1.0f,  1.0f);
	unique_vertices[4] = vec3( 1.0f,  1.0f, -1.0f);
	unique_vertices[5] = vec3(-1.0f,  1.0f, -1.0f);
	unique_vertices[6] = vec3(-1.0f, -1.0f, -1.0f);
	unique_vertices[7] = vec3( 1.0f, -1.0f, -1.0f);

	// scale up a bit, avoid clipped by near plan
	for (int i = 0; i < 8; ++i) {
		unique_vertices[i] *= 64.0f;
	}

	vertex_s vertices[6 * 4];

	vertex_s * v = vertices;

	// left
	v[0].mPos = unique_vertices[6];
	v[1].mPos = unique_vertices[5];
	v[2].mPos = unique_vertices[2];
	v[3].mPos = unique_vertices[1];
	v += 4;

	// front
	v[0].mPos = unique_vertices[5];
	v[1].mPos = unique_vertices[4];
	v[2].mPos = unique_vertices[1];
	v[3].mPos = unique_vertices[0];
	v += 4;

	// right
	v[0].mPos = unique_vertices[4];
	v[1].mPos = unique_vertices[7];
	v[2].mPos = unique_vertices[0];
	v[3].mPos = unique_vertices[3];
	v += 4;

	// back
	v[0].mPos = unique_vertices[7];
	v[1].mPos = unique_vertices[6];
	v[2].mPos = unique_vertices[3];
	v[3].mPos = unique_vertices[2];
	v += 4;

	// top
	v[0].mPos = unique_vertices[1];
	v[1].mPos = unique_vertices[0];
	v[2].mPos = unique_vertices[2];
	v[3].mPos = unique_vertices[3];
	v += 4;

	// bottom
	v[0].mPos = unique_vertices[6];
	v[1].mPos = unique_vertices[7];
	v[2].mPos = unique_vertices[5];
	v[3].mPos = unique_vertices[4];

	for (int i = 0; i < 6; ++i) {
		vertices[i * 4].mTexCoord = vec2(0.0f);
		vertices[i * 4 + 1].mTexCoord = vec2(1.0f, 0.0f);
		vertices[i * 4 + 2].mTexCoord = vec2(0.0f, 1.0f);
		vertices[i * 4 + 3].mTexCoord = vec2(1.0f);
	}

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_s) * 24, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_s), (const void *)offsetof(vertex_s, mPos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_s), (const void *)offsetof(vertex_s, mTexCoord));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// load textures
	const char * FILES[] = { "lf.bmp", "ft.bmp", "rt.bmp", "bk.bmp", "tp.bmp", "bt.bmp" };
	for (int i = 0; i < 6; ++i) {
		char filename[MAX_PATH];
		sprintf_(filename, "%s%s%s", cfg.mSkyboxDir, PATH_SEPERATOR, FILES[i]);
		mTextures[i] = GL_CreateTexture2D(cfg.mResDir, filename, true, false);
		if (!mTextures[i]) {
			return false;
		}
	}

	return true;
}

void Skybox::Draw(UniformBuffers *ub) {
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(mProgram_SkyBox.mProgram);
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ub->mUBO[UniformBuffers::UBO_MODEL_VIEW_PROJ_MATRIX_FOLLOW_CAMERA]);
		glBindVertexArray(mVAO);

		for (int i = 0; i < 6; ++i) {
			glBindTextureUnit(0, mTextures[i]);
			glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}
