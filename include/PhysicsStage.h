#ifndef PILLARSTAGE_H
#define PILLARSTAGE_H

#include <list>
#include <vector>

#include <btBulletDynamicsCommon.h>
#include "BaseStage.h"

// Forward declarations
class PhysicsStageStatics;
class Pillar;
class Skybox;
class BaseAnimation;
class RotationAnimation;
class ObjectInstance;
class Vehicle;
class SimpleTimer;

/**
 * Describes a lens flare image; a series of these will make up for the
 * whole effect. The texture is assumed to be such that the flare image(s)
 * take up the whole texture in vertical direction, ie. the v goes 0.0 .. 1.0
 */
struct LensFlare
{
    // Texture u on the left side of this flare
    float m_leftU;

    // Texture u on the right side of this flare
    float m_rightU;

    // Scalar distance from the screen center along the flare vector
    float m_distance;

    // Size modifier; final size of the flare is sizeScaler * maxsize
    float m_sizeScaler;
};

/**
 * Physics engine stage with realtime shadows.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class PhysicsStage : public BaseStage
{
public: // Construction and destruction
    PhysicsStage(TextRenderer& textRenderer,
                 GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                 GLuint simpleColorProgram,
                 GLint simpleColorMvpLoc, GLint simpleColorColorLoc);
    ~PhysicsStage();

protected: // From BaseStage
    bool SetupImpl();
    void RenderImpl(const TimeSample& time);
    void TeardownImpl();
    bool ViewportResized(int viewportWidth, int viewportHeight);
    void UpdateScore(const TimeSample& now);

private:
    void Animate(const TimeSample& time);
    void UpdateDefaultUniforms(const float* objectTransform);
    void PrepareRenderWall();
    void RenderPillarsDepthMap(float* vpMatrix);
    void RenderTreeTrunksDepthMap(float* vpMatrix);
    void RenderTreeLeavesDepthMap(float* vpMatrix);
    void RenderDepthMap();
    void RenderSkybox(float* viewMatrix);
    void PrepareRenderPillars();
    void RenderPillars(float* vpMatrix, GLint mvpLoc);
    void RenderTerrain(float* vMatrix, float* vpMatrix);
    void RenderWalkway(float* vpMatrix);
    void RenderTrees(float* vpMatrix);
    void RenderWallCorners(float* vpMatrix, GLint mvpLoc);
    void RenderWallSegments(float* vpMatrix, GLint mvpLoc);
    void SetupWheelRender(const float* wheelTransform,
                          const float* objectTransform,
                          const float* vpMatrix, GLint mvpLoc);
    void UpdateVehicleUniforms(const float* objectTransform);
    void RenderVehicle(float* vpMatrix, GLint mvpLoc);
    void DetectSunVisibility(float* vpMatrix);
    void RenderLensFlares();
    void StepPhysics();
    void CopyPhysicsTransforms();
    static void BulletTickCallback(btDynamicsWorld* world, btScalar timeStep);
    void SetupPhysicsEngine();
    void SetupLensFlares();
    static void TimerCallback(SimpleTimer* timer, int timerId);
    void AddTransAnim(float initialX, float initialY, float initialZ,
                      float destinationX, float destinationY,
                      float destinationZ,
                      float initialDelay, float duration, float* location);
    void AddTransAnim(const float* initialPtr,
                      float destinationX, float destinationY,
                      float destinationZ,
                      float initialDelay, float duration, float* location);
    void SetupAnimations();
    bool SetupShadowMapping();
    void CreatePillars();
    void CreateVehicle();
    void AddNewPillar(float x, float y, float z);
    void AddBodies(std::list<ObjectInstance*>& objects);
    void DestroyBodies(std::vector<btRigidBody*>& bodies);
    void DestroyObjects(std::list<ObjectInstance*>& objects);

private: // Data
    // Shader programs
    GLuint m_defaultProgram;
    GLuint m_skyboxProgram;
    GLuint m_terrainProgram;
    GLuint m_shadowMapProgram;
    GLuint m_shadowMapTransparentProgram;
    GLuint m_vehicleProgram;

    // Textures
    GLuint m_pillarTexture;
    GLuint m_wallSegmentTexture;
    GLuint m_skyboxTexture;
    GLuint m_terrainTexture;
    GLuint m_terrainTexture2;
    GLuint m_treeBarkTexture;
    GLuint m_treeLeavesTexture;
    GLuint m_lensFlareTexture;
    GLuint m_walkwayTexture;
    GLuint m_buggyBlueTexture;
    GLuint m_buggyNormalmap;

    // Uniform locations
    GLint m_defaultMvpLoc;
    GLint m_defaultTextureLoc;
    GLint m_defaultLightPosLoc;
    GLint m_defaultShadowTextureLoc;
    GLint m_defaultShadowMatrixLoc;
    GLint m_skyboxMvpLoc;
    GLint m_skyboxTextureLoc;
    GLint m_terrainMvpLoc;
    GLint m_terrainMvLoc;
    GLint m_terrainTextureLoc;
    GLint m_terrainTexture2Loc;
    GLint m_terrainShadowTextureLoc;
    GLint m_terrainShadowMatrixLoc;
    GLint m_shadowMapMvpLoc;
    GLint m_shadowMapTransparentMvpLoc;
    GLint m_shadowMapTransparentTextureLoc;
    GLint m_vehicleMvpLoc;
    GLint m_vehicleTextureLoc;
    GLint m_vehicleNormalmapLoc;
    GLint m_vehicleLightPosLoc;
    GLint m_vehicleEyePosLoc;
    GLint m_vehicleShadowTextureLoc;
    GLint m_vehicleShadowMatrixLoc;
    GLint m_vehicleShininessLoc;
    GLint m_vehicleSpecularColorLoc;

    // Shadow mapping
    GLuint m_shadowMapTexture;
    GLuint m_shadowMapFBO;
    GLuint m_shadowMapDepthRenderBuffer;
    bool m_shadowMapping;
    float m_lightMatrix[16];

    // List of active animations
    std::list<BaseAnimation*> m_animations;

    // Camera location (in world coordinates)
    float m_cameraLocation[3];
    float m_cameraTarget[3];

    // Lens flares
    int m_lensFlareMaxSize;
    std::list<LensFlare> m_lensFlares;
    bool m_sunVisible;
    int m_sunScreenX;
    int m_sunScreenY;
    TimeSample* m_lastSunVisibilityTestTime;

    // Objects
    PhysicsStageStatics* m_statics;
    Pillar* m_pillar;
    Skybox* m_skybox;
    Vehicle* m_vehicle;

    // Vehicle wheel rotation
    RotationAnimation* m_wheelRotationAnimation;
    float m_wheelRotation;

    // Object physics shapes
    btCollisionShape* m_pillarShape;
    btCollisionShape* m_vehicleShape;

    // Object instances / physics rigid bodies
    std::vector<btRigidBody*> m_pillarBodies;
    std::list<ObjectInstance*> m_terrains;
    std::list<ObjectInstance*> m_wallSegments;
    std::list<ObjectInstance*> m_wallCorners;
    std::list<ObjectInstance*> m_trees;
    btRigidBody* m_vehicleBody;

    // Physics engine objects
    btBroadphaseInterface* m_broadphase;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    // Time of the previous physics step / frame render
    timeval m_lastStepTime;
};

#endif // PILLARSTAGE_H
