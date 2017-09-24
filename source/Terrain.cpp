/*
terrain geometry
*/

#include "Precompiled.h"

Terrain::Terrain():
	mQuadCollapseMesh(nullptr)
{
}

Terrain::~Terrain() {
	Shutdown();
}

bool Terrain::Init(const config_s &cfg) {
	if (mQuadCollapseMesh) {
		delete mQuadCollapseMesh;
		mQuadCollapseMesh = nullptr;
	}

	char fullfilename[MAX_PATH];
	sprintf_s(fullfilename, "%s\\%s", cfg.mResDir, cfg.mTerrainHeight);

	height_field_s hf;
	if (!File_LoadRawHeightFieldFrom8BitBMP(fullfilename, hf)) {
		return false;
	}

	for (int h = 0; h < hf.mHeight; ++h) {
		byte * line = hf.mData + hf.mWidth * h;
		for (int w = 0; w < hf.mWidth; ++w) {

			float x = (float)w;
			float y = (float)h;
			float z = (float)line[w] * cfg.mTerrainZScale;

			mVertices.Add(vec3(x, y, z));
		}
	}

	mQuadCollapseMesh = NEW__ QuadCollapseMesh();
	return mQuadCollapseMesh->Build(mVertices.GetItems(), hf.mWidth, hf.mHeight);
}

void Terrain::Shutdown() {
	if (mQuadCollapseMesh) {
		delete mQuadCollapseMesh;
		mQuadCollapseMesh = nullptr;
	}
}

int	Terrain::GetSize() const {
	return mQuadCollapseMesh->GetMaxLevelVerticesLength() - 1;
}

void Terrain::Update(const camera_s &cam, const frustum_plane_s &fp) {
	//double t1 = Sys_GetRelativeTime();
	mQuadCollapseMesh->Update(cam, fp);
	//double t2 = Sys_GetRelativeTime();

	//int ms = (int)((t2 - t1) * 1000.0);

	//printf("update time elapsed: %d ms\n", ms);
}

const triangle_mesh_s & Terrain::GetMesh() const {
	return mQuadCollapseMesh->GetActiveMesh();
}
