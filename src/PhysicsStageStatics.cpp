#include "CommonFunctions.h"
#include "PhysicsStageStatics.h"
#include "WallSegment_data.h"
#include "WallCorner_data.h"
#include "PillarsTerrain_data.h"
#include "Tree_data.h"
#include "ObjectMotionState.h"
#include "ObjectInstance.h"
#include "MatrixOperations.h"

// Y coordinate of the "floor"
//static const GLfloat FloorY = 0.0;

static const unsigned int NumTreeLeavesPolygons = 236;
static const unsigned int NumTreeTrunkPolygons = 261;

PhysicsStageStatics::PhysicsStageStatics()
    : m_wallSegmentVertexBuffer(0),
      m_wallCornerVertexBuffer(0),
      m_terrainVertexBuffer(0),   
      m_terrainIndexBuffer(0),
      m_treeVertexBuffer(0),
      m_treeLeavesIndexBuffer(0),
      m_treeTrunkIndexBuffer(0),
      m_walkwayVertexBuffer(0),
      m_walkwayIndexBuffer(0),
      m_floorShape(NULL),
      m_wallSegmentShape(NULL),
      m_wallCornerShape(NULL)
{
}

PhysicsStageStatics::~PhysicsStageStatics()
{
    // Release all OpenGL resources
    glDeleteBuffers(1, &m_wallSegmentVertexBuffer);
    glDeleteBuffers(1, &m_wallCornerVertexBuffer);
    glDeleteBuffers(1, &m_terrainVertexBuffer);
    glDeleteBuffers(1, &m_terrainIndexBuffer);
    glDeleteBuffers(1, &m_treeVertexBuffer);
    glDeleteBuffers(1, &m_treeLeavesIndexBuffer);
    glDeleteBuffers(1, &m_treeTrunkIndexBuffer);
    glDeleteBuffers(1, &m_walkwayVertexBuffer);
    glDeleteBuffers(1, &m_walkwayIndexBuffer);

    delete m_floorShape;
    delete m_wallSegmentShape;
    delete m_wallCornerShape;
}

const float WalkwayY = 0.10;

const VertexAttribs WalkwayVertices[] = {
    // large area that hosts the pillars
    {-88.5, WalkwayY, -48.5,  0.0, 2.5,        0.0, 1.0, 0.0},
    {-88.5, WalkwayY, -20,    0.0, 0.0,        0.0, 1.0, 0.0},
    {-31.5, WalkwayY, -20,    5.0, 0.0,        0.0, 1.0, 0.0},
    {-31.5, WalkwayY, -48.5,   5.0, 2.5,        0.0, 1.0, 0.0},

    // pathlike walkway lined with trees
    {-64, WalkwayY, -20,   0.0, 10.0,        0.0, 1.0, 0.0},
    {-64, WalkwayY, 100,   0.0, 0.0,        0.0, 1.0, 0.0},
    {-56, WalkwayY, 100,   1.0, 0.0,        0.0, 1.0, 0.0},
    {-56, WalkwayY, -20,   1.0, 10.0,        0.0, 1.0, 0.0},
};

const GLushort WalkwayIndices[] = {
    0, 1, 3,
    3, 1, 2,
    4, 5, 7,
    7, 5, 6
};

const int WalkwayNumIndices = sizeof(WalkwayIndices) / sizeof(GLushort);

