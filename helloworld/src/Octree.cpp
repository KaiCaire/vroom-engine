#include "Octree.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h" 
#include "ResourceMesh.h"        
#include "Log.h"

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <limits> 

//world bounding box calculation
AABB GetGameObjectAABB(const std::shared_ptr<GameObject>& obj) {
	auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(obj->GetComponent(ComponentType::MESH_RENDERER));
	auto transformComp = std::dynamic_pointer_cast<TransformComponent>(obj->GetComponent(ComponentType::TRANSFORM));

	//check if object has mesh and transform
    if (!renderComp || !transformComp) return {};
    std::shared_ptr<ResourceMesh> mesh = renderComp->GetMesh();
    if (!mesh) return {};

	//get world transform and local bounding box
    glm::mat4 worldMatrix = transformComp->GetGlobalTransform();
    glm::vec3 localMin = mesh->minAABB;
    glm::vec3 localMax = mesh->maxAABB;

    //define the corners
    std::vector<glm::vec3> localCorners = {
        glm::vec3(localMin.x, localMin.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMin.z),
        glm::vec3(localMin.x, localMax.y, localMin.z),
        glm::vec3(localMin.x, localMin.y, localMax.z),

        glm::vec3(localMax.x, localMax.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMax.z),
        glm::vec3(localMin.x, localMax.y, localMax.z),
        glm::vec3(localMax.x, localMax.y, localMax.z)
    };

    //find world extremes
    glm::vec3 worldMin(std::numeric_limits<float>::max());
    glm::vec3 worldMax(std::numeric_limits<float>::lowest());

    for (const auto& corner : localCorners) {
        //transform the corner into world space
        glm::vec4 worldCorner = worldMatrix * glm::vec4(corner, 1.0f);

        glm::vec3 p = glm::vec3(worldCorner);

        //expand the world box 
        worldMin = glm::min(worldMin, p);
        worldMax = glm::max(worldMax, p);
    }

    return AABB{ worldMin, worldMax };
}

//bounding box functions
bool AABB::Contains(const AABB& other) const {
    return (other.min.x >= min.x && other.max.x <= max.x &&
        other.min.y >= min.y && other.max.y <= max.y &&
        other.min.z >= min.z && other.max.z <= max.z);
}

bool AABB::Intersects(const AABB& other) const {
    return (max.x > other.min.x && min.x < other.max.x &&
        max.y > other.min.y && min.y < other.max.y &&
        max.z > other.min.z && min.z < other.max.z);
}

void AABB::Recalculate(const glm::vec3& p) {
    min.x = glm::min(min.x, p.x);
    min.y = glm::min(min.y, p.y);
    min.z = glm::min(min.z, p.z);

    max.x = glm::max(max.x, p.x);
    max.y = glm::max(max.y, p.y);
    max.z = glm::max(max.z, p.z);
}

//node functions
OctreeNode::OctreeNode(const AABB& bounds, int level, int capacity)
    : bounds(bounds), level(level), maxCapacity(capacity), isDivided(false) {
}

OctreeNode::~OctreeNode() {
    Clear();
}

void OctreeNode::Clear() {
    objects.clear();
    children.clear();
    isDivided = false;
}

int OctreeNode::GetChildIndex(const AABB& objBounds) const {
    int index = 0;
    glm::vec3 center = (bounds.min + bounds.max) * 0.5f;
    glm::vec3 objCenter = (objBounds.min + objBounds.max) * 0.5f;

    if (objCenter.x >= center.x) index += 1; // 1, 3, 5, 7 (+X)
    if (objCenter.y >= center.y) index += 2; // 2, 3, 6, 7 (+Y)
    if (objCenter.z >= center.z) index += 4; // 4, 5, 6, 7 (+Z)

    return index;
}

