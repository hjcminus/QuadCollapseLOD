/*
quad collapse mesh
*/

#pragma once

/*
================================================================================
QuadCollapseMesh
================================================================================
*/
#define	MAX_QUAD_LEVEL_COUNT	13

struct vert_node_s;
struct quad_node_s;
struct quad_leaf_s;

class QuadCollapseMesh {
public:

	QuadCollapseMesh();
	~QuadCollapseMesh();

	bool						Build(const vec3 *vertices, int width, int height);
	void						Update(const camera_s &cam, const frustum_plane_s &fp);
	int							GetMaxLevelVerticesLength() const;
	const triangle_mesh_s &		GetActiveMesh() const;

private:

	uint32						mUpdateFrame;
	const vec3 *				mOriginalPosRef;
	uint32_t					mMaxLevel;
	int							mMaxLevelVerticesLength;

	// memory pool
	vert_node_s *				mVertNodePool;
	quad_node_s *				mQuadNodePool;
	quad_leaf_s *				mQuadLeafPool;
	int							mVertNodePoolSize;
	int							mQuadNodePoolSize;
	int							mQuadLeafPoolSize;
	int							mQuadNodePoolAllocated;
	int							mQuadLeafPoolAllocated;

	float						mVertNodesActiveDistance[MAX_QUAD_LEVEL_COUNT];
	float						mQuadNodesCullRadius[MAX_QUAD_LEVEL_COUNT];
	int							mVertNodesLevelOffset[MAX_QUAD_LEVEL_COUNT];

	ItemArray<vec3, 65536>		mActiveVertices;
	triangle_mesh_s				mActiveMesh;

	// root nodes
	quad_node_s	*				mRootQuadnode;
	vert_node_s *				mRootVertnodes[4];

	void						BuildVertNodes();
	quad_node_s *				RecursiveBuildQuadNodes(uint32_t level, int32_t x0, int32_t y0, int32_t step);
	void						CollapseQuad(quad_node_s *quad_node, int32_t x0, int32_t y0, int32_t step);

	vert_node_s *				GetVertNode(uint32_t level, int32_t x, int32_t y, bool init_mode);
	quad_node_s *				AllocQuadNode();
	quad_leaf_s *				AllocQuadLeaf();

	void						RecursiveUpdateVertNode(const vec3 &view_pos, const frustum_plane_s &fp, vert_node_s * vert_node);
	void						QuadNodeSetBoundary(const frustum_plane_s &fp, quad_node_s *quad_node);

	void						RecursiveSetActiveMesh(quad_node_s *quad_node);
	void						AddActiveVertNode(vert_node_s * vert_node);
};
