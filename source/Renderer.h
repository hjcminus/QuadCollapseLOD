/*
renderer
*/

#pragma once

class Renderer {
public:

	Renderer();
	~Renderer();

	bool						Init(const config_s &cfg);
	void						Shutdown();

	void						ResizeViewport(int view_width, int view_height);
	void						GetViewport(int &view_width, int &view_height) const;
	void						ToggleWireframeMode();
	void						SetHeightFieldSize(int size);
	void						UpdateTerrainMesh(const triangle_mesh_s & tm);
	void						Printf(const char *fmt, ...);

	void						Draw(const camera_s &cam, uint32_t draw_flags);

private:

	int							mViewWidth;
	int							mViewHeight;

	UniformBuffers *			mUniformBuffer;
	Skybox *					mSkybox;
	RenderTerrain *				mTerrain;
	RenderText *				mTextOutput;

	void						SetupUniformBuffers(const camera_s &cam);
};
