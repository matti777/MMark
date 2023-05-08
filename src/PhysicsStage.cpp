#include "PhysicsStage.h"
#include "CommonFunctions.h"
#include "PhysicsStageStatics.h"
#include "Pillar.h"
#include "Skybox.h"
#include "Vehicle.h"
#include "MatrixOperations.h"
#include "ObjectMotionState.h"
#include "TranslationAnimation.h"
#include "RotationAnimation.h"
#include "BSplineAnimation.h"
#include "SplineCameraPathAnimation.h"
#include "SimpleTimer.h"
#include "ObjectInstance.h"
#include "TextRenderer.h"

// REFERENCES
// - Tangent Space Bump Mapping:
//   see: http://www.ozone3d.net/tutorials/bump_mapping_p4.php
//   see: http://fabiensanglard.net/bumpMapping/index.php
//   see: http://www.ozone3d.net/tutorials/mesh_deformer_p2.php#tangent_space
//   see: http://www.dhpoware.com/demos/gl3NormalMapping.html
//   see: http://www.terathon.com/code/tangent.html


// TODO
// - lens flare size to scale better with viewport size
// - camera pathing
// - change Terrain secondary texture & shading
// - lens flare occlusion sampling every 100ms

// bias matrix is used to transform unit cube [-1,1] into [0,1]
// all components get c = c*0.5 + 0.5
// Used for shadow mapping.
static const float BiasMatrix[16] = {
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
};

// Identity matrix
static const float IdentityMatrix[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

// TimerAnimation ids
enum TimerIds
{
    VehicleStartTimer,
    VehicleAccelerateTimer,
    SetSmallestFarClipTimer,
    SetMediumFarClipTimer,
    SetSmallerFarClipTimer
//    ,
//    SetLargeFarClipTimer
};

// Sun occlusion test interval for Lens flares (in seconds)
const float SunOcclusionTestInterval = 0.1;

// Default gravity vector
const btVector3 DefaultGravity(0.0, -9.81, 0.0);

// Pillar object's Bullet properties
static const float PillarMass = 0.5;
static const float PillarBounciness = 0.3;
static const float PillarFriction = 0.85;

// Pillar layout
static const int NumPillarRows = 5;
static const int NumPillarColumns = 10;
static const float PillarSpacing = 5.0; // meters

// Vehicle object's Bullet properties
static const float VehicleMass = 0.7;
static const float VehicleBounciness = 0.1;
static const float VehicleFriction = 0.15;

// Vehicle's initial position (on the XZ plane)
const float VehicleInitialX = -60;
const float VehicleInitialZ = 72;

// Vehicle shading properties
const float VehicleWheelShininess = 3.0;
const float VehicleWheelSpecularColor[3] = { 108/255.0, 102/255.0, 64/255.0 };
const float VehicleBodyShininess = 16.0;
const float VehicleBodySpecularColor[3] = { 0.8, 0.9, 0.9 };

// Duration of the stage, in seconds
static const float StageDuration = 76.0;

// Info popup texts
static const char* InfoPopupHeader = "game environment test";
static const char* InfoPopupMessage = "physics / shadow mapping";

// Near / far clip planes for this stage
const float PhysicsStageNearClip = 1.0;
const float PhysicsStageFarClip = 300.0;

// Size of the shadow map (ShadowMapSize * ShadowMapSize)
const int ShadowMapSize = 2048;

// Sun position
static const float SunWorldPosition[4] = { -550.0, 470.0, -450.0, 1.0 };

PhysicsStage::PhysicsStage(TextRenderer& textRenderer,
                           GLuint rectIndexBuffer, GLuint defaultFrameBuffer,
                           GLuint simpleColorProgram,
                           GLint simpleColorMvpLoc, GLint simpleColorColorLoc)
    : BaseStage(textRenderer, rectIndexBuffer, defaultFrameBuffer,
                simpleColorProgram, simpleColorMvpLoc, simpleColorColorLoc,
                StageDuration,
                InfoPopupHeader, InfoPopupMessage, 
                PhysicsStageNearClip, PhysicsStageFarClip),
      m_defaultProgram(0),
      m_skyboxProgram(0),
      m_terrainProgram(0),
      m_shadowMapProgram(0),
      m_shadowMapTransparentProgram(0),
      m_vehicleProgram(0),
      m_pillarTexture(0),
      m_wallSegmentTexture(0),
      m_skyboxTexture(0),
      m_terrainTexture(0),
      m_terrainTexture2(0),
      m_treeBarkTexture(0),
      m_treeLeavesTexture(0),
      m_lensFlareTexture(0),
      m_walkwayTexture(0),
      m_buggyBlueTexture(0),
      m_buggyNormalmap(0),
      m_defaultMvpLoc(-1),
      m_defaultTextureLoc(-1),
      m_defaultLightPosLoc(-1),
      m_defaultShadowTextureLoc(-1),
      m_defaultShadowMatrixLoc(-1),
      m_skyboxMvpLoc(-1),
      m_skyboxTextureLoc(-1),
      m_terrainMvpLoc(-1),
      m_terrainMvLoc(-1),
      m_terrainTextureLoc(-1),
      m_terrainTexture2Loc(-1),
      m_terrainShadowTextureLoc(-1),
      m_terrainShadowMatrixLoc(-1),
      m_shadowMapMvpLoc(-1),
      m_shadowMapTransparentMvpLoc(-1),
      m_shadowMapTransparentTextureLoc(-1),
      m_vehicleMvpLoc(-1),
      m_vehicleTextureLoc(-1),
      m_vehicleNormalmapLoc(-1),
      m_vehicleLightPosLoc(-1),
      m_vehicleEyePosLoc(-1),
      m_vehicleShadowTextureLoc(-1),
      m_vehicleShadowMatrixLoc(-1),
      m_vehicleShininessLoc(-1),
      m_vehicleSpecularColorLoc(-1),
      m_shadowMapTexture(0),
      m_shadowMapFBO(0),
      m_shadowMapDepthRenderBuffer(0),
      m_lensFlareMaxSize(-1),
      m_sunVisible(false),
      m_sunScreenX(-1),
      m_sunScreenY(-1),
      m_lastSunVisibilityTestTime(NULL),
      m_statics(NULL),
      m_pillar(NULL),
      m_skybox(NULL),
      m_vehicle(NULL),
      m_wheelRotation(0.0),
      m_pillarShape(NULL),
      m_vehicleShape(NULL),
      m_vehicleBody(NULL),
      m_broadphase(NULL),
      m_collisionConfiguration(NULL),
      m_dispatcher(NULL),
      m_solver(NULL),
      m_dynamicsWorld(NULL)
{
    memset(m_cameraTarget, 0, sizeof(m_cameraTarget));
    memset(m_cameraLocation, 0, sizeof(m_cameraLocation));
    m_lastStepTime.tv_sec = 0;
    m_lastStepTime.tv_usec = 0;
}

PhysicsStage::~PhysicsStage()
{
    LOG_DEBUG("PhysicsStage::~PhysicsStage()");
    TeardownImpl();
}

void PhysicsStage::UpdateScore(const TimeSample& now)
{
    BaseStage::UpdateScore(now);

    // Scale the base score up to compensate for the lower frame rate
    // compared to the chess scene
    m_stageData.m_score = (int)(m_stageData.m_score * 2.5);

    if ( !DepthBufferExtensionPresent() )
    {
        LOG_DEBUG("Depth buffer support missing, reducing score");
        m_stageData.m_score /= 2;
    }
}

void PhysicsStage::UpdateDefaultUniforms(const float* objectTransform)
{
    // Transform the light position into object space
    float inverseObjectTransform[16];
    CalculateInverseTransform(objectTransform, inverseObjectTransform);
    float lightObjectSpacePos[3];
    Transformv3(inverseObjectTransform, SunWorldPosition,
                lightObjectSpacePos);

    // Update uniforms
    glUniform3fv(m_defaultLightPosLoc, 1, lightObjectSpacePos);

    // Setup shadow matrix: modelMatrix*lightVPmatrix*biasMatrix.
    float shadowMatrix[16];
    MatrixMultiply(objectTransform, m_lightMatrix, shadowMatrix);
    MatrixMultiply(shadowMatrix, BiasMatrix, shadowMatrix);
    glUniformMatrix4fv(m_defaultShadowMatrixLoc, 1, GL_FALSE, shadowMatrix);
}

void PhysicsStage::RenderTreeTrunksDepthMap(float* vpMatrix)
{
    m_statics->PrepareRenderTreeTrunkShadowMap();

    for (std::list<ObjectInstance*>::iterator iter = m_trees.begin();
         iter != m_trees.end(); iter++ )
    {
        float mvpMatrix[16];
        MatrixMultiply((*iter)->GetTransform(), vpMatrix, mvpMatrix);
        glUniformMatrix4fv(m_shadowMapMvpLoc, 1, GL_FALSE, mvpMatrix);
        m_statics->RenderTreeTrunk();
    }
}

void PhysicsStage::RenderTreeLeavesDepthMap(float* vpMatrix)
{
    m_statics->PrepareRenderTreeLeavesShadowMap();

    // Provide texture for alpha-testing
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_shadowMapTransparentTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_treeLeavesTexture);

    glDisable(GL_CULL_FACE);
    for (std::list<ObjectInstance*>::iterator iter = m_trees.begin();
         iter != m_trees.end(); iter++ )
    {
        float mvpMatrix[16];
        MatrixMultiply((*iter)->GetTransform(), vpMatrix, mvpMatrix);
        glUniformMatrix4fv(m_shadowMapTransparentMvpLoc,
                           1, GL_FALSE, mvpMatrix);
        m_statics->RenderTreeLeaves();
    }
    glEnable(GL_CULL_FACE);
}

