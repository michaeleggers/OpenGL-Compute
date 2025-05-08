#include "g_tree.h"

#include <vector>

#include "utils.h"

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

static int g_VerticesGenerated = 0;

void AddBranchRec(Branch               root,
                  float                angle,
                  float                branch_length,
                  std::vector<Branch>& branch_list,
                  int                  max_depth,
                  int                  parentID,
                  int                  current_depth)
{
    if ( max_depth == 0 )
    {
        return;
    }

    glm::vec3 root_dir = glm::normalize(root.end.pos - root.start.pos);

    float     treeDepthPct = (float)current_depth / (float)max_depth;
    glm::vec4 color        = CreateColorForDepthPct(treeDepthPct);

    Branch branch      = {};
    branch.start       = root.end;
    branch.start.color = color;
    branch.end         = branch.start;
    branch_length *= 0.7f;

    glm::vec3 rotation_axis             = glm::normalize(glm::vec3( // rotate around arbitrary axis
        rand_between(-1.0f, 1.0f),
        rand_between(-1.0f, 1.0f),
        rand_between(-1.0f, 1.0f)));
    glm::quat q                         = glm::angleAxis(glm::radians(angle), rotation_axis);
    glm::vec3 new_branch_dir            = glm::rotate(q, root_dir);
    branch.end.pos                      = branch.start.pos + branch_length * new_branch_dir;
    branch.end.color                    = color;
    branch.computeData.parentIndex      = parentID;
    branch.computeData.branchDir        = glm::vec4(branch_length * new_branch_dir, 1.0f);
    branch.computeData.vertexIndexStart = g_VerticesGenerated++;
    branch.computeData.vertexIndexEnd   = g_VerticesGenerated++;
    branch.computeData.depth            = current_depth;

    branch_list.push_back(branch);

    int newParentID = branch_list.size() - 1;

    AddBranchRec(branch, angle, branch_length, branch_list, max_depth - 1, newParentID, current_depth + 1);
    AddBranchRec(branch, -angle, branch_length, branch_list, max_depth - 1, newParentID, current_depth + 1);
}

std::vector<Branch> CreateTree(glm::vec3 root_start, glm::vec3 root_end, float branch_angle, int max_depth)
{

    float branch_length = glm::length(root_end - root_start);

    Vertex v0 = {};
    v0.pos    = glm::vec3(0.0f);
    v0.color  = CreateColorForDepthPct(0.0f);
    Vertex v1 = {};
    v1.pos    = glm::vec3(0.0f, branch_length, 0.0f);
    v1.color  = CreateColorForDepthPct(0.0f);

    Branch root                       = { v0, v1, BranchComputeData() };
    root.computeData.parentIndex      = -1;
    root.computeData.branchDir        = glm::vec4(v1.pos - v0.pos, 1.0f);
    root.computeData.vertexIndexStart = g_VerticesGenerated++;
    root.computeData.vertexIndexEnd   = g_VerticesGenerated++;
    root.computeData.depth            = 0;

    std::vector<Branch> branch_list = {};
    branch_list.push_back(root);
    AddBranchRec(root, branch_angle, branch_length, branch_list, max_depth, 0, 1);
    AddBranchRec(root, -branch_angle, branch_length, branch_list, max_depth, 0, 1);
    AddBranchRec(root, 30.0f, branch_length, branch_list, max_depth, 0, 1);
    AddBranchRec(root, 35.0f, branch_length, branch_list, max_depth, 0, 1);

    // Reset the UID generator. Important if a new tree shall be created!
    g_VerticesGenerated = 0;

    return branch_list;
}

glm::vec4 CreateColorForDepthPct(float treeDepthPct)
{
    float amountRed = -1.5f * glm::log(glm::pow(treeDepthPct, 6) + 1.0f) + 1;

    return glm::vec4(0.75 * amountRed, 1.0f - 0.6 * amountRed, 0.5 * amountRed / 4.0f, 1.0f);
}
