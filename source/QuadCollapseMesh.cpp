/*
quad collapse mesh
*/

#include "Precompiled.h"

/*
================================================================================
constants
================================================================================
*/
static const float	ACTIVE_SCALE = 16.0f;
static const int	MAX_LENGTH = 4097;

/*
================================================================================
internal data structure
================================================================================
*/
enum triangulation_mode_t {
	TM_NW_SE,
	TM_SW_NE,
};

enum node_state_t {
	NS_ACTIVE,
	NS_BOUNDARY
};

struct vert_node_s {
	struct {
		uint32_t				mActiveFrame : 24;
		uint32_t				mState : 1;
		uint32_t				mLevel : 4;
		uint32_t				mAdjcentQuadsCount : 3;
	};
	const vec3 *				mOriginalPos;
	float						mInterpolationFactor;	// position interpolation
	vec3						mInterpolatedPos;
	vert_node_s *				mParent;
	vert_node_s *				mFirstChild;
	vert_node_s *				mNextSibling;
	struct quad_node_s *		mAdjcentQuads[4];

	void						AddChild(vert_node_s *child);
	void						AddAdjcentQuad(struct quad_node_s * quad_node);
	bool						HasChild(vert_node_s * test_node) const;
};

void vert_node_s::AddChild(vert_node_s *child) {
	if (child->mParent == this) {
		return; // avoid dual add
	}

	if (child->mLevel != mLevel + 1) {
		//ASSERT(false);
		return;
	}

	child->mParent = this;

	if (child->mNextSibling) {
		//ASSERT(false);
		return;
	}

	if (mFirstChild) {
		// head insert
		child->mNextSibling = mFirstChild;
		mFirstChild = child;
	}
	else {
		mFirstChild = child;
	}
}

void vert_node_s::AddAdjcentQuad(struct quad_node_s * quad_node) {
	//ASSERT(quadNode->mLevel == mLevel); //assure in the same level

	for (int i = 0; i < (int)mAdjcentQuadsCount; i++) {
		if (mAdjcentQuads[i] == quad_node) {
			//ASSERT(false);
			return;
		}
	}
	//ASSERT(mAdjcentQuadsCount < 4);
	mAdjcentQuads[mAdjcentQuadsCount++] = quad_node;
}

bool vert_node_s::HasChild(vert_node_s * test_node) const {
	vert_node_s * child = mFirstChild;
	while (child) {
		if (child == test_node) {
			return true;
		}
		child = child->mNextSibling;
	}
	return false;
}

struct quad_node_s {
	struct {
		uint32_t				mActiveFrame : 26;
		uint32_t				mState : 1;
		uint32_t				mLevel : 4;
		uint32_t				mTriangulationMode : 1;
	};
	quad_node_s *				mParent; // null if it is root
	vert_node_s *				mCornerVertNodes[4];
	vert_node_s *				mCenterVertNode;
	quad_node_s *				mChildren[4];

	bool						IsDiagonalConnected(const vert_node_s *vn1, const vert_node_s *vn2) const;
};

bool quad_node_s::IsDiagonalConnected(const vert_node_s *vn1, const vert_node_s *vn2) const {
	if (mTriangulationMode == TM_NW_SE)
	{
		return ((vn1 == mCornerVertNodes[1] && vn2 == mCornerVertNodes[3]))
			|| ((vn1 == mCornerVertNodes[3] && vn2 == mCornerVertNodes[1]));
	}
	else // TM_SW_NE
	{
		return ((vn1 == mCornerVertNodes[0] && vn2 == mCornerVertNodes[2]))
			|| ((vn1 == mCornerVertNodes[2] && vn2 == mCornerVertNodes[0]));
	}
}

struct quad_leaf_s {
	struct {
		uint32_t				mActiveFrame : 26;
		uint32_t				mState : 1;
		uint32_t				mLevel : 4;
		uint32_t				mTriangleMode : 1;
	};
	quad_node_s *				mParent;
	vert_node_s *				mCornerVertNodes[4];
};