void PhysicsStage::RenderDepthMap()
{
    // Adjust viewport to match the shadow map size
    glViewport(0, 0, ShadowMapSize, ShadowMapSize);

    // Start using our offscreen FBO & texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);

    // Check FBO status
    //TODO remove this; shouldnt be necessary
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if ( fboStatus != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG_DEBUG("RenderDepthMap(): FBO not complete.");
        return;
    }

    // Depth checking on and clear the buffer
    glClear(GL_DEPTH_BUFFER_BIT);

    // Render the shadow-casting objects
    glUseProgram(m_shadowMapProgram);
    glDisableVertexAttribArray(NORMAL_INDEX);
    glDisableVertexAttribArray(TEXCOORD_INDEX);
    RenderPillars(m_lightMatrix, m_shadowMapMvpLoc);
    RenderWallCorners(m_lightMatrix, m_shadowMapMvpLoc);
    RenderWallSegments(m_lightMatrix, m_shadowMapMvpLoc);
    RenderTreeTrunksDepthMap(m_lightMatrix);
    RenderVehicle(m_lightMatrix, m_shadowMapMvpLoc);
    glUseProgram(m_shadowMapTransparentProgram);
    glEnableVertexAttribArray(TEXCOORD_INDEX);
    RenderTreeLeavesDepthMap(m_lightMatrix);
    glEnableVertexAttribArray(NORMAL_INDEX);

    // Go back to using the default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);

    // Reset the viewport
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
}

void PhysicsStage::RenderSkybox(float* viewMatrix)
{
    // Extract rotation from the view matrix and calculate (m)vp matrix
    float rotationMatrix[16];
    MatrixExtractRotation(viewMatrix, rotationMatrix);
    float skyboxMvpMatrix[16];
    MatrixMultiply(rotationMatrix, m_perspectiveProjectionMatrix,
                   skyboxMvpMatrix);

    glUniformMatrix4fv(m_skyboxMvpLoc, 1, GL_FALSE, skyboxMvpMatrix);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_skyboxTextureLoc, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    m_skybox->Render();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
}

void PhysicsStage::PrepareRenderPillars()
{
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_pillarTexture);
}

void PhysicsStage::RenderPillars(float* vpMatrix, GLint mvpLoc)
{
    m_pillar->PrepareRender();

    for ( unsigned int i = 0; i < m_pillarBodies.size(); i++ )
    {
        btRigidBody* body = m_pillarBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(body->getMotionState());

        // Upload this pillar's transform
        float mvpMatrix[16];
        MatrixMultiply(motionState->GetObjectTransform(), vpMatrix, mvpMatrix);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
        if ( mvpLoc == m_defaultMvpLoc )
        {
            UpdateDefaultUniforms(motionState->GetObjectTransform());
        }

        // Render the pillar
        m_pillar->Render();
    }
}

void PhysicsStage::RenderTerrain(float* vMatrix, float* vpMatrix)
{
    glUniformMatrix4fv(m_terrainMvpLoc, 1, GL_FALSE, vpMatrix);
    glUniformMatrix4fv(m_terrainMvLoc, 1, GL_FALSE, vMatrix);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_terrainTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_terrainTextureLoc, 1);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture2);

    glActiveTexture(GL_TEXTURE2);
    glUniform1i(m_terrainShadowTextureLoc, 2);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);

    // Setup shadow matrix: modelMatrix*lightVPmatrix*biasMatrix.
    // modelMatrix = identity for terrain
    float shadowMatrix[16];
    MatrixMultiply(m_lightMatrix, BiasMatrix, shadowMatrix);
    glUniformMatrix4fv(m_terrainShadowMatrixLoc, 1, GL_FALSE, shadowMatrix);

    m_statics->RenderTerrain();
}

void PhysicsStage::RenderWalkway(float* vpMatrix)
{
    glUniformMatrix4fv(m_defaultMvpLoc, 1, GL_FALSE, vpMatrix);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_walkwayTexture);

    UpdateDefaultUniforms(IdentityMatrix);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2, -2);
    m_statics->RenderWalkway();
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void PhysicsStage::PrepareRenderWall()
{
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_wallSegmentTexture);
}

