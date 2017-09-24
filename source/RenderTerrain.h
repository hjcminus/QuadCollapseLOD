/*
render terrain
*/

#pragma once

class RenderTerrain {
public:
	RenderTerrain();
	~RenderTerrain();

	bool						Init(const config_s &cfg);

	void						ToggleWireframeMode();
	void						Update(const triangle_mesh_s & tm);
	int							GetDrawTriangleCount() const;
	void						Draw(UniformBuffers *ub, uint32_t draw_flags);

private:

	bool						mDrawWireframe;

	GLuint						mVAO;
	GLuint						mVBO;
	GLuint						mBaseTexture;
	GLuint						mDetailTexture;
	int							mVertexBufferCapacity;

	int							mNumTerrainTriangles;

	gl_program_s				mProgram_Terrain;
	gl_program_s				mProgram_Wireframe;

	void						RecreateVertexBuffer();
};
