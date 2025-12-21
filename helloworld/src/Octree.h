#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

//forward decleration
class GameObject;
struct Frustum;

//bounding box struct
struct AABB {
    glm::vec3 min = glm::vec3(0.0f);
    glm::vec3 max = glm::vec3(0.0f);

    //check if box contains another AABB
    bool Contains(const AABB& other) const;
    //check if box intersects another AABB
    bool Intersects(const AABB& other) const;
    void Recalculate(const glm::vec3& p);
};

AABB GetGameObjectAABB(const std::shared_ptr<GameObject>& obj);

//octree node
class OctreeNode {
public:
    OctreeNode(const AABB& bounds, int level, int capacity);
    ~OctreeNode();

    bool Insert(const std::shared_ptr<GameObject>& obj);
    void Subdivide();
    void Clear();

    const AABB& GetBounds() const { return bounds; }
    int GetObjectCount() const { return (int)objects.size(); }

    //checks if node is visible for frustum cooling 
    void Query(const Frustum& frustum, std::vector<std::shared_ptr<GameObject>>& result);

private:
    AABB bounds;
    int level;
    int maxCapacity;
    bool isDivided;

    std::vector<std::shared_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<OctreeNode>> children;

    //determines who has the object's center point
    int GetChildIndex(const AABB& objBounds) const;
};

//main octree
class Octree {
public:
    Octree(const AABB& worldBounds, int capacity = 10, int maxLevels = 5);
    ~Octree();

    void Insert(const std::shared_ptr<GameObject>& obj);
    void Rebuild(const std::vector<std::shared_ptr<GameObject>>& allObjects);
    void Clear();
    void Query(const Frustum& frustum, std::vector<std::shared_ptr<GameObject>>& result);

private:
    std::unique_ptr<OctreeNode> root;
    AABB worldBounds;
    //how many times nodes can subdivide
    int maxLevels;
    //objects that can be in one node
    int capacity;
};

//check if a box is visible
bool AABBIsVisible(const AABB& aabb, const Frustum& frustum);