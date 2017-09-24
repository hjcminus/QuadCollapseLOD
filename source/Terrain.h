/*
terrain geometry
*/

#pragma once

class Terrain {
public:
	Terrain();
	~Terrain();

	bool						Init(const config_s &cfg);
	void						Shutdown();

	int							GetSize() const;
	void						Update(const camera_s &cam, const frustum_plane_s &fp);
	const triangle_mesh_s &		GetMesh() const;


private:

	ItemArray<vec3, 1024 * 1024>	mVertices;

	QuadCollapseMesh *			mQuadCollapseMesh;
};
