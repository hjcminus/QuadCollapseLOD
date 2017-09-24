/*
skybox
*/

#pragma once

class Skybox {
public:
	Skybox();
	~Skybox();

	bool						Init(const config_s &cfg);
	void						Draw(UniformBuffers *ub);

private:

	uint32_t					mVAO;
	uint32_t					mVBO;
	uint32_t					mTextures[6];

	gl_program_s				mProgram_SkyBox;
};