QuadCollapseMesh::QuadCollapseMesh():
	mUpdateFrame(0),
	mOriginalPosRef(nullptr),
	mMaxLevel(0),
	mMaxLevelVerticesLength(0),
	mVertNodePool(nullptr),
	mQuadNodePool(nullptr),
	mQuadLeafPool(nullptr),
	mVertNodePoolSize(0),
	mQuadNodePoolSize(0),
	mQuadLeafPoolSize(0),
	mQuadNodePoolAllocated(0),
	mQuadLeafPoolAllocated(0),
	mRootQuadnode(nullptr)
{
	memset(mVertNodesActiveDistance, 0, sizeof(mVertNodesActiveDistance));
	memset(mQuadNodesCullRadius, 0, sizeof(mQuadNodesCullRadius));
	memset(mVertNodesLevelOffset, 0, sizeof(mVertNodesLevelOffset));
	memset(mRootVertnodes, 0, sizeof(mRootVertnodes));
}

QuadCollapseMesh::~QuadCollapseMesh() {
	if (mQuadLeafPool) {
		free(mQuadLeafPool);
		mQuadLeafPool = nullptr;
	}

	if (mQuadNodePool) {
		free(mQuadNodePool);
		mQuadNodePool = nullptr;
	}

	if (mVertNodePool) {
		free(mVertNodePool);
		mVertNodePool = nullptr;
	}
}

bool QuadCollapseMesh::Build(const vec3 *vertices, int width, int height) {
	mOriginalPosRef = vertices;

	// validate param
	if (width != height) {
		SYS_ERROR("height field must be squared\n");
		return false;
	}

	mMaxLevelVerticesLength = width;
	if (mMaxLevelVerticesLength < 2 || !IsPowerOf2(mMaxLevelVerticesLength - 1)) {
		SYS_ERROR("wrong length\n");
		return false;
	}

	if (mMaxLevelVerticesLength > MAX_LENGTH) {
		SYS_ERROR("height field too large\n");
		return false;
	}

	// check quad tree level
	int temp = mMaxLevelVerticesLength - 1;
	int level_count = 0;
	while (temp) {
		level_count++;
		temp >>= 1;
	}

	if (level_count > MAX_QUAD_LEVEL_COUNT) {
		SYS_ERROR("height map is too large\n");
		return false;
	}

	mMaxLevel = level_count - 1; // level range: 0 ~ mMaxLevel
	mVertNodePoolSize = 0;

	int quad_count_per_edge = 1;
	int vert_count_per_edge = 2;

	for (int i = 0; i < level_count; ++i) {
		float quad_size = (float)((mMaxLevelVerticesLength - 1) >> i);
		float quad_half_size = quad_size * 0.5f;
		float quad_node_cull_radius = sqrtf(quad_half_size * quad_half_size * 2.0f);

		mQuadNodesCullRadius[i] = quad_node_cull_radius;
		mVertNodesActiveDistance[i] = quad_node_cull_radius * ACTIVE_SCALE;

		if (i == level_count - 1) { // last level
			mQuadLeafPoolSize = quad_count_per_edge * quad_count_per_edge;
		}
		else {
			mQuadNodePoolSize += quad_count_per_edge * quad_count_per_edge;
		}

		mVertNodesLevelOffset[i] = mVertNodePoolSize;
		mVertNodePoolSize += (vert_count_per_edge * vert_count_per_edge);

		quad_count_per_edge <<= 1;
		vert_count_per_edge = vert_count_per_edge + vert_count_per_edge - 1;
	}

	// init memory pool
	mVertNodePool = (vert_node_s*)malloc(sizeof(vert_node_s) * mVertNodePoolSize);
	mQuadNodePool = (quad_node_s*)malloc(sizeof(quad_node_s) * mQuadNodePoolSize);
	mQuadLeafPool = (quad_leaf_s*)malloc(sizeof(quad_leaf_s) * mQuadLeafPoolSize);

	memset(mVertNodePool, 0, sizeof(vert_node_s) * mVertNodePoolSize);
	memset(mQuadNodePool, 0, sizeof(quad_node_s) * mQuadNodePoolSize);
	memset(mQuadLeafPool, 0, sizeof(quad_leaf_s) * mQuadLeafPoolSize);

	mQuadNodePoolAllocated = 0;
	mQuadLeafPoolAllocated = 0;

	BuildVertNodes();
	mRootQuadnode = RecursiveBuildQuadNodes(0, 0, 0, mMaxLevelVerticesLength - 1);

	return true;
}

