/*
render text
*/

#pragma once

class RenderText {
public:
	RenderText();
	~RenderText();

	bool						Init(const config_s &cfg);
	void						Print(const char *text);
	void						Draw(UniformBuffers *ub);

private:

	// texture coordinate
	struct char_info_s {
		float					mS0;
		float					mT0;
		float					mS1;
		float					mT1;
	};

	char_info_s					mCharInfos[95];

	GLuint						mVAO;
	GLuint						mVBO;
	GLuint						mFontTexture;
	int							mNumChar;

	gl_program_s				mProgram_Text;
};
