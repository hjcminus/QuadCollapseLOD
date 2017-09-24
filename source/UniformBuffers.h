/*
shared uniform buffer
*/

#pragma once

struct UniformBuffers {
	enum {
		UBO_MODEL_VIEW_PROJ_MATRIX,
		UBO_MODEL_VIEW_PROJ_MATRIX_FOLLOW_CAMERA,
		UBO_ORTHO_MODEL_VIEW_PROJ_MATRIX,
		UBO_WIREFRAME_COLOR,
		UBO_FOG,
		UBO_FONT_COLOR,
		UBO_HEIGHT_FIELD,

		UBO_COUNT
	};

	GLuint						mUBO[UBO_COUNT];

	UniformBuffers();
	~UniformBuffers();

	bool						Init();

	void						SetModelViewProjMatrix(const mat4 &mvp, const mat4 &mv);
	void						SetModelViewProjMatrix_FollowCamera(const mat4 &mvp);
	void						SetOrthoModelViewProjMatrix(const mat4 &mvp);
	void						SetFog(const vec3 &fog_color, float density);
	void						SetWireframeColor(const vec3 &wireframe_color);
	void						SetFontColor(const vec3 &font_color);
	void						SetHeightFieldSize(int size);
};
