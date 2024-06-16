#include "g_tree.h"

#include <vector>

#include "utils.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>


void AddBranchRec(Branch root, float angle, float branch_length, std::vector<Branch>& branch_list, int max_depth, int parentID) {
	if (max_depth == 0) {
		return;
	}

	glm::vec3 root_dir = glm::normalize(root.end.pos - root.start.pos);

	Branch branch = {};
	branch.start = root.end;
	branch.end = branch.start;
	branch_length *= 0.7f;

	glm::vec3 rotation_axis = glm::normalize(glm::vec3(
		rand_between(-1.0f, 1.0f),
		rand_between(-1.0f, 1.0f),
		rand_between(-1.0f, 1.0f)
		//0.0f, 0.0f, 1.0f
	));
	glm::quat q = glm::angleAxis(glm::radians(angle), rotation_axis); // rotate around arbitrary axis
	glm::vec3 new_branch_dir = glm::rotate(q, root_dir);
	branch.end.pos = branch.start.pos + branch_length * new_branch_dir;
	branch.computeData.orientation = { q.x, q.y, q.z, q.w };
	branch.computeData.parentIndex = parentID;

	branch_list.push_back(branch);

	AddBranchRec(branch, angle, branch_length, branch_list, max_depth - 1, parentID + 1);
	AddBranchRec(branch, -angle, branch_length, branch_list, max_depth - 1, parentID + 1);
}

std::vector<Branch> CreateTree(glm::vec3 root_start, glm::vec3 root_end, float branch_angle, int max_depth) {	

	float branch_length = glm::length(root_end - root_start);

	Vertex v0 = {};
	v0.pos = glm::vec3(0.0f);
	v0.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Vertex v1 = {};
	v1.pos = glm::vec3(0.0f, branch_length, 0.0f);
	v1.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	Branch root = { v0, v1 };
	glm::quat q = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	root.computeData.orientation= { q.x, q.y, q.z, q.w };
	root.computeData.parentIndex = -1;

	std::vector<Branch> branch_list = {};
	branch_list.push_back(root);
	AddBranchRec(root, branch_angle, branch_length, branch_list, max_depth, -1);
	AddBranchRec(root, -branch_angle, branch_length, branch_list, max_depth, -1);
	AddBranchRec(root, 30.0f, branch_length, branch_list, max_depth, -1);
	AddBranchRec(root, 35.0f, branch_length, branch_list, max_depth, -1);

	return branch_list;
}
