#ifndef PILLARSGROUND_H
#define PILLARSGROUND_H

#include <vector>
#include <list>

#include <btBulletDynamicsCommon.h>
#include "OpenGLAPI.h"

// Forward declarations
class ObjectInstance;

/**
 * Represents the ground & all static objects in the physics scene.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class PhysicsStageStatics
{
public: // Construction and destruction
    ~PhysicsStageStatics();
    static PhysicsStageStatics* Create();

public: // Public API
    void CreateObjects(std::list<ObjectInstance*>& terrains, 
                       std::list<ObjectInstance*>& wallCorners,
                       std::list<ObjectInstance*>& wallSegments,
                       std::list<ObjectInstance*>& trees);
    void PrepareRenderWallSegment();
    void RenderWallSegment();
    void PrepareRenderWallCorner();
    void RenderWallCorner();
    void RenderTerrain();
    void PrepareRenderTreeTrunkShadowMap();
    void PrepareRenderTreeLeavesShadowMap();
    void PrepareRenderTreeTrunk();
    void PrepareRenderTreeLeaves();
    void RenderTreeTrunk();
    void RenderTreeLeaves();
    void RenderWalkway();

private:
    bool Setup();
    PhysicsStageStatics();
    void AddStaticBody(const btVector3& translation,
                       const btQuaternion& rotation,
                       btCollisionShape* shape,
                       std::list<ObjectInstance*>& objects);
    void AddWallSegment(float x, float y, float z, const btQuaternion& rotation,
                        std::list<ObjectInstance*>& objects);
    void AddWallCorner(float x, float y, float z,
                       std::list<ObjectInstance*>& objects);
    void AddTree(float x, float y, float z, float yRotAngle,
                 std::list<ObjectInstance*>& objects);

private: // Data
    // Vertex/index buffers
    GLuint m_wallSegmentVertexBuffer;
    GLuint m_wallCornerVertexBuffer;
    GLuint m_terrainVertexBuffer;
    GLuint m_terrainIndexBuffer;
    GLuint m_treeVertexBuffer;
    GLuint m_treeLeavesIndexBuffer;
    GLuint m_treeTrunkIndexBuffer;
    GLuint m_walkwayVertexBuffer;
    GLuint m_walkwayIndexBuffer;

    // Collision shapes
    btStaticPlaneShape* m_floorShape;
    btBoxShape* m_wallSegmentShape;
    btBoxShape* m_wallCornerShape;
};

#endif // PILLARSGROUND_H
