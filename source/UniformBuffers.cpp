/*
shared uniform buffer
*/

#include "Precompiled.h"

UniformBuffers::UniformBuffers() {
	memset(mUBO, 0, sizeof(mUBO));
}

UniformBuffers::~UniformBuffers() {
	glDeleteBuffers(UBO_COUNT, mUBO);
}

bool UniformBuffers::Init() {
	mUBO[UBO_MODEL_VIEW_PROJ_MATRIX] = GL_CreateUniformBuffer(sizeof(mat4) * 2);
	mUBO[UBO_MODEL_VIEW_PROJ_MATRIX_FOLLOW_CAMERA] = GL_CreateUniformBuffer(sizeof(mat4));
	mUBO[UBO_ORTHO_MODEL_VIEW_PROJ_MATRIX] = GL_CreateUniformBuffer(sizeof(mat4));
	mUBO[UBO_WIREFRAME_COLOR] = GL_CreateUniformBuffer(sizeof(vec4));
	mUBO[UBO_FOG] = GL_CreateUniformBuffer(sizeof(vec4));
	mUBO[UBO_FONT_COLOR] = GL_CreateUniformBuffer(sizeof(vec4));
	mUBO[UBO_HEIGHT_FIELD] = GL_CreateUniformBuffer(sizeof(vec4));

	for (int i = 0; i < UBO_COUNT; ++i) {
		if (!mUBO[i]) {
			return false;
		}
	}

	return true;
}

void UniformBuffers::SetModelViewProjMatrix(const mat4 &mvp, const mat4 &mv) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_MODEL_VIEW_PROJ_MATRIX]);
	GLubyte * buf = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &mvp, sizeof(mvp));
	buf += sizeof(mat4);
	memcpy(buf, &mv, sizeof(mv));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetModelViewProjMatrix_FollowCamera(const mat4 &mvp) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_MODEL_VIEW_PROJ_MATRIX_FOLLOW_CAMERA]);
	GLubyte * buf = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &mvp, sizeof(mvp));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetOrthoModelViewProjMatrix(const mat4 &mvp) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_ORTHO_MODEL_VIEW_PROJ_MATRIX]);
	GLubyte * buf = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &mvp, sizeof(mvp));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetFog(const vec3 &fog_color, float density) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_FOG]);
	GLubyte * buf = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &fog_color, sizeof(fog_color));
	buf += sizeof(vec3);
	memcpy(buf, &density, sizeof(density));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetWireframeColor(const vec3 &wireframe_color) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_WIREFRAME_COLOR]);
	vec4 * buf = (vec4*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	*buf = vec4(wireframe_color, 1.0f);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetFontColor(const vec3 &font_color) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_FONT_COLOR]);
	vec4 * buf = (vec4*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	*buf = vec4(font_color, 1.0f);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void UniformBuffers::SetHeightFieldSize(int size) {
	glBindBuffer(GL_UNIFORM_BUFFER, mUBO[UBO_HEIGHT_FIELD]);
	vec4 * buf = (vec4*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	buf->x = (float)size;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}