bool PhysicsStageStatics::Setup()
{
    // Create wall segment vertex buffer
    glGenBuffers(1, &m_wallSegmentVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_wallSegmentVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WallSegment_vertices),
                 WallSegment_vertices, GL_STATIC_DRAW);

    // Create wall corner vertex buffer
    glGenBuffers(1, &m_wallCornerVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_wallCornerVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WallCorner_vertices),
                 WallCorner_vertices, GL_STATIC_DRAW);

    // Create terrain vertex/index buffers
    glGenBuffers(1, &m_terrainVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_terrainVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PillarsTerrain_vertices),
                 PillarsTerrain_vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &m_terrainIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_terrainIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(PillarsTerrain_indices),
                 PillarsTerrain_indices, GL_STATIC_DRAW);
    
    // Create tree vertex/index buffers
    glGenBuffers(1, &m_treeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_treeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tree_vertices),
                 Tree_vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &m_treeLeavesIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeLeavesIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 NumTreeLeavesPolygons * 3 * sizeof(GLushort),
                 &Tree_indices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &m_treeTrunkIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeTrunkIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 NumTreeTrunkPolygons * 3 * sizeof(GLushort),
                 &Tree_indices[NumTreeLeavesPolygons * 3], GL_STATIC_DRAW);

    // Create walkway vertex/index buffers
    glGenBuffers(1, &m_walkwayVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_walkwayVertexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(WalkwayVertices),
                 WalkwayVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_walkwayIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_walkwayIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(WalkwayIndices),
                 WalkwayIndices, GL_STATIC_DRAW);
    
    return (glGetError() == GL_NO_ERROR);
}

PhysicsStageStatics* PhysicsStageStatics::Create()
{
    PhysicsStageStatics* statics = new PhysicsStageStatics();
    if ( !statics->Setup() )
    {
        LOG_DEBUG("Failed to setup PhysicsStageStatics.");
        delete statics;
        return NULL;
    }
    else
    {
        return statics;
    }
}

void PhysicsStageStatics::PrepareRenderTreeTrunkShadowMap()
{
    // Set up rendering coords (depth) only
    glBindBuffer(GL_ARRAY_BUFFER, m_treeVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeTrunkIndexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
}

void PhysicsStageStatics::PrepareRenderTreeLeavesShadowMap()
{
    // Set up rendering with texture coords for alpha detection
    glBindBuffer(GL_ARRAY_BUFFER, m_treeVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeLeavesIndexBuffer);
    glVertexAttribPointer(COORD_INDEX, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, x));
    glVertexAttribPointer(TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE,
                          sizeof(VertexAttribs),
                          (const GLvoid*)offsetof(VertexAttribs, u));
}

void PhysicsStageStatics::PrepareRenderTreeTrunk()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_treeVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeTrunkIndexBuffer);
    SetVertexAttribsPointers();
}

void PhysicsStageStatics::PrepareRenderTreeLeaves()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_treeVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_treeLeavesIndexBuffer);
    SetVertexAttribsPointers();
}

void PhysicsStageStatics::RenderTreeTrunk()
{
    glDrawElements(GL_TRIANGLES, NumTreeTrunkPolygons * 3, 
                   GL_UNSIGNED_SHORT, NULL);
}

void PhysicsStageStatics::RenderTreeLeaves()
{
    glDrawElements(GL_TRIANGLES, NumTreeLeavesPolygons * 3, 
                   GL_UNSIGNED_SHORT, NULL);
}

void PhysicsStageStatics::RenderWalkway()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_walkwayVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_walkwayIndexBuffer);
    SetVertexAttribsPointers();
    glDrawElements(GL_TRIANGLES, WalkwayNumIndices,
                   GL_UNSIGNED_SHORT, NULL);
}

void PhysicsStageStatics::RenderTerrain() 
{
    glBindBuffer(GL_ARRAY_BUFFER, m_terrainVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_terrainIndexBuffer);
    SetVertexAttribsPointers();
    glDrawElements(GL_TRIANGLES, PillarsTerrainNumIndices, 
                   GL_UNSIGNED_SHORT, NULL);
}

void PhysicsStageStatics::PrepareRenderWallSegment()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_wallSegmentVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    SetVertexAttribsPointers();
}

void PhysicsStageStatics::RenderWallSegment()
{
    glDrawArrays(GL_TRIANGLES, 0, WallSegmentNumVertices);
}

void PhysicsStageStatics::PrepareRenderWallCorner()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_wallCornerVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    SetVertexAttribsPointers();
}

void PhysicsStageStatics::RenderWallCorner()
{
    glDrawArrays(GL_TRIANGLES, 0, WallCornerNumVertices);
}