void QuadCollapseMesh::BuildVertNodes() {
	uint32_t level = 0;
	int32_t step = mMaxLevelVerticesLength - 1;

	while (step > 0) {
		for (int32_t y = 0; y < mMaxLevelVerticesLength; y += step) {
			for (int32_t x = 0; x < mMaxLevelVerticesLength; x += step) {
				vert_node_s * vert_node = GetVertNode(level, x, y, true);

				vert_node->mActiveFrame = 0;
				vert_node->mState = 0;
				vert_node->mLevel = level;
				vert_node->mAdjcentQuadsCount = 0;
				vert_node->mOriginalPos = mOriginalPosRef + y * mMaxLevelVerticesLength + x;
				vert_node->mInterpolationFactor = 1.0f;
				vert_node->mInterpolatedPos = *vert_node->mOriginalPos;
				vert_node->mParent = nullptr;
				vert_node->mFirstChild = nullptr;
				vert_node->mNextSibling = nullptr;
				memset(vert_node->mAdjcentQuads, 0, sizeof(vert_node->mAdjcentQuads));
			}
		}

		step >>= 1;
		level++;
	}

	mRootVertnodes[0] = GetVertNode(0, 0, 0, false);
	mRootVertnodes[1] = GetVertNode(0, mMaxLevelVerticesLength - 1, 0, false);
	mRootVertnodes[2] = GetVertNode(0, mMaxLevelVerticesLength - 1, mMaxLevelVerticesLength - 1, false);
	mRootVertnodes[3] = GetVertNode(0, 0, mMaxLevelVerticesLength - 1, false);
}

quad_node_s * QuadCollapseMesh::RecursiveBuildQuadNodes(uint32_t level, int32_t x0, int32_t y0, int32_t step) {
	if (step == 1) { // leaf
		quad_leaf_s * result = AllocQuadLeaf();

		result->mActiveFrame = 0;
		result->mState = 0;
		result->mLevel = level;
		result->mTriangleMode = TM_NW_SE;
		result->mParent = nullptr;
		result->mCornerVertNodes[0] = GetVertNode(level, x0, y0, false);
		result->mCornerVertNodes[1] = GetVertNode(level, x0 + step, y0, false);
		result->mCornerVertNodes[2] = GetVertNode(level, x0 + step, y0 + step, false);
		result->mCornerVertNodes[3] = GetVertNode(level, x0, y0 + step, false);

		for (int32_t i = 0; i < 4; ++i) {
			vert_node_s * corner_vert_nodes = result->mCornerVertNodes[i];
			if (corner_vert_nodes->mLevel == level) {
				corner_vert_nodes->AddAdjcentQuad((quad_node_s*)result);
			}
		}

		return (quad_node_s*)result;
	}
	else {
		quad_node_s * result = AllocQuadNode();

		result->mActiveFrame = 0;
		result->mState = 0;
		result->mLevel = level;
		result->mParent = nullptr;

		int half_step = step >> 1;
		result->mCenterVertNode = GetVertNode(level + 1, x0 + half_step, y0 + half_step, false);
		result->mCornerVertNodes[0] = GetVertNode(level, x0, y0, false);
		result->mCornerVertNodes[1] = GetVertNode(level, x0 + step, y0, false);
		result->mCornerVertNodes[2] = GetVertNode(level, x0 + step, y0 + step, false);
		result->mCornerVertNodes[3] = GetVertNode(level, x0, y0 + step, false);

		for (int i = 0; i < 4; ++i) {
			vert_node_s * corner_vert_nodes = result->mCornerVertNodes[i];
			corner_vert_nodes->AddAdjcentQuad(result);
		}

		uint32_t next_level = level + 1;

		result->mChildren[0] = RecursiveBuildQuadNodes(next_level, x0, y0, half_step);
		result->mChildren[1] = RecursiveBuildQuadNodes(next_level, x0 + half_step, y0, half_step);
		result->mChildren[2] = RecursiveBuildQuadNodes(next_level, x0 + half_step, y0 + half_step, half_step);
		result->mChildren[3] = RecursiveBuildQuadNodes(next_level, x0, y0 + half_step, half_step);

		for (int i = 0; i < 4; ++i) {
			result->mChildren[i]->mParent = result;
		}

		CollapseQuad(result, x0, y0, step);

		return result;
	}
}

