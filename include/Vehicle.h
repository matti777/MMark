#ifndef VEHICLE_H
#define VEHICLE_H

#include <btBulletDynamicsCommon.h>

#include "OpenGLAPI.h"

/**
 * Represents a vehicle in the physics scene.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class Vehicle
{
public: // Construction and deconstruction
    static Vehicle* Create();
   ~Vehicle();

public: // Public API
    void UpdateWheelTransforms(float rotation);
    void RenderBody();
    void PrepareRenderFrontWheel();
    void RenderFrontWheel();
    void PrepareRenderRearWheel();
    void RenderRearWheel();
    const float* FrontRightWheelTransform()
    {
        return m_frontRightWheelTransform;
    }
    const float* FrontLeftWheelTransform()
    {
        return m_frontLeftWheelTransform;
    }
    const float* RearRightWheelTransform()
    {
        return m_rearRightWheelTransform;
    }
    const float* RearLeftWheelTransform()
    {
        return m_rearLeftWheelTransform;
    }

    /** Returns the extents for the Bullet collision shape. */
    static btVector3 GetExtents();

private:
    Vehicle();
    bool Setup();
    void SetupTransforms();

private: // Data
    // Vertex/index buffers
    GLuint m_bodyVertexBuffer;
    GLuint m_frontWheelVertexBuffer;
    GLuint m_rearWheelVertexBuffer;

    // Wheel translations (positions relative to the vehicle body)
    float m_frontRightWheelTranslation[16];
    float m_frontLeftWheelTranslation[16];
    float m_rearRightWheelTranslation[16];
    float m_rearLeftWheelTranslation[16];

    // Rotation used to rotate left side wheels 180 deg around y axis
    float m_yRotation[16];

    // Complete wheel transforms with rotations & translation
    float m_frontRightWheelTransform[16];
    float m_frontLeftWheelTransform[16];
    float m_rearRightWheelTransform[16];
    float m_rearLeftWheelTransform[16];
};

#endif // VEHICLE_H