void PhysicsStageStatics::AddStaticBody(const btVector3& translation,
                                        const btQuaternion& rotation,
                                        btCollisionShape* shape,
                                        std::list<ObjectInstance*>& objects)
{
    ObjectMotionState* motionState =
            new ObjectMotionState(btTransform(rotation, translation),
                                  NULL);
    motionState->UpdateObjectTransform();

    // Body construction info: mass = 0, inertia = 0 vector for static shape
    // Objects with mass = 0 are static (immovable)
    btRigidBody::btRigidBodyConstructionInfo bodyCI(0, motionState, shape);
    objects.push_back(new ObjectInstance(new btRigidBody(bodyCI)));
}

void PhysicsStageStatics::AddWallSegment(float x, float y, float z,
                                         const btQuaternion& rotation,
                                         std::list<ObjectInstance*>& objects)
{
    if ( m_wallSegmentShape == NULL )
    {
        m_wallSegmentShape = new btBoxShape(btVector3(WallSegmentHalfWidth,
                                                      WallSegmentHalfHeight,
                                                      WallSegmentHalfDepth));
    }

    AddStaticBody(btVector3(x, y, z), rotation, m_wallSegmentShape, objects);
}

void PhysicsStageStatics::AddWallCorner(float x, float y, float z,
                                        std::list<ObjectInstance*>& wallCorners)
{
    if ( m_wallCornerShape == NULL )
    {
        m_wallCornerShape = new btBoxShape(btVector3(WallCornerHalfWidth,
                                                     WallCornerHalfHeight,
                                                     WallCornerHalfDepth));
    }

    AddStaticBody(btVector3(x, y, z), btQuaternion(0, 0, 0),
                  m_wallCornerShape, wallCorners);
}

void PhysicsStageStatics::AddTree(float x, float y, float z, float yRotAngle,
                                  std::list<ObjectInstance*>& trees)
{
    float yRotation[16];
    MatrixCreateRotation(yRotation, yRotAngle, 0.0, 1.0, 0.0);
    float translation[16];
    MatrixCreateTranslation(translation, x, y, z);
    float transform[16];
    MatrixMultiply(yRotation, translation, transform);
    trees.push_back(new ObjectInstance(transform));
}

void PhysicsStageStatics::CreateObjects(std::list<ObjectInstance*>& terrains, 
                                        std::list<ObjectInstance*>& wallCorners,
                                        std::list<ObjectInstance*>& wallSegments,
                                        std::list<ObjectInstance*>& trees)
{
    // Create floor static body
    if ( m_floorShape == NULL )
    {
        m_floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    }

    // Create the collision object for ground
    AddStaticBody(btVector3(0, 0, 0), btQuaternion(0, 0, 0),
                  m_floorShape, terrains);
    
    // Create the wall corners
    AddWallCorner(-88, WallCornerHalfHeight, -48, wallCorners);
    AddWallCorner(-32, WallCornerHalfHeight, -48, wallCorners);
    AddWallCorner(-32, WallCornerHalfHeight, -22, wallCorners);
    AddWallCorner(-88, WallCornerHalfHeight, -22, wallCorners);
    AddWallCorner(-60, WallCornerHalfHeight, -48, wallCorners);

    // Create the wall segments
    AddWallSegment(-88, WallSegmentHalfHeight, -35,
                   btQuaternion(btVector3(0, 1, 0), M_PI / 2), wallSegments);
    AddWallSegment(-75, WallSegmentHalfHeight, -48,
                   btQuaternion(0, 0, 0), wallSegments);
    AddWallSegment(-45, WallSegmentHalfHeight, -48,
                   btQuaternion(0, 0, 0), wallSegments);
    AddWallSegment(-32, WallSegmentHalfHeight, -35,
                   btQuaternion(btVector3(0, 1, 0), M_PI / 2), wallSegments);
    
    // Create the trees
    AddTree(-66, TreeHalfHeight, -12, 0.2 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, -12, 0.0 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 0,   0.4 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 0,   0.7 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 12,  0.9 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 12,  0.1 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 24,  0.95 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 24,  0.35 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 36,  0.32 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 36,  0.11 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 48,  0.78 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 48,  0.17 * 2*M_PI, trees);
    AddTree(-66, TreeHalfHeight, 60,  0.06 * 2*M_PI, trees);
    AddTree(-54, TreeHalfHeight, 60,  0.55 * 2*M_PI, trees);
}

