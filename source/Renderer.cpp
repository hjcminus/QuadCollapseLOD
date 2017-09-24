/*
renderer
*/

#include "Precompiled.h"
#include <stdarg.h>

Renderer::Renderer():
	mViewWidth(1),
	mViewHeight(1),
	mUniformBuffer(nullptr),
	mSkybox(nullptr),
	mTerrain(nullptr),
	mTextOutput(nullptr)
{
}

Renderer::~Renderer() {
	Shutdown();
}

bool Renderer::Init(const config_s &cfg) {
	mUniformBuffer = NEW__ UniformBuffers();
	if (!mUniformBuffer->Init()) {
		return false;
	}

	mUniformBuffer->SetFog(cfg.mFogColor, cfg.mFogDensity);
	mUniformBuffer->SetWireframeColor(cfg.mWireframeColor);
	mUniformBuffer->SetFontColor(cfg.mFontColor);

	mSkybox = NEW__ Skybox();
	if (!mSkybox->Init(cfg)) {
		return false;
	}

	mTerrain = NEW__ RenderTerrain();
	if (!mTerrain->Init(cfg)) {
		return false;
	}

	mTextOutput = NEW__ RenderText();
	if (!mTextOutput->Init(cfg)) {
		return false;
	}

	glClearColor(cfg.mBackgroundColor.x, cfg.mBackgroundColor.y, cfg.mBackgroundColor.z, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	return true;
}

void Renderer::Shutdown() {
	if (mTextOutput) {
		delete mTextOutput;
		mTextOutput = nullptr;
	}

	if (mTerrain) {
		delete mTerrain;
		mTerrain = nullptr;
	}

	if (mSkybox) {
		delete mSkybox;
		mSkybox = nullptr;
	}

	if (mUniformBuffer) {
		delete mUniformBuffer;
		mUniformBuffer = nullptr;
	}
}

void Renderer::ResizeViewport(int view_width, int view_height) {
	mViewWidth = max(1, view_width);
	mViewHeight = max(1, view_height);

	glViewport(0, 0, mViewWidth, mViewHeight);
}

void Renderer::GetViewport(int &view_width, int &view_height) const {
	view_width = mViewWidth;
	view_height = mViewHeight;
}

void Renderer::ToggleWireframeMode() {
	mTerrain->ToggleWireframeMode();
}

void Renderer::SetHeightFieldSize(int size) {
	mUniformBuffer->SetHeightFieldSize(size);
}

void Renderer::UpdateTerrainMesh(const triangle_mesh_s & tm) {
	mTerrain->Update(tm);
}

void Renderer::Printf(const char *fmt, ...) {
	char buffer[MAX_PRINT_TEXT_LEN];

	va_list argptr;
	va_start(argptr, fmt);
	vsprintf_s(buffer, fmt, argptr);
	va_end(argptr);

	mTextOutput->Print(buffer);
}

void Renderer::Draw(const camera_s &cam, uint32_t draw_flags) {
	SetupUniformBuffers(cam);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (draw_flags & DF_SKYBOX) {
		mSkybox->Draw(mUniformBuffer);
	}

	mTerrain->Draw(mUniformBuffer, draw_flags);
	mTextOutput->Draw(mUniformBuffer);
	
	glFinish();

	//GL_CheckError();
}

void Renderer::SetupUniformBuffers(const camera_s &cam) {
	mat4 prespective_proj_matrix = perspective(cam.mFovy * PI / 180.0f, (float)mViewWidth / mViewHeight, cam.mZNear, cam.mZFar);
	mat4 orthographic_proj_matrix = ortho(0.0f, (float)mViewWidth, 0.0f, (float)mViewHeight);
	mat4 orthographic_mvp_matrix = translate(orthographic_proj_matrix, vec3(4.0f, mViewHeight - 20.0f, 0.0f)); // move text to left top
	mat4 view_matrix = lookAt(cam.mPos, cam.mTarget, cam.mUp);
	mat4 mvpmatrix = prespective_proj_matrix * view_matrix;
	mat4 viewmatrix_followcamera = lookAt(vec3(0.0f), cam.mTarget - cam.mPos, cam.mUp);
	mat4 mvpmatrix_followcamera = prespective_proj_matrix * viewmatrix_followcamera;

	mUniformBuffer->SetModelViewProjMatrix(mvpmatrix, view_matrix);
	mUniformBuffer->SetModelViewProjMatrix_FollowCamera(mvpmatrix_followcamera);
	mUniformBuffer->SetOrthoModelViewProjMatrix(orthographic_mvp_matrix);
}