void PhysicsStage::RenderWallCorners(float* vpMatrix, GLint mvpLoc)
{
    m_statics->PrepareRenderWallCorner();
    
    for ( std::list<ObjectInstance*>::iterator iter = m_wallCorners.begin();
         iter != m_wallCorners.end(); iter++ )
    {
        float mvpMatrix[16];
        btRigidBody* body = (*iter)->GetBody();
        ObjectMotionState* motionState =
            static_cast<ObjectMotionState*>(body->getMotionState());
        MatrixMultiply(motionState->GetObjectTransform(),
                       vpMatrix, mvpMatrix);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
        m_statics->RenderWallCorner();
    }
}

void PhysicsStage::RenderWallSegments(float* vpMatrix, GLint mvpLoc)
{
    m_statics->PrepareRenderWallSegment();
    
    for ( std::list<ObjectInstance*>::iterator iter = m_wallSegments.begin();
         iter != m_wallSegments.end(); iter++ )
    {
        float mvpMatrix[16];
        btRigidBody* body = (*iter)->GetBody();
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(body->getMotionState());
        MatrixMultiply(motionState->GetObjectTransform(),
                       vpMatrix, mvpMatrix);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
        if ( mvpLoc == m_defaultMvpLoc )
        {
            UpdateDefaultUniforms(motionState->GetObjectTransform());
        }
        m_statics->RenderWallSegment();
    }
}

void PhysicsStage::UpdateVehicleUniforms(const float* objectTransform)
{
    // Transform the camera / eye position into object space
    float inverseObjectTransform[16];
    CalculateInverseTransform(objectTransform, inverseObjectTransform);
    float eyeObjectSpacePos[3];
    Transformv3(inverseObjectTransform, m_cameraLocation,
                eyeObjectSpacePos);

    // Transform the light position into object space
    float lightObjectSpacePos[3];
    Transformv3(inverseObjectTransform, SunWorldPosition,
                lightObjectSpacePos);

    // Update uniforms
    glUniform3fv(m_vehicleLightPosLoc, 1, lightObjectSpacePos);
    glUniform3fv(m_vehicleEyePosLoc, 1, eyeObjectSpacePos);

    // Setup shadow matrix: modelMatrix*lightVPmatrix*biasMatrix.
    float shadowMatrix[16];
    MatrixMultiply(objectTransform, m_lightMatrix, shadowMatrix);
    MatrixMultiply(shadowMatrix, BiasMatrix, shadowMatrix);
    glUniformMatrix4fv(m_vehicleShadowMatrixLoc, 1, GL_FALSE, shadowMatrix);
}

void PhysicsStage::SetupWheelRender(const float* wheelTransform,
                                    const float* objectTransform,
                                    const float* vpMatrix,
                                    GLint mvpLoc)
{
    float modelTransform[16];
    float mvpMatrix[16];

    MatrixMultiply(wheelTransform, objectTransform, modelTransform);
    MatrixMultiply(modelTransform, vpMatrix, mvpMatrix);
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
    if ( mvpLoc == m_vehicleMvpLoc )
    {
        UpdateVehicleUniforms(modelTransform);
    }
}

void PhysicsStage::RenderVehicle(float* vpMatrix, GLint mvpLoc)
{
    if ( mvpLoc == m_vehicleMvpLoc )
    {
        // Rendering normally instead of the shadow map
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_vehicleTextureLoc, 0);
        glBindTexture(GL_TEXTURE_2D, m_buggyBlueTexture);

        glActiveTexture(GL_TEXTURE1);
        glUniform1i(m_vehicleNormalmapLoc, 1);
        glBindTexture(GL_TEXTURE_2D, m_buggyNormalmap);

        glActiveTexture(GL_TEXTURE2);
        glUniform1i(m_vehicleShadowTextureLoc, 2);
        glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);

        glEnableVertexAttribArray(TANGENT_INDEX);
    }

    ObjectMotionState* motionState =
            static_cast<ObjectMotionState*>(m_vehicleBody->getMotionState());
    const float* objectTransform = motionState->GetObjectTransform();

    // Update the wheel transforms with current rotation
    m_vehicle->UpdateWheelTransforms(m_wheelRotation);

    // Disable element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Render vehicle body
    float mvpMatrix[16];
    MatrixMultiply(objectTransform, vpMatrix, mvpMatrix);
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
    if ( mvpLoc == m_vehicleMvpLoc )
    {
        // Set illumination properties for body
        glUniform1f(m_vehicleShininessLoc, VehicleBodyShininess);
        glUniform3fv(m_vehicleSpecularColorLoc, 1, VehicleBodySpecularColor);
        
        UpdateVehicleUniforms(objectTransform);
    }
    m_vehicle->RenderBody();

    if ( mvpLoc == m_vehicleMvpLoc )
    {
        // Set illumination properties for wheels
        glUniform1f(m_vehicleShininessLoc, VehicleWheelShininess);
        glUniform3fv(m_vehicleSpecularColorLoc, 1, VehicleWheelSpecularColor);
    }
    
    // Render front right wheel
    SetupWheelRender(m_vehicle->FrontRightWheelTransform(),
                     objectTransform, vpMatrix, mvpLoc);
    m_vehicle->PrepareRenderFrontWheel();
    m_vehicle->RenderFrontWheel();

    // Render front left wheel
    SetupWheelRender(m_vehicle->FrontLeftWheelTransform(),
                     objectTransform, vpMatrix, mvpLoc);
    m_vehicle->RenderFrontWheel();

    // Render rear right wheel
    SetupWheelRender(m_vehicle->RearRightWheelTransform(),
                     objectTransform, vpMatrix, mvpLoc);
    m_vehicle->PrepareRenderRearWheel();
    m_vehicle->RenderRearWheel();

    // Render rear left wheel
    SetupWheelRender(m_vehicle->RearLeftWheelTransform(),
                     objectTransform, vpMatrix, mvpLoc);
    m_vehicle->RenderRearWheel();

    if ( mvpLoc == m_vehicleMvpLoc )
    {
        glDisableVertexAttribArray(TANGENT_INDEX);
    }
}

// For sorting the trees
struct ObjectTransform
{
    // Object transform
    float m_transform[16];

    // The MVP matrix for the object
    float m_mvpMatrix[16];
};

bool CompareMvps(ObjectTransform& first, ObjectTransform& second)
{
    // Compare the Z components of the matrix translation part
    return (first.m_mvpMatrix[14] > second.m_mvpMatrix[14]);
}