void OctreeNode::Subdivide() {
    if (isDivided) return;

    glm::vec3 min = bounds.min;
    glm::vec3 max = bounds.max;
    glm::vec3 half = (min + max) * 0.5f;

    // Create bounds for the 8 children
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ min, half }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ glm::vec3(half.x, min.y, min.z), glm::vec3(max.x, half.y, half.z) }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ glm::vec3(min.x, half.y, min.z), glm::vec3(half.x, max.y, half.z) }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ glm::vec3(half.x, half.y, min.z), glm::vec3(max.x, max.y, half.z) }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ glm::vec3(min.x, min.y, half.z), glm::vec3(half.x, half.y, max.z) }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ glm::vec3(min.x, half.y, half.z), glm::vec3(half.x, max.y, max.z) }, level + 1, maxCapacity));
    children.emplace_back(std::make_unique<OctreeNode>(AABB{ half, max }, level + 1, maxCapacity));

    isDivided = true;

    //reinsert existing objects
    std::vector<std::shared_ptr<GameObject>> objectsToReinsert;
    objectsToReinsert.swap(objects);

    for (const auto& obj : objectsToReinsert) {
        AABB objBounds = GetGameObjectAABB(obj);

        //try inserting into the appropriate child
        int index = GetChildIndex(objBounds);

        if (children[index]->GetBounds().Contains(objBounds)) {
            children[index]->Insert(obj);
        }
        else {
            objects.push_back(obj);
        }
    }
}

bool OctreeNode::Insert(const std::shared_ptr<GameObject>& obj) {
    //only insert if object is active and renderable
    if (!obj->IsActive() || !obj->GetComponent(ComponentType::TRANSFORM) || !obj->GetComponent(ComponentType::MESH_RENDERER)) {
        return false;
    }

    AABB objBounds = GetGameObjectAABB(obj);
    if (!bounds.Contains(objBounds)) {
        return false; //object is outside this node bounds
    }

    //node is not divided and has space
    if (!isDivided && objects.size() < maxCapacity) {
        objects.push_back(obj);
        return true;
    }

    //node is not divided and reached capacity -> subdivide
    if (!isDivided && level < 5) { 
        Subdivide();
    }

    //node is divided or max depth reached
    if (isDivided) {
        //try inserting into a child node
        int index = GetChildIndex(objBounds);

        //check if the child node can fully contain the AABB
        if (children[index]->GetBounds().Contains(objBounds)) {
            if (children[index]->Insert(obj)) {
                return true;
            }
        }
    }

    //keep the object in the current node
    objects.push_back(obj);
    return true;
}

void OctreeNode::Query(const AABB& frustum, std::vector<std::shared_ptr<GameObject>>& result) {
    if (!bounds.Intersects(frustum)) {
        return; //does not intersect the frustum/query box
    }

    //add objects stored in this node
    result.insert(result.end(), objects.begin(), objects.end());

    //recurse into children
    if (isDivided) {
        for (const auto& child : children) {
            child->Query(frustum, result);
        }
    }
}

//octree functions
Octree::Octree(const AABB& worldBounds, int capacity, int maxLevels)
    : worldBounds(worldBounds), capacity(capacity), maxLevels(maxLevels) {
    root = std::make_unique<OctreeNode>(worldBounds, 0, capacity);
    LOG("Octree initialized with bounds (Min: %.2f, %.2f, %.2f, Max: %.2f, %.2f, %.2f)",
        worldBounds.min.x, worldBounds.min.y, worldBounds.min.z,
        worldBounds.max.x, worldBounds.max.y, worldBounds.max.z);
}

Octree::~Octree() {}

void Octree::Clear() {
    if (root) {
        root->Clear();
    }
    root = std::make_unique<OctreeNode>(worldBounds, 0, capacity);
    LOG("Octree cleared.");
}

void Octree::Insert(const std::shared_ptr<GameObject>& obj) {
    if (root) {
        root->Insert(obj);
    }
}

void Octree::Rebuild(const std::vector<std::shared_ptr<GameObject>>& allObjects) {
    Clear();
    int count = 0;
    for (const auto& obj : allObjects) {
        if (root->Insert(obj)) {
            count++;
        }
    }
    LOG("Octree rebuilt, successfully inserted %d objects.", count);
}

void Octree::Query(const AABB& frustum, std::vector<std::shared_ptr<GameObject>>& result) {
    if (root) {
        root->Query(frustum, result);
    }
}