void QuadCollapseMesh::CollapseQuad(quad_node_s *quad_node, int32_t x0, int32_t y0, int32_t step) {
	vert_node_s * vert_node_child_bt = quad_node->mChildren[0]->mCornerVertNodes[1];
	vert_node_s * vert_node_child_rt = quad_node->mChildren[1]->mCornerVertNodes[2];
	vert_node_s * vert_node_child_tp = quad_node->mChildren[2]->mCornerVertNodes[3];
	vert_node_s * vert_node_child_lf = quad_node->mChildren[3]->mCornerVertNodes[0];
	vert_node_s * vert_node_child_ct = quad_node->mChildren[0]->mCornerVertNodes[2];

	vert_node_s * vert_node_child_sw = quad_node->mChildren[0]->mCornerVertNodes[0];
	vert_node_s * vert_node_child_se = quad_node->mChildren[1]->mCornerVertNodes[1];
	vert_node_s * vert_node_child_ne = quad_node->mChildren[2]->mCornerVertNodes[2];
	vert_node_s * vert_node_child_nw = quad_node->mChildren[3]->mCornerVertNodes[3];

	vert_node_s * vert_node_parent_sw = quad_node->mCornerVertNodes[0];
	vert_node_s * vert_node_parent_se = quad_node->mCornerVertNodes[1];
	vert_node_s * vert_node_parent_ne = quad_node->mCornerVertNodes[2];
	vert_node_s * vert_node_parent_nw = quad_node->mCornerVertNodes[3];

	vert_node_parent_sw->AddChild(vert_node_child_sw);
	vert_node_parent_se->AddChild(vert_node_child_se);
	vert_node_parent_ne->AddChild(vert_node_child_ne);
	vert_node_parent_nw->AddChild(vert_node_child_nw);

	if (vert_node_child_ct->mParent) {
		SYS_ERROR("vert_node_child_ct->mParent is not null\n");
		return;
	}

	if (!vert_node_child_bt->mParent) {
		vec3 delta1 = *vert_node_child_bt->mOriginalPos - *vert_node_parent_se->mOriginalPos;
		vec3 delta2 = *vert_node_child_bt->mOriginalPos - *vert_node_parent_sw->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_se->AddChild(vert_node_child_bt);
		}
		else {
			vert_node_parent_sw->AddChild(vert_node_child_bt);
		}
	}

	if (!vert_node_child_rt->mParent) {
		vec3 delta1 = *vert_node_child_rt->mOriginalPos - *vert_node_parent_ne->mOriginalPos;
		vec3 delta2 = *vert_node_child_rt->mOriginalPos - *vert_node_parent_se->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_ne->AddChild(vert_node_child_rt);
		}
		else {
			vert_node_parent_se->AddChild(vert_node_child_rt);
		}
	}

	if (!vert_node_child_tp->mParent) {
		vec3 delta1 = *vert_node_child_tp->mOriginalPos - *vert_node_parent_ne->mOriginalPos;
		vec3 delta2 = *vert_node_child_tp->mOriginalPos - *vert_node_parent_nw->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_ne->AddChild(vert_node_child_tp);
		}
		else {
			vert_node_parent_nw->AddChild(vert_node_child_tp);
		}
	}

	if (!vert_node_child_lf->mParent) {
		vec3 delta1 = *vert_node_child_lf->mOriginalPos - *vert_node_parent_nw->mOriginalPos;
		vec3 delta2 = *vert_node_child_lf->mOriginalPos - *vert_node_parent_sw->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_nw->AddChild(vert_node_child_lf);
		}
		else {
			vert_node_parent_sw->AddChild(vert_node_child_lf);
		}
	}

	if (   vert_node_parent_sw->HasChild(vert_node_child_lf)
		&& vert_node_parent_sw->HasChild(vert_node_child_bt)
		&& vert_node_parent_ne->HasChild(vert_node_child_rt)
		&& vert_node_parent_ne->HasChild(vert_node_child_tp))
	{
		vec3 delta1 = *vert_node_child_ct->mOriginalPos - *vert_node_parent_sw->mOriginalPos;
		vec3 delta2 = *vert_node_child_ct->mOriginalPos - *vert_node_parent_ne->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_sw->AddChild(vert_node_child_ct);
		}
		else {
			vert_node_parent_ne->AddChild(vert_node_child_ct);
		}
	}
	else if (
		   vert_node_parent_se->HasChild(vert_node_child_bt)
		&& vert_node_parent_se->HasChild(vert_node_child_rt)
		&& vert_node_parent_nw->HasChild(vert_node_child_tp)
		&& vert_node_parent_nw->HasChild(vert_node_child_lf))
	{
		vec3 delta1 = *vert_node_child_ct->mOriginalPos - *vert_node_parent_se->mOriginalPos;
		vec3 delta2 = *vert_node_child_ct->mOriginalPos - *vert_node_parent_nw->mOriginalPos;

		if (length(delta1) < length(delta2)) {
			vert_node_parent_se->AddChild(vert_node_child_ct);
		}
		else {
			vert_node_parent_nw->AddChild(vert_node_child_ct);
		}
	}
	else if (
		   vert_node_parent_sw->HasChild(vert_node_child_lf)
		&& vert_node_parent_sw->HasChild(vert_node_child_bt)) 
	{
		vert_node_parent_sw->AddChild(vert_node_child_ct);
	}
	else if (
		   vert_node_parent_se->HasChild(vert_node_child_bt) 
		&& vert_node_parent_se->HasChild(vert_node_child_rt)) 
	{
		vert_node_parent_se->AddChild(vert_node_child_ct);
	}
	else if (
		   vert_node_parent_ne->HasChild(vert_node_child_rt) 
		&& vert_node_parent_ne->HasChild(vert_node_child_tp)) 
	{
		vert_node_parent_ne->AddChild(vert_node_child_ct);
	}
	else if (
		   vert_node_parent_nw->HasChild(vert_node_child_tp)
		&& vert_node_parent_nw->HasChild(vert_node_child_lf)) 
	{
		vert_node_parent_nw->AddChild(vert_node_child_ct);
	}
	else {
		if (quad_node->mChildren[0]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_sw)) {
			vert_node_parent_sw->AddChild(vert_node_child_ct);
		}
		else if (quad_node->mChildren[1]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_se)) {
			vert_node_parent_se->AddChild(vert_node_child_ct);
		}
		else if (quad_node->mChildren[2]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_ne)) {
			vert_node_parent_ne->AddChild(vert_node_child_ct);
		}
		else if (quad_node->mChildren[3]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_nw)) {
			vert_node_parent_nw->AddChild(vert_node_child_ct);
		}
		else {
			float dist[4];

			vert_node_s * parents[4] = { vert_node_parent_sw, vert_node_parent_se, vert_node_parent_ne, vert_node_parent_nw };

			for (int i = 0; i < 4; ++i) {
				vec3 delta = *vert_node_child_ct->mOriginalPos - *parents[i]->mOriginalPos;
				dist[i] = length(delta);
			}

			float min_dist = dist[0];
			int min_parent_idx = 0;
			for (int32_t i = 1; i < 4; ++i) {
				if (dist[i] < min_dist) {
					min_parent_idx = i;
					min_dist = dist[i];
				}
			}

			parents[min_parent_idx]->AddChild(vert_node_child_ct);
		}
	}

	// determine triangle layout of current quadnode

	if (vert_node_child_ct->mParent == vert_node_parent_sw) {
		if (vert_node_parent_ne->HasChild(vert_node_child_tp) || vert_node_parent_ne->HasChild(vert_node_child_rt)) {
			quad_node->mTriangulationMode = TM_SW_NE;
		}
		else {
			if (quad_node->mChildren[2]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_ne)) {
				quad_node->mTriangulationMode = TM_SW_NE;
			}
			else {
				quad_node->mTriangulationMode = TM_NW_SE;
			}
		}
	}
	else if (vert_node_child_ct->mParent == vert_node_parent_se) {
		if (vert_node_parent_nw->HasChild(vert_node_child_tp) || vert_node_parent_nw->HasChild(vert_node_child_lf)) {
			quad_node->mTriangulationMode = TM_NW_SE;
		}
		else {
			if (quad_node->mChildren[3]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_nw)) {
				quad_node->mTriangulationMode = TM_NW_SE;
			}
			else {
				quad_node->mTriangulationMode = TM_SW_NE;
			}
		}
	}
	else if (vert_node_child_ct->mParent == vert_node_parent_ne) {
		if (vert_node_parent_sw->HasChild(vert_node_child_bt) || vert_node_parent_sw->HasChild(vert_node_child_lf)) {
			quad_node->mTriangulationMode = TM_SW_NE;
		}
		else {
			if (quad_node->mChildren[0]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_sw)) {
				quad_node->mTriangulationMode = TM_SW_NE;
			}
			else {
				quad_node->mTriangulationMode = TM_NW_SE;
			}
		}
	}
	else // vert_node_child_ct->parent == vertNodeParentNW
	{
		if (vert_node_parent_se->HasChild(vert_node_child_bt) || vert_node_parent_se->HasChild(vert_node_child_rt)) {
			quad_node->mTriangulationMode = TM_NW_SE;
		}
		else {
			if (quad_node->mChildren[1]->IsDiagonalConnected(vert_node_child_ct, vert_node_child_se)) {
				quad_node->mTriangulationMode = TM_NW_SE;
			}
			else {
				quad_node->mTriangulationMode = TM_SW_NE;
			}
		}
	}
}