void PhysicsStage::RenderTrees(float* vpMatrix)
{
    // Sort the trees back to front
    std::list<ObjectTransform> trees;

    for ( std::list<ObjectInstance*>::iterator iter = m_trees.begin();
          iter != m_trees.end(); iter++ )
    {
        ObjectTransform transform;
        CopyMatrix((*iter)->GetTransform(), transform.m_transform);
        MatrixMultiply((*iter)->GetTransform(), vpMatrix,
                       transform.m_mvpMatrix);
        trees.push_back(transform);
    }
    trees.sort(CompareMvps);

    // First pass; render the trunks of each tree
    m_statics->PrepareRenderTreeTrunk();

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_treeBarkTexture);

    std::list<ObjectTransform>::const_iterator iter = trees.begin();
    for (; iter != trees.end(); iter++ )
    {
        glUniformMatrix4fv(m_defaultMvpLoc, 1, GL_FALSE, (*iter).m_mvpMatrix);
        UpdateDefaultUniforms((*iter).m_transform);
        m_statics->RenderTreeTrunk();
    }

    // Second pass; render the leaves of each tree
    m_statics->PrepareRenderTreeLeaves();

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_defaultTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, m_treeLeavesTexture);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    for ( iter = trees.begin(); iter != trees.end(); iter++ )
    {
        glUniformMatrix4fv(m_defaultMvpLoc, 1, GL_FALSE, (*iter).m_mvpMatrix);
        UpdateDefaultUniforms((*iter).m_transform);
        m_statics->RenderTreeLeaves();
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void PhysicsStage::Animate(const TimeSample& time)
{
    for (std::list<BaseAnimation*>::iterator iter = m_animations.begin();
        iter != m_animations.end(); iter++)
    {
        if ( (*iter)->Animate(time) )
        {
            // Animation completed; remove it from the list
            delete (*iter);
            iter = m_animations.erase(iter);
        }
    }
}

void PhysicsStage::DetectSunVisibility(float* vpMatrix)
{
    // Transform the sun world space -> clip space
    float sunScreenPos[4];
    Transformv4(vpMatrix, SunWorldPosition, sunScreenPos);

    // Calculate x = x' / w, y = y' / w ie. transform clip space -> NDC
    float x = sunScreenPos[0] / sunScreenPos[3];
    float y = sunScreenPos[1] / sunScreenPos[3];

    // Transform from normalized [-1, 1] to [0, screen_dimension]
    // And negate y since opengl y+ is up and screen y+ is down
    m_sunScreenX = int(((x * 0.5) + 0.5) * m_viewportWidth);
    m_sunScreenY = int(((-y * 0.5) + 0.5) * m_viewportHeight);

    bool flareOnScreen = false;

    // Check if Sun position is on screen
    if ( (sunScreenPos[2] > PhysicsStageNearClip) &&
        (m_sunScreenX >= 0) && (m_sunScreenX < m_viewportWidth) &&
        (m_sunScreenY >= 0) && (m_sunScreenY < m_viewportHeight) ) {
        flareOnScreen = true;
    }

    if ( !flareOnScreen )
    {
        m_sunVisible = false;
        return;
    }

    float timeSinceLastTest = 60*60;
    if ( m_lastSunVisibilityTestTime == NULL )
    {
        m_lastSunVisibilityTestTime = new TimeSample();
    }
    else
    {
        timeSinceLastTest = m_lastSunVisibilityTestTime->ElapsedTime();
    }

    // Do not test every frame to avoid pipeline stalls due to glReadPixels().
    // Also removes the stroboscope effect with occlusion changing every frame
    if ( timeSinceLastTest >= SunOcclusionTestInterval )
    {
        // OpenGL uses inverted screen coordinates
        int invertedY = m_viewportHeight - m_sunScreenY;

        // Read the 4-byte RGBA pixel at sun's location
        GLubyte pixel[4];
        glReadPixels(m_sunScreenX, invertedY, 1, 1, GL_RGBA,
                     GL_UNSIGNED_BYTE, (GLvoid*)pixel);

        // If alpha = 1.0 at the pixel, the sun is occluded by an object
        m_sunVisible = ( pixel[3] < 255 );
        m_lastSunVisibilityTestTime->Reset();
    }
}

void PhysicsStage::RenderLensFlares()
{
    // Set up the shader program + uniforms. We'll reuse the simpletexture
    // program from the text renderer
    m_textRenderer.SetProgram();
    m_textRenderer.SetTexture(m_lensFlareTexture);

    // Use additive blending for drawing the flares
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);

    // Prevent lens flares affecting the alpha buffer
    glColorMask(true, true, true, false);

    // Calculate the 2D vector from screen center to the light
    float screenMidX = (m_viewportWidth / 2);
    float screenMidY = (m_viewportHeight / 2);
    float vx = m_sunScreenX - screenMidX;
    float vy = m_sunScreenY - screenMidY;

    CommonGL::Rect rect;
    for ( std::list<LensFlare>::iterator iter = m_lensFlares.begin();
         iter != m_lensFlares.end(); iter++ )
    {
        const LensFlare& flare = *iter;
        int size = flare.m_sizeScaler * m_lensFlareMaxSize;
        int x = (int)(screenMidX + (flare.m_distance * vx));
        int y = (int)(screenMidY + (flare.m_distance * vy));
        rect.SetCentered(x, y, size);
        DrawImage2D(rect, m_viewportWidth, m_viewportHeight,
                    flare.m_leftU, 1.0, flare.m_rightU, 0.0);
    }

    // Restore alpha based blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(true, true, true, true);
}

void PhysicsStage::RenderImpl(const TimeSample& time)
{
//    LOG_DEBUG("error 1 = 0x%x", glGetError());

    // Advance the physics calculations
    StepPhysics();

    // Animate everything
    Animate(time);

    // Copy the object transforms
    CopyPhysicsTransforms();

    if ( m_shadowMapping )
    {
        // Render the shadow depth map for this frame
        RenderDepthMap();
    }

    // Create a look-at matrix
    float lookat[16];
    MatrixSetLookat(lookat, m_cameraLocation, m_cameraTarget);

    // Setup view-projection matrix
    float vpMatrix[16];
    MatrixMultiply(lookat, m_perspectiveProjectionMatrix, vpMatrix);

    // Calculate sun position on screen from previous frame and detect occlusion
    DetectSunVisibility(vpMatrix);

    // Clear the screen & depth buffer
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the skybox
    glUseProgram(m_skyboxProgram);
    RenderSkybox(lookat);

    // Use terrain shader and draw terrain
    glUseProgram(m_terrainProgram);
    RenderTerrain(lookat, vpMatrix);

    // Render the vehicle
    glUseProgram(m_vehicleProgram);
    RenderVehicle(vpMatrix, m_vehicleMvpLoc);

    // Use the default program; shared by most renders in this scene
    glUseProgram(m_defaultProgram);

    // Setup shadow texture
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_defaultShadowTextureLoc, 1);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);

    // Render the road/walkway
    RenderWalkway(vpMatrix);

    // Render most of the static objects
    PrepareRenderWall();
    RenderWallCorners(vpMatrix, m_defaultMvpLoc);
    RenderWallSegments(vpMatrix, m_defaultMvpLoc);

    // Render all the pillars
    PrepareRenderPillars();
    RenderPillars(vpMatrix, m_defaultMvpLoc);
    
    // Render trees last so the transparency of the leaves works properly
    RenderTrees(vpMatrix);

    if ( m_sunVisible )
    {
        // Draw the lens flares
        RenderLensFlares();
    }
    //    LOG_DEBUG("error 2 = 0x%x", glGetError());

    //TODO remove: render depth map on screen for debug
//    m_textRenderer.SetProgram();
//    m_textRenderer.SetTexture(m_shadowMapTexture);
//    CommonGL::Rect depthMapRect(50, 50, 350, 350);
//    DrawImage2D(depthMapRect, m_viewportWidth, m_viewportHeight);

