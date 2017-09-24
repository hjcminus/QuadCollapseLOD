/*
application
*/

#include "Precompiled.h"

DemoApp::DemoApp():
	mRenderer(nullptr),
	mTerrain(nullptr),
	mYaw(0.0f),
	mPitch(0.0f),
	mMoveSpeed(10.0f)
{
}

DemoApp::~DemoApp() {
	Shutdown();
}

bool DemoApp::Init(const config_s &cfg) {
	mRenderer = NEW__ Renderer();
	mTerrain = NEW__ Terrain();

	if (!mRenderer->Init(cfg)) {
		return false;
	}

	if (!mTerrain->Init(cfg)) {
		return false;
	}

	mRenderer->SetHeightFieldSize(mTerrain->GetSize()); // edge length
	
	mCamera.mPos = cfg.mCameraPos;
	mCamera.mTarget = mCamera.mPos + START_FORWARD;
	mCamera.mUp = START_UP;
	mCamera.mZNear = cfg.mZNear;
	mCamera.mZFar = cfg.mZFar;
	mCamera.mFovy = cfg.mFovy;

	mYaw = cfg.mCameraYaw;
	mPitch = cfg.mCameraPitch;

	UpdateCameraOrientation();

	return true;
}

void DemoApp::Shutdown() {
	if (mTerrain) {
		delete mTerrain;
		mTerrain = nullptr;
	}

	if (mRenderer) {
		delete mRenderer;
		mRenderer = nullptr;
	}
}

void DemoApp::UpdateCameraOrientation() {
	// clamp rotation
	mYaw = ClampYaw(mYaw);
	mPitch = ClampPitch(mPitch);

	// update rotation
	mat4 yawMatrix = rotate(mat4(1.0f), mYaw * PI / 180.0f, START_UP);

	mCameraForward = (yawMatrix * vec4(START_FORWARD, 0.0f)).xyz;
	mCameraRight = (yawMatrix * vec4(START_RIGHT, 0.0f)).xyz;

	mat4 pitchMatrix = rotate(mat4(1.0f), mPitch * PI / 180.0f, mCameraRight);

	mCameraForward = (pitchMatrix * vec4(mCameraForward, 0.0f)).xyz;
	mCamera.mUp = (pitchMatrix * vec4(START_UP, 0.0f)).xyz;

	mCameraForward = normalize(mCameraForward);
	mCameraRight = normalize(mCameraRight);
	mCamera.mUp = normalize(mCamera.mUp);
}

void DemoApp::ResizeViewport(int width, int height) {
	mRenderer->ResizeViewport(width, height);
}

void DemoApp::ProcessInput(float frame_time, const input_s &input) {
	mYaw += input.mMouseDeltaX * -0.1f;
	mPitch += input.mMouseDeltaY * -0.1f;

	UpdateCameraOrientation();
	
	// check movement
	if (input.mMovementKey == MK_FORWARD) {
		mCamera.mPos += mCameraForward * mMoveSpeed * frame_time;
	}
	else if (input.mMovementKey == MK_BACKWARD) {
		mCamera.mPos -= mCameraForward * mMoveSpeed * frame_time;
	}
	else if (input.mMovementKey == MK_LEFT) {
		mCamera.mPos -= mCameraRight * mMoveSpeed * frame_time;
	}
	else if (input.mMovementKey == MK_RIGHT) {
		mCamera.mPos += mCameraRight * mMoveSpeed * frame_time;
	}
	else if (input.mMovementKey == MK_STRAIGHT_UP) {
		mCamera.mPos += START_UP * mMoveSpeed * frame_time;
	}
	else if (input.mMovementKey == MK_STRAIGHT_DOWN) {
		mCamera.mPos -= START_UP * mMoveSpeed * frame_time;
	}

	mCamera.mTarget = mCamera.mPos + mCameraForward;

	int view_width, view_height;
	mRenderer->GetViewport(view_width, view_height);
	mFrumstumPlane.Setup(view_width, view_height, mCamera);

	// update terrain
	mTerrain->Update(mCamera, mFrumstumPlane);

	const triangle_mesh_s & tm = mTerrain->GetMesh();
	mRenderer->UpdateTerrainMesh(tm);
	mRenderer->Printf("draw triangle count: %d, camera pos: %d, %d, %d, move speed: %f\n", tm.mNumTriangles,
		(int)mCamera.mPos.x, (int)mCamera.mPos.y, (int)mCamera.mPos.z, mMoveSpeed);
}

void DemoApp::SetMoveSpeed(float move_speed) {
	mMoveSpeed = move_speed;
}

void DemoApp::UpdateScreen(uint32_t draw_flags) {
	mRenderer->Draw(mCamera, draw_flags);
}