void QuadCollapseMesh::Update(const camera_s &cam, const frustum_plane_s &fp) {
	mUpdateFrame++;

	for (int i = 0; i < 4; ++i) {
		mRootVertnodes[i]->mInterpolatedPos = *(mRootVertnodes[i]->mOriginalPos);
		RecursiveUpdateVertNode(cam.mPos, fp, mRootVertnodes[i]);
	}

	mActiveVertices.Reset();
	RecursiveSetActiveMesh(mRootQuadnode);
	mActiveMesh.mVertices = mActiveVertices.GetItems();
	mActiveMesh.mNumTriangles = mActiveVertices.GetCount() / 3;
}

int QuadCollapseMesh::GetMaxLevelVerticesLength() const {
	return mMaxLevelVerticesLength;
}

const triangle_mesh_s & QuadCollapseMesh::GetActiveMesh() const {
	return mActiveMesh;
}

vert_node_s * QuadCollapseMesh::GetVertNode(uint32_t level, int32_t x, int32_t y, bool init_mode) {
	vert_node_s * level_vert_node = mVertNodePool + mVertNodesLevelOffset[level];

	uint32_t shift = mMaxLevel - level;
	uint32_t row_step = (1 << level) + 1;

	x >>= shift;
	y >>= shift;

	vert_node_s * result = level_vert_node + y * row_step + x;

	if (!init_mode) { // do check
		if (result->mLevel != level) {
			SYS_ERROR("get vert node error\n");
		}
	}

	return result;
}