//    LOG_DEBUG("error = 0x%x", glGetError());
}

void PhysicsStage::StepPhysics()
{
    btScalar seconds = 0.0;
    timeval now;
    gettimeofday(&now, NULL);

    if ( m_lastStepTime.tv_sec > 0 )
    {
        // there has been a previous step, calculate time between steps
        seconds = (btScalar)(now.tv_sec - m_lastStepTime.tv_sec);
        seconds += (btScalar)((now.tv_usec - m_lastStepTime.tv_usec) * 0.000001);
    }

    m_lastStepTime.tv_sec = now.tv_sec;
    m_lastStepTime.tv_usec = now.tv_usec;

    // Keep vehicle from "falling asleep"
    m_vehicleBody->activate();

    // Be sure that timeStep < substeps*fixedTimeStep (1/60)
    int substeps = ceil(seconds / (1/60.0));
    m_dynamicsWorld->stepSimulation(seconds, substeps);
}

void PhysicsStage::CopyPhysicsTransforms()
{
    // Pillars
    for ( unsigned int i = 0; i < m_pillarBodies.size(); i++ )
    {
        btRigidBody* body = m_pillarBodies[i];
        ObjectMotionState* motionState =
                static_cast<ObjectMotionState*>(body->getMotionState());
        motionState->UpdateObjectTransform();
    }

    // Vehicle
    ObjectMotionState* motionState =
            static_cast<ObjectMotionState*>(m_vehicleBody->getMotionState());
    motionState->UpdateObjectTransform();

    // TODO boards?
}

void PhysicsStage::AddNewPillar(float x, float y, float z)
{
//    LOG_DEBUG("AddNewPillar(): %f, %f, %f", x, y, z);

    if ( m_pillarShape == NULL )
    {
        // Create the shared shape object to match the dimensions of the
        // rendarable object; arguments to btVector3 are half dimensions;
        // it describes a vector from origin to a 'corner' of the shape
        m_pillarShape = new btCylinderShape(Pillar::GetExtents());
    }

    // Create the motion state. It will reflect the given initial position
    // and orientation of the object
    ObjectMotionState* motionState =
            new ObjectMotionState(btTransform(btQuaternion(0.0, 0.0, 0.0, 1.0),
                                              btVector3(x, y, z)), m_pillar);

    // Calculate inertia
    btScalar mass = PillarMass;
    btVector3 inertia(0, 0, 0);
    m_pillarShape->calculateLocalInertia(mass, inertia);

    // Construct the rigid body for this block
    btRigidBody::btRigidBodyConstructionInfo
            bodyCI(PillarMass, motionState, m_pillarShape, inertia);
    bodyCI.m_friction = PillarFriction;
    bodyCI.m_restitution = PillarBounciness;
    btRigidBody* body = new btRigidBody(bodyCI);
    body->setUserPointer(m_pillar);

    // Add the created body to the world
    m_dynamicsWorld->addRigidBody(body);

    // ..and to our internal list of bodies
    m_pillarBodies.push_back(body);
}

void PhysicsStage::CreatePillars()
{
    // Create NumPillarRows x NumPillarColumns pillars
    const float Xoffs = -58;
    const float Zoffs = -33;
    
    btVector3 extents = Pillar::GetExtents();

    for ( int j = 0; j < NumPillarRows; j++ )
    {
        for ( int i = 0; i < NumPillarColumns; i++ )
        {
            float x = (i - (NumPillarColumns / 2)) * PillarSpacing + Xoffs;
            float y = extents.getY();
            float z = (j - (NumPillarRows / 2)) * PillarSpacing + Zoffs;
            AddNewPillar(x, y, z);
        }
    }
}

void PhysicsStage::CreateVehicle()
{
    btVector3 extents = Vehicle::GetExtents();
    if ( m_vehicleShape == NULL )
    {
        m_vehicleShape = new btBoxShape(extents);
    }

    // Initial position of the vehicle
    btVector3 initialPos(VehicleInitialX, extents.getY(), VehicleInitialZ);

    // Create the motion state. It will reflect the given initial position
    // and orientation of the object
    ObjectMotionState* motionState =
            new ObjectMotionState(btTransform(btQuaternion(0.0, 0.0, 0.0, 1.0),
                                              initialPos), m_vehicle);

    // Calculate inertia
    btScalar mass = VehicleMass;
    btVector3 inertia(0, 0, 0);
    m_pillarShape->calculateLocalInertia(mass, inertia);

    // Construct the rigid body for this block
    btRigidBody::btRigidBodyConstructionInfo
            bodyCI(VehicleMass, motionState, m_vehicleShape, inertia);
    bodyCI.m_friction = VehicleFriction;
    bodyCI.m_restitution = VehicleBounciness;
    bodyCI.m_linearSleepingThreshold = btScalar(0.0f);
    bodyCI.m_angularSleepingThreshold = btScalar(0.0f);

    m_vehicleBody = new btRigidBody(bodyCI);
    m_vehicleBody->setUserPointer(m_vehicle);

    // Add the created body to the world
    m_dynamicsWorld->addRigidBody(m_vehicleBody);
}

void PhysicsStage::BulletTickCallback(btDynamicsWorld* world,
                                      btScalar /*timeStep*/)
{
    // NOTE this is executed in the physics thread

    PhysicsStage* stage =
            reinterpret_cast<PhysicsStage*>(world->getWorldUserInfo());
    int numManifolds = world->getDispatcher()->getNumManifolds();
    for ( int i = 0; i < numManifolds; i++ )
    {
        btPersistentManifold* contactManifold =
                world->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* objA =
                static_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* objB =
                static_cast<btCollisionObject*>(contactManifold->getBody1());

        // Detect a collision between two pillars; when this happens,
        // set normal gravity to the vehicle and stop listening to callbacks
        if ( (objA->getUserPointer() == stage->m_pillar) &&
             (objB->getUserPointer() == stage->m_pillar) )
        {
            stage->m_wheelRotationAnimation->SetRotationSpeed(0.2 * M_PI);
            stage->m_vehicleBody->setGravity(DefaultGravity);
            world->setInternalTickCallback(NULL);
            break;
        }
    }
}

void PhysicsStage::SetupPhysicsEngine()
{
    // create the engine resources
    m_broadphase = new btDbvtBroadphase();
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_solver = new btSequentialImpulseConstraintSolver();

    // create the 'world' and apply gravity
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase,
                                                  m_solver,
                                                  m_collisionConfiguration);
    m_dynamicsWorld->setGravity(DefaultGravity);
    m_dynamicsWorld->setInternalTickCallback(PhysicsStage::BulletTickCallback,
                                             this);
}

