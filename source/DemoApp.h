/*
application
*/

#pragma once

class DemoApp {
public:

	DemoApp();
	~DemoApp();

	bool						Init(const config_s &cfg);
	void						Shutdown();

	void						ResizeViewport(int width, int height);
	void						ProcessInput(float frame_time, const input_s &input);
	void						SetMoveSpeed(float move_speed);
	void						UpdateScreen(uint32_t draw_flags);

private:

	camera_s					mCamera;
	frustum_plane_s				mFrumstumPlane;
	float						mYaw;
	float						mPitch;
	float						mMoveSpeed;

	vec3						mCameraForward;
	vec3						mCameraRight;

	// sub system
	Renderer *					mRenderer;
	Terrain	*					mTerrain;

	void						UpdateCameraOrientation();
};