quad_node_s * QuadCollapseMesh::AllocQuadNode() {
	if (mQuadNodePoolAllocated >= mQuadNodePoolSize) {
		SYS_ERROR("quad node pool overflow\n");
		return nullptr;
	}
	else {
		return mQuadNodePool + mQuadNodePoolAllocated++;
	}
}

quad_leaf_s * QuadCollapseMesh::AllocQuadLeaf() {
	if (mQuadLeafPoolAllocated >= mQuadLeafPoolSize) {
		SYS_ERROR("quad leaf pool overflow\n");
		return nullptr;
	}
	else {
		return mQuadLeafPool + mQuadLeafPoolAllocated++;
	}
}

void QuadCollapseMesh::RecursiveUpdateVertNode(const vec3 &view_pos, const frustum_plane_s &fp, vert_node_s * vert_node) {
	if (vert_node->mActiveFrame == mUpdateFrame) {
		return;
	}

	vert_node->mActiveFrame = mUpdateFrame;
	vert_node->mState = NS_BOUNDARY;

	vec3 delta = view_pos - *vert_node->mOriginalPos;
	float dist = length(delta);

	if (dist < mVertNodesActiveDistance[vert_node->mLevel]) {
		if (vert_node->mFirstChild) {
			vert_node->mState = NS_ACTIVE;

			vert_node_s * child = vert_node->mFirstChild;
			while (child) {
				if (child->mParent != vert_node) {
					SYS_ERROR("child->mParent != vert_node\n");
				}
				RecursiveUpdateVertNode(view_pos, fp, child);
				child = child->mNextSibling;
			}
		}
		else {
			vert_node->mInterpolatedPos = *vert_node->mOriginalPos;
			for (int32_t i = 0; i < (int32_t)vert_node->mAdjcentQuadsCount; ++i) {
				QuadNodeSetBoundary(fp, vert_node->mAdjcentQuads[i]);
			}
		}
	}
	else {
		if (vert_node->mParent) {
			// interpolate

			// http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

			// we need find interpolation factor t
			// a ray from *(vertNode->mOriginalPos) to mCamera->mViewPos, intersection 
			// vertNode->mParent active sphere
			// d is the distance from *(vertNode->mOriginalPos) to vertNode->mParent active sphere surface
			// dist is distance from mCamera->mViewPos to *(vertNode->mOriginalPos)
			// when dist is d, t is 0, dist is mVertNodesActiveDistance[vertNode->mLevel] t is  1.

			vert_node_s * p = vert_node->mParent;

			vec3 o_minus_c = *vert_node->mOriginalPos - *p->mOriginalPos;

			vec3 l = view_pos - *vert_node->mOriginalPos;
			l = normalize(l);

			float l_dot_o_minus_c = dot(l, o_minus_c);

			float r = mVertNodesActiveDistance[vert_node->mLevel - 1];

			float sqr_length = dot(o_minus_c, o_minus_c);
			float temp = (l_dot_o_minus_c * l_dot_o_minus_c) - sqr_length + r * r;

			float d = -l_dot_o_minus_c + sqrtf(temp);
			float t = (d - dist) / (d - mVertNodesActiveDistance[vert_node->mLevel]);

			vec3 p_origin = *p->mOriginalPos;
			vec3 c_origin = *vert_node->mOriginalPos;

			vert_node->mInterpolatedPos[0] = p_origin[0] + t * (c_origin[0] - p_origin[0]);
			vert_node->mInterpolatedPos[1] = p_origin[1] + t * (c_origin[1] - p_origin[1]);
			vert_node->mInterpolatedPos[2] = p_origin[2] + t * (c_origin[2] - p_origin[2]);

			for (uint32_t i = 0; i < vert_node->mAdjcentQuadsCount; ++i) {
				QuadNodeSetBoundary(fp, vert_node->mAdjcentQuads[i]);
			}
		}
		else {
			vert_node->mInterpolatedPos = *vert_node->mOriginalPos;
			for (uint32_t i = 0; i < vert_node->mAdjcentQuadsCount; ++i) {
				QuadNodeSetBoundary(fp, vert_node->mAdjcentQuads[i]);
			}
		}
	}
}