void PhysicsStage::SetupLensFlares()
{
    // Calculate the lens flare size from the screen height so it will
    // be somewhat proportionate
    m_lensFlareMaxSize = m_viewportHeight / 2;
    m_lensFlareMaxSize = std::min(m_lensFlareMaxSize, 512);
    m_lensFlareMaxSize = std::max(m_lensFlareMaxSize, 64);
//    LOG_DEBUG("PhysicsStage::SetupLensFlare(): lens flare size: %d", m_lensFlareMaxSize);

    // Clear existing lens flares off the list
    m_lensFlares.clear();

    const LensFlare flares[] = {
        { 0.00, 0.25,   1.0,  1.0 }, // Main flare, starburst
        { 0.50, 0.75,   0.5,  0.5 }, // Yellow halo
        { 0.75, 1.00,   0.33, 0.125 }, // Turquoise round full halo
        { 0.00, 0.25,   0.125,  0.25 }, // Main flare, starburst
        { 0.25, 0.50,   -0.125,  0.75 }, // Blue full hexagon
        { 0.00, 0.25,   -0.33, 0.125 }, // Main flare, starburst
        { 0.25, 0.50,   -0.5,  0.25 }, // Blue full hexagon
        { 0.75, 1.00,   -1.0,  0.40 }, // Turquoise round full halo
    };
    const int numFlares = sizeof(flares) / sizeof(LensFlare);

    for ( int i = 0; i < numFlares; i++ )
    {
        m_lensFlares.push_back(flares[i]);
    }
}

void PhysicsStage::AddTransAnim(float initialX, float initialY, float initialZ,
                                float destinationX, float destinationY,
                                float destinationZ, float initialDelay,
                                float duration, float* location)
{
    m_animations.push_back(new TranslationAnimation(initialX, initialY,
                                                    initialZ, destinationX,
                                                    destinationY, destinationZ,
                                                    initialDelay, duration,
                                                    location));
}

void PhysicsStage::AddTransAnim(const float* initialPtr,
                                float destinationX, float destinationY,
                                float destinationZ, float initialDelay,
                                float duration, float* location)
{
    m_animations.push_back(new TranslationAnimation(initialPtr, destinationX,
                                                    destinationY, destinationZ,
                                                    initialDelay, duration,
                                                    location));
}

void PhysicsStage::TimerCallback(SimpleTimer* timer, int timerId)
{
    // TODO: be sure not to execute these while physics thread is going on

    PhysicsStage* stage = reinterpret_cast<PhysicsStage*>(timer->GetUserData());

    switch ( timerId )
    {
        case VehicleStartTimer:
        {
            // Start moving vehicle slowly; set zero friction and apply velocity
            LOG_DEBUG("start vehicle");
            stage->m_vehicleBody->setFriction(0);
            btVector3 velocity(0, 0, -5.5);
            stage->m_vehicleBody->setLinearVelocity(velocity);
            
            stage->m_wheelRotationAnimation->SetRotationSpeed(1.2 * M_PI);
            break;
        }
        case VehicleAccelerateTimer:
            // Start accelerating the vehicle towards the pillars; restore
            // friction and apply sideways gravity
            LOG_DEBUG("accelerate vehicle");
            stage->m_vehicleBody->setFriction(VehicleFriction);
            stage->m_vehicleBody->setGravity(btVector3(0.0, -0.0, -5.0));
            break;
        case SetSmallestFarClipTimer:
            LOG_DEBUG("Setting shortest frustum");
            stage->m_nearClip = 1.6;
            stage->m_farClip = 18.0;
            stage->RecalculateProjection();
            break;
        case SetMediumFarClipTimer:
            LOG_DEBUG("Setting medium frustum");
            stage->m_farClip = 300.0;
            stage->RecalculateProjection();
            break;
        case SetSmallerFarClipTimer:
            LOG_DEBUG("Setting shorter frustum");
            stage->m_nearClip = 2.5;
            stage->m_farClip = 250.0;
            stage->RecalculateProjection();
            break;
//        case SetLargeFarClipTimer:
//            LOG_DEBUG("Setting large frustum");
//            stage->m_nearClip = PhysicsStageNearClip;
//            stage->m_farClip = PhysicsStageFarClip;
//            stage->RecalculateProjection();
//            break;
    }
}

void PhysicsStage::SetupAnimations()
{
    // Approach from the edge of the map towards the pillars
    float A = 0.0;
    std::vector<Vector3> points;
    points.push_back(Vector3(260, 8, 190));
    points.push_back(Vector3(230, 7, 185));
    points.push_back(Vector3(190, 10, 180));
    points.push_back(Vector3(150, 6, 140));
    points.push_back(Vector3(130, 5, 110)); // hard right
    points.push_back(Vector3(143, 4, 70));
    points.push_back(Vector3(185, 6, 30)); // left turn in canyon
    points.push_back(Vector3(175, 15, -62)); // hard left after canyon
    points.push_back(Vector3(100, 15, -57));
    points.push_back(Vector3(10, 10, -60)); // Arrival to the plain
    points.push_back(Vector3(-20, 7, -32));
    points.push_back(Vector3(-32, 5, -26)); // arriving to the pillars
    points.push_back(Vector3(-38, 2.5, -25));
    points.push_back(Vector3(-41.5, 3.5, -26));
    points.push_back(Vector3(-43, 4.0, -35)); // hard right into the pillars
    points.push_back(Vector3(-73, 5, -35));
    points.push_back(Vector3(-74, 2, -20));

    m_animations.push_back(new SplineCameraPathAnimation(points, A+0, 40,
                                                         m_cameraLocation,
                                                         m_cameraTarget, 3.0));

    // Moving to the vehicle
    float M = A + 39;
    AddTransAnim(m_cameraLocation, -72, 2, 40,  M+0, 6, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -70, 4, 68,  M+6, 3, m_cameraLocation);
    AddTransAnim(m_cameraTarget, -62, 8, 35,  M, 3, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -62, 0, 55,  M+3, 3, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -62, 0, 70,  M+6, 2, m_cameraTarget);

    // Vehicle inspection & cruise towards the pillars;
    float V = M + 9;
    AddTransAnim(m_cameraLocation, -63, 4, 75,   V, 3, m_cameraLocation);
    AddTransAnim(m_cameraLocation,  -60, 2.8, 77,   V+3, 2, m_cameraLocation);
    AddTransAnim(m_cameraLocation,  -57, 2.3, 75,   V+5, 2, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -48, 1, 53,   V+7, 5, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -48, 1, 30,   V+12, 5, m_cameraLocation);
    AddTransAnim(m_cameraLocation, -35, 10, -35,  V+17, 3, m_cameraLocation);

    AddTransAnim(m_cameraTarget,  -60, 0, 72, V+0, 2, m_cameraTarget);
    AddTransAnim(m_cameraTarget,  -60, 2.5, 45, V+7, 5, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -60, 2.5, 22,  V+12, 5, m_cameraTarget);
    AddTransAnim(m_cameraTarget, -60, 1.0, -20,  V+17, 3, m_cameraTarget);

    // Camera path around the pillars while they fall
    float P = V + 20;
    points.clear();
    points.push_back(Vector3(-35, 10, -35));
    points.push_back(Vector3(-35, 12, -55));
    points.push_back(Vector3(-55, 14, -60));
    points.push_back(Vector3(-80, 13, -55));
    points.push_back(Vector3(-82, 13, -35));
    points.push_back(Vector3(-85, 12, -15));
    m_animations.push_back(new BSplineAnimation(points, P, 8,
                                                m_cameraLocation));
    AddTransAnim(m_cameraTarget, -60, 1.0, -30,  P, 4, m_cameraTarget);

    // Vehicle movement control
    m_animations.push_back(new SimpleTimer(0, V+7, this, VehicleStartTimer,
                                           false, PhysicsStage::TimerCallback));
    m_animations.push_back(new SimpleTimer(0, V+18, this,
                                           VehicleAccelerateTimer,
                                           false, PhysicsStage::TimerCallback));
    
    // Other timers
    m_animations.push_back(new SimpleTimer(0, V+2, this,
                                           SetSmallestFarClipTimer,
                                           false, PhysicsStage::TimerCallback));
    m_animations.push_back(new SimpleTimer(0, V+4, this,
                                           SetMediumFarClipTimer,
                                           false, PhysicsStage::TimerCallback));
    m_animations.push_back(new SimpleTimer(0, V+18, this,
                                           SetSmallerFarClipTimer,
                                           false, PhysicsStage::TimerCallback));
//    m_animations.push_back(new SimpleTimer(0, V+22, this,
//                                           SetLargeFarClipTimer,
//                                           false, PhysicsStage::TimerCallback));

    // Vehicle wheel rotation
    m_wheelRotationAnimation = new RotationAnimation(0, 0, &m_wheelRotation);
    m_animations.push_back(m_wheelRotationAnimation);
}

