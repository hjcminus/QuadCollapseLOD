/*
render terrain
*/

#include "Precompiled.h"

RenderTerrain::RenderTerrain():
	mDrawWireframe(false),
	mVAO(0),
	mVBO(0),
	mBaseTexture(0),
	mDetailTexture(0),
	mNumTerrainTriangles(0),
	mVertexBufferCapacity(0)
{
}

RenderTerrain::~RenderTerrain() {
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteTextures(1, &mDetailTexture);
	glDeleteTextures(1, &mBaseTexture);
	GL_DeleteProgram(mProgram_Wireframe);
	GL_DeleteProgram(mProgram_Terrain);
}

bool RenderTerrain::Init(const config_s &cfg) {
	// create program
	if (!GL_CreateProgram(cfg.mResDir, "terrain.vert", "terrain.frag", mProgram_Terrain)) {
		return false;
	}

	if (!GL_CreateProgram(cfg.mResDir, "terrain_wireframe.vert", "terrain_wireframe.frag", mProgram_Wireframe)) {
		return false;
	}

	mBaseTexture = GL_CreateTexture2D(cfg.mResDir, cfg.mTerrainBase, true, true);
	if (!mBaseTexture) {
		return false;
	}

	mDetailTexture = GL_CreateTexture2D(cfg.mResDir, cfg.mTerrainDetail, true, true);
	if (!mDetailTexture) {
		return false;
	}

	mVertexBufferCapacity = 1024 * 1024;
	RecreateVertexBuffer();

	return true;
}

void RenderTerrain::ToggleWireframeMode() {
	mDrawWireframe = !mDrawWireframe;
}

void RenderTerrain::Update(const triangle_mesh_s & tm) {
	size_t size = sizeof(vec3) * tm.mNumTriangles * 3;
	mNumTerrainTriangles = tm.mNumTriangles;

	if (tm.mNumTriangles > 0) {
		if (tm.mNumTriangles * 3 > mVertexBufferCapacity) {
			mVertexBufferCapacity = tm.mNumTriangles * 3;
			RecreateVertexBuffer();
		}

		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, tm.mVertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

int	RenderTerrain::GetDrawTriangleCount() const {
	return mNumTerrainTriangles;
}

void RenderTerrain::Draw(UniformBuffers *ub, uint32_t draw_flags) {
	if (mNumTerrainTriangles > 0) {

		if (draw_flags & DF_SOLID_TERRAIN) {
			glUseProgram(mProgram_Terrain.mProgram);
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, ub->mUBO[UniformBuffers::UBO_MODEL_VIEW_PROJ_MATRIX]);
				glBindBufferBase(GL_UNIFORM_BUFFER, 1, ub->mUBO[UniformBuffers::UBO_HEIGHT_FIELD]);
				glBindBufferBase(GL_UNIFORM_BUFFER, 2, ub->mUBO[UniformBuffers::UBO_FOG]);
				glBindVertexArray(mVAO);

				glBindTextureUnit(0, mBaseTexture);
				glBindTextureUnit(1, mDetailTexture);
				glDrawArrays(GL_TRIANGLES, 0, mNumTerrainTriangles * 3);
			}
		}

		if (draw_flags & DF_WIREFRAME_TERRAIN) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(-1, 0);

			glUseProgram(mProgram_Wireframe.mProgram);
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, ub->mUBO[UniformBuffers::UBO_MODEL_VIEW_PROJ_MATRIX]);
				glBindBufferBase(GL_UNIFORM_BUFFER, 1, ub->mUBO[UniformBuffers::UBO_WIREFRAME_COLOR]);
				glBindVertexArray(mVAO);
				glDrawArrays(GL_TRIANGLES, 0, mNumTerrainTriangles * 3);
			}

			glPolygonOffset(0, 0);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

void RenderTerrain::RecreateVertexBuffer() {
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);

	size_t init_size = sizeof(vec3) * mVertexBufferCapacity;

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, init_size, nullptr, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (const void *)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