void QuadCollapseMesh::QuadNodeSetBoundary(const frustum_plane_s &fp, quad_node_s *quad_node) {
	if (quad_node->mActiveFrame == mUpdateFrame && quad_node->mState == NS_ACTIVE) {
		return; // already set
	}

	if (quad_node->mLevel < mMaxLevel && fp.CullHorizontalCircle(*(quad_node->mCenterVertNode->mOriginalPos), mQuadNodesCullRadius[quad_node->mLevel])) {
		return; // culled away
	}

	quad_node->mActiveFrame = mUpdateFrame;
	quad_node->mState = NS_BOUNDARY;

	quad_node_s * p = quad_node->mParent;
	while (p) {
		if (p->mActiveFrame == mUpdateFrame && p->mState == NS_ACTIVE) {
			break;
		}

		p->mActiveFrame = mUpdateFrame;
		p->mState = NS_ACTIVE;
		p = p->mParent;
	}
}

void QuadCollapseMesh::RecursiveSetActiveMesh(quad_node_s *quad_node) {
	if (!quad_node) {
		return;
	}

	if (quad_node->mActiveFrame == mUpdateFrame) {
		if (quad_node->mState == NS_BOUNDARY) {

			if (quad_node->mTriangulationMode == TM_SW_NE) {
				AddActiveVertNode(quad_node->mCornerVertNodes[0]);
				AddActiveVertNode(quad_node->mCornerVertNodes[1]);
				AddActiveVertNode(quad_node->mCornerVertNodes[2]);

				AddActiveVertNode(quad_node->mCornerVertNodes[0]);
				AddActiveVertNode(quad_node->mCornerVertNodes[2]);
				AddActiveVertNode(quad_node->mCornerVertNodes[3]);
			}
			else {
				AddActiveVertNode(quad_node->mCornerVertNodes[0]);
				AddActiveVertNode(quad_node->mCornerVertNodes[1]);
				AddActiveVertNode(quad_node->mCornerVertNodes[3]);

				AddActiveVertNode(quad_node->mCornerVertNodes[1]);
				AddActiveVertNode(quad_node->mCornerVertNodes[2]);
				AddActiveVertNode(quad_node->mCornerVertNodes[3]);
			}
		}
		else { // NS_ACTIVE
			for (int32_t i = 0; i < 4; ++i) {
				RecursiveSetActiveMesh(quad_node->mChildren[i]);
			}
		}
	}
	else { // not active, but a child of active quad
		if (quad_node->mTriangulationMode == TM_SW_NE) {
			AddActiveVertNode(quad_node->mCornerVertNodes[0]);
			AddActiveVertNode(quad_node->mCornerVertNodes[1]);
			AddActiveVertNode(quad_node->mCornerVertNodes[2]);

			AddActiveVertNode(quad_node->mCornerVertNodes[0]);
			AddActiveVertNode(quad_node->mCornerVertNodes[2]);
			AddActiveVertNode(quad_node->mCornerVertNodes[3]);
		}
		else {
			AddActiveVertNode(quad_node->mCornerVertNodes[0]);
			AddActiveVertNode(quad_node->mCornerVertNodes[1]);
			AddActiveVertNode(quad_node->mCornerVertNodes[3]);

			AddActiveVertNode(quad_node->mCornerVertNodes[1]);
			AddActiveVertNode(quad_node->mCornerVertNodes[2]);
			AddActiveVertNode(quad_node->mCornerVertNodes[3]);
		}
	}
}

void QuadCollapseMesh::AddActiveVertNode(vert_node_s * vert_node) {
	if (vert_node->mActiveFrame == mUpdateFrame) {
		mActiveVertices.Add(vert_node->mInterpolatedPos);
	}
	else {
		vec3 * pos = nullptr;
		vert_node_s * p = vert_node->mParent;

		while (p) {
			if (p->mActiveFrame == mUpdateFrame) {
				pos = &p->mInterpolatedPos;
				break;
			}
			p = p->mParent;
		}

		if (pos) {
			mActiveVertices.Add(*pos);
		}
		else {
			return;
		}
	}
}