bool PhysicsStage::SetupShadowMapping()
{
    m_shadowMapping = CreateDepthTextureAndFBO(&m_shadowMapFBO,
                                               &m_shadowMapTexture,
                                               &m_shadowMapDepthRenderBuffer,
                                               ShadowMapSize, ShadowMapSize,
                                               false);
    
    // Go back to using the default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer);

    if ( !m_shadowMapping )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("ShadowMap", &m_shadowMapProgram) )
    {
        return false;
    }
    m_shadowMapMvpLoc = glGetUniformLocation(m_shadowMapProgram, "mvp_matrix");

    if ( !LoadShaderFromBundle("ShadowMapTransparent",
                               &m_shadowMapTransparentProgram) )
    {
        return false;
    }
    m_shadowMapTransparentMvpLoc =
            glGetUniformLocation(m_shadowMapTransparentProgram, "mvp_matrix");
    m_shadowMapTransparentTextureLoc =
            glGetUniformLocation(m_shadowMapTransparentProgram, "texture");

    // Setup light's projection matrix to match the shadow map dimensions
    float lightProjection[16];
    MatrixPerspectiveProjection(lightProjection, 8.5,
                                (float)(ShadowMapSize)/(float)ShadowMapSize,
                                760.0f, 870.0f);//TODO adjust z-range more?

    const float lightTarget[3] = { -60, 0, 0 };

    // Setup light orientation (view) matrix
    float lookat[16];
    MatrixSetLookat(lookat, SunWorldPosition, lightTarget);

    // Setup light matrix (view * projection)
    MatrixMultiply(lookat, lightProjection, m_lightMatrix);

    return true;
}

bool PhysicsStage::ViewportResized(int viewportWidth, int viewportHeight)
{
    bool ret = BaseStage::ViewportResized(viewportWidth, viewportHeight);
    if ( ret )
    {
        // Setup lens flares
        SetupLensFlares();
    }

    return ret;
}

