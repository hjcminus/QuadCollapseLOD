/*
render text
*/

#include "Precompiled.h"

RenderText::RenderText():
	mVAO(0),
	mVBO(0),
	mFontTexture(0),
	mNumChar(0)
{
}

RenderText::~RenderText() {
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteTextures(1, &mFontTexture);
	GL_DeleteProgram(mProgram_Text);
}

bool RenderText::Init(const config_s &cfg) {
	// init texture coordinates
	int count = 0;
	for (float y = 128.0f; y > 16.0f; y -= 16.0f) {
		for (float x = 0.0f; x < 112.0f; x += 8.0f) {
			mCharInfos[count].mS0 = x / 128.0f;
			mCharInfos[count].mT0 = (y - 16.0f) / 128.0f;
			mCharInfos[count].mS1 = (x + 8.0f) / 128.0f;
			mCharInfos[count].mT1 = y / 128.0f;
			count++;
			if (count >= 95) {
				break;
			}
		}

		if (count >= 95) {
			break;
		}
	}

	// create program
	if (!GL_CreateProgram(cfg.mResDir, "text.vert", "text.frag", mProgram_Text)) {
		return false;
	}

	mFontTexture = GL_CreateTexture2D(cfg.mResDir, "font\\font.bmp", true, true);
	if (!mFontTexture) {
		return false;
	}

	size_t size = sizeof(vertex_s) * MAX_PRINT_TEXT_LEN * 4;

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_s), (const void *)offsetof(vertex_s, mPos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_s), (const void *)offsetof(vertex_s, mTexCoord));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void RenderText::Print(const char *text) {
#define TEXT_CX	8.0f
#define TEXT_CY	16.0f

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	vertex_s * cur = (vertex_s*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	float x = 0.0f;
	float y = 0.0f;

	mNumChar = 0;
	const char * pc = text;
	while (*pc) {
		if (mNumChar >= MAX_PRINT_TEXT_LEN - 1) {
			break; // truncate
		}

		mNumChar++;
		char c = *pc;

		// clamp char
		if (c < 32) {
			c = 32;
		}

		int index = c - 32;

		if (index >= 95) {
			index = 0;
		}

		char_info_s * tex_coord = mCharInfos + index;

		cur[0].mPos.x = x;
		cur[0].mPos.y = y;
		cur[0].mPos.z = 0.0f;
		cur[0].mTexCoord.x = tex_coord->mS0;
		cur[0].mTexCoord.y = tex_coord->mT0;

		cur[1].mPos.x = x + TEXT_CX;
		cur[1].mPos.y = y;
		cur[1].mPos.z = 0.0f;
		cur[1].mTexCoord.x = tex_coord->mS1;
		cur[1].mTexCoord.y = tex_coord->mT0;

		cur[2].mPos.x = x + TEXT_CX;
		cur[2].mPos.y = y + TEXT_CY;
		cur[2].mPos.z = 0.0f;
		cur[2].mTexCoord.x = tex_coord->mS1;
		cur[2].mTexCoord.y = tex_coord->mT1;

		cur[3].mPos.x = x;
		cur[3].mPos.y = y + TEXT_CY;
		cur[3].mPos.z = 0.0f;
		cur[3].mTexCoord.x = tex_coord->mS0;
		cur[3].mTexCoord.y = tex_coord->mT1;

		x += TEXT_CX;
		cur += 4;

		pc++;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderText::Draw(UniformBuffers *ub) {
	if (mNumChar) {
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);

		glUseProgram(mProgram_Text.mProgram);
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ub->mUBO[UniformBuffers::UBO_ORTHO_MODEL_VIEW_PROJ_MATRIX]);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ub->mUBO[UniformBuffers::UBO_FONT_COLOR]);
			glBindVertexArray(mVAO);

			glBindTextureUnit(0, mFontTexture);
			glDrawArrays(GL_QUADS, 0, mNumChar * 4);
		}

		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}
}