bool PhysicsStage::SetupImpl()
{
    LOG_DEBUG("PhysicsStage::Setup()");

    //TODO remove, we'll use animations later on
    // Car viewing:
    m_cameraLocation[0] = -57.0;
    m_cameraLocation[1] = 3.0;
    m_cameraLocation[2] = VehicleInitialZ;
    m_cameraTarget[0] = VehicleInitialX;
    m_cameraTarget[1] = 0.0;
    m_cameraTarget[2] = VehicleInitialZ;
    // Pillars collision viewing:
//    m_cameraLocation[0] = -60.0;
//    m_cameraLocation[1] = 20.0;
//    m_cameraLocation[2] = -70.0;
//    m_cameraTarget[0] = -60.0;
//    m_cameraTarget[1] = 0.0;
//    m_cameraTarget[2] = 0.0;

    // Initialize shadow mapping
    if ( !SetupShadowMapping() )
    {
        LOG_DEBUG("Shadow mapping disabled.");
    }

    if ( !Load2DTextureFromBundle("white_marble.jpg", &m_pillarTexture,
                                  false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("brick_wall.jpg", &m_wallSegmentTexture,
                                  false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("terrain_seamless.jpg",
                                  &m_terrainTexture, false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("cliffside.jpg",
                                  &m_terrainTexture2, false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("tree_bark.jpg",
                                  &m_treeBarkTexture, false, true) )
    {
        return false;
    }
    
    if ( !Load2DTextureFromBundle("tree_leaves.png",
                                  &m_treeLeavesTexture, true, true) )
    {
        return false;
    }
    
    if ( !Load2DTextureFromBundle("lens_flares.jpg",
                                  &m_lensFlareTexture, true, false) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("walkway.jpg",
                                  &m_walkwayTexture, false, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("buggy_blue.jpg",
                                  &m_buggyBlueTexture, true, true) )
    {
        return false;
    }

    if ( !Load2DTextureFromBundle("buggy_normalmap.jpg",
                                  &m_buggyNormalmap, true, true) )
    {
        return false;
    }

    if ( !LoadCubeTextureFromBundle("pillars_skybox_left.jpg",
                                    "pillars_skybox_right.jpg",
                                    "pillars_skybox_top.jpg",
                                    "pillars_skybox_bottom.jpg",
                                    "pillars_skybox_back.jpg",
                                    "pillars_skybox_front.jpg",
                                    &m_skyboxTexture) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("PhysicsStageDefault", &m_defaultProgram) )
    {
        return false;
    }
    
    if ( !LoadShaderFromBundle("Skybox", &m_skyboxProgram) )
    {
        return false;
    }
    
    if ( !LoadShaderFromBundle("Terrain", &m_terrainProgram) )
    {
        return false;
    }

    if ( !LoadShaderFromBundle("Vehicle", &m_vehicleProgram) )
    {
        return false;
    }

    // Get GLSL uniform locations
    m_defaultMvpLoc = glGetUniformLocation(m_defaultProgram, "mvp_matrix");
    m_defaultTextureLoc = glGetUniformLocation(m_defaultProgram, "texture");
    m_defaultLightPosLoc = glGetUniformLocation(m_defaultProgram, "light_pos");
    m_defaultShadowTextureLoc =
            glGetUniformLocation(m_defaultProgram, "shadow_texture");
    m_defaultShadowMatrixLoc =
            glGetUniformLocation(m_defaultProgram, "shadow_matrix");
    m_skyboxMvpLoc = glGetUniformLocation(m_skyboxProgram, "mvp_matrix");
    m_skyboxTextureLoc = glGetUniformLocation(m_skyboxProgram, "skybox");
    m_terrainMvpLoc = glGetUniformLocation(m_terrainProgram, "mvp_matrix");
    m_terrainMvLoc = glGetUniformLocation(m_terrainProgram, "mv_matrix");
    m_terrainTextureLoc = glGetUniformLocation(m_terrainProgram, "texture");
    m_terrainTexture2Loc = glGetUniformLocation(m_terrainProgram, "texture2");
    m_terrainShadowTextureLoc = glGetUniformLocation(m_terrainProgram,
                                                     "shadow_texture");
    m_terrainShadowMatrixLoc = glGetUniformLocation(m_terrainProgram,
                                                     "shadow_matrix");
    m_vehicleMvpLoc = glGetUniformLocation(m_vehicleProgram, "mvp_matrix");
    m_vehicleTextureLoc = glGetUniformLocation(m_vehicleProgram, "texture");
    m_vehicleNormalmapLoc = glGetUniformLocation(m_vehicleProgram, "normalMap");
    m_vehicleLightPosLoc = glGetUniformLocation(m_vehicleProgram, "light_pos");
    m_vehicleEyePosLoc = glGetUniformLocation(m_vehicleProgram, "eye_pos");
    m_vehicleShadowTextureLoc =
            glGetUniformLocation(m_vehicleProgram, "shadow_texture");
    m_vehicleShadowMatrixLoc =
            glGetUniformLocation(m_vehicleProgram, "shadow_matrix");
    m_vehicleShininessLoc = glGetUniformLocation(m_vehicleProgram, "shininess");
    m_vehicleSpecularColorLoc =
            glGetUniformLocation(m_vehicleProgram, "specularColor");

    // Create objects
    m_statics = PhysicsStageStatics::Create();
    m_pillar = Pillar::Create();
    m_skybox = Skybox::Create(15.0);
    m_vehicle = Vehicle::Create();

    if ( (m_statics == NULL) || (m_pillar == NULL) || (m_skybox == NULL) ||
         (m_vehicle == NULL) )
    {
        LOG_DEBUG("PhysicsStage::Setup(): Object initialization failed!");
        return false;
    }

    int glError = glGetError();
    if ( glError != GL_NO_ERROR )
    {
        LOG_DEBUG("glError in setup: 0x%x", glError);
        return false;
    }

    // Set up the physics
    SetupPhysicsEngine();

    // Create all static bodies & collision shapes
    m_statics->CreateObjects(m_terrains, m_wallCorners, m_wallSegments, 
                             m_trees);
    AddBodies(m_terrains);
    AddBodies(m_wallCorners);
    AddBodies(m_wallSegments);

    // Create the pillars
    CreatePillars();

    // Create the vehicle
    CreateVehicle();

    // Setup animations
    SetupAnimations();

    // Make sure we have blending turned on
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0, 1.0, 1.0, 0.3);

    // Set LEQUAL depth func to be able to draw the walkway on top of terrain
    glDepthFunc(GL_LEQUAL);

    return true;
}

void PhysicsStage::AddBodies(std::list<ObjectInstance*>& objects)
{
    for ( std::list<ObjectInstance*>::iterator iter = objects.begin();
         iter != objects.end(); iter++ )
    {
        btRigidBody* body = (*iter)->GetBody();
        if ( body != NULL ) 
        {
            m_dynamicsWorld->addRigidBody(body);
        }
    }
}

void PhysicsStage::DestroyBodies(std::vector<btRigidBody*>& bodies)
{
    for ( unsigned int i = 0; i < bodies.size(); i++ )
    {
        btRigidBody* body = bodies[i];
        m_dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
    bodies.clear();
}

void PhysicsStage::DestroyObjects(std::list<ObjectInstance*>& objects)
{
    for (std::list<ObjectInstance*>::iterator iter = objects.begin();
         iter != objects.end(); iter++)
    {
        btRigidBody* body = (*iter)->GetBody();
        if ( body != NULL ) 
        {
            m_dynamicsWorld->removeRigidBody(body);
            delete body->getMotionState();
            delete body;
        }
        delete (*iter);
        iter = objects.erase(iter);
    }
    objects.clear();
}

void PhysicsStage::TeardownImpl()
{
    LOG_DEBUG("PhysicsStage::Teardown()");

    // Restore default depth func
    glDepthFunc(GL_LESS);

    glDeleteTextures(1, &m_pillarTexture);
    glDeleteTextures(1, &m_wallSegmentTexture);
    glDeleteTextures(1, &m_skyboxTexture);
    glDeleteTextures(1, &m_terrainTexture);
    glDeleteTextures(1, &m_terrainTexture2);
    glDeleteTextures(1, &m_treeBarkTexture);
    glDeleteTextures(1, &m_treeLeavesTexture);
    glDeleteTextures(1, &m_lensFlareTexture);
    glDeleteTextures(1, &m_walkwayTexture);
    glDeleteTextures(1, &m_buggyBlueTexture);
    glDeleteTextures(1, &m_buggyNormalmap);

    UnloadShader(m_defaultProgram);
    UnloadShader(m_skyboxProgram);
    UnloadShader(m_terrainProgram);
    UnloadShader(m_shadowMapProgram);
    UnloadShader(m_shadowMapTransparentProgram);
    UnloadShader(m_vehicleProgram);

    glDeleteFramebuffers(1, &m_shadowMapFBO);
    glDeleteTextures(1, &m_shadowMapTexture);
    glDeleteRenderbuffers(1, &m_shadowMapDepthRenderBuffer);

    m_lensFlares.clear();
    m_sunVisible = false;
    delete m_lastSunVisibilityTestTime;
    m_lastSunVisibilityTestTime = NULL;

    m_lastStepTime.tv_sec = 0;
    m_lastStepTime.tv_usec = 0;

    DestroyBodies(m_pillarBodies);
    DestroyObjects(m_terrains);
    DestroyObjects(m_wallCorners);
    DestroyObjects(m_wallSegments);
    DestroyObjects(m_trees);

    if ( m_dynamicsWorld != NULL )
    {
        m_dynamicsWorld->removeRigidBody(m_vehicleBody);
    }

    if ( m_vehicleBody != NULL )
    {
        delete m_vehicleBody->getMotionState();
        delete m_vehicleBody;
        m_vehicleBody = NULL;
    }

    delete m_pillarShape;
    m_pillarShape = NULL;

    delete m_vehicleShape;
    m_vehicleShape = NULL;

    while ( !m_animations.empty() )
    {
        delete m_animations.front();
        m_animations.pop_front();
    }

    // All remaining animations were already deleted above
    m_wheelRotationAnimation = NULL;

    delete m_statics;
    m_statics = NULL;

    delete m_pillar;
    m_pillar = NULL;

    delete m_skybox;
    m_skybox = NULL;

    delete m_vehicle;
    m_vehicle = NULL;

    delete m_dynamicsWorld;
    m_dynamicsWorld = NULL;

    delete m_solver;
    m_solver = NULL;

    delete m_dispatcher;
    m_dispatcher = NULL;

    delete m_collisionConfiguration;
    m_collisionConfiguration = NULL;

    delete m_broadphase;
    m_broadphase = NULL;
    
    LOG_DEBUG("PhysicsStage::Teardown() done.");
}

