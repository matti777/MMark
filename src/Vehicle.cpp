#include <math.h>

#include "Vehicle.h"
#include "Buggy_body_data.h"
#include "Buggy_front_wheel_data.h"
#include "Buggy_rear_wheel_data.h"
#include "CommonFunctions.h"
#include "MatrixOperations.h"

Vehicle::Vehicle()
    : m_bodyVertexBuffer(0),
      m_frontWheelVertexBuffer(0),
      m_rearWheelVertexBuffer(0)
{
}

Vehicle::~Vehicle()
{
    glDeleteBuffers(1, &m_bodyVertexBuffer);
    glDeleteBuffers(1, &m_frontWheelVertexBuffer);
    glDeleteBuffers(1, &m_rearWheelVertexBuffer);
}

// Offsets (translations) for wheels compared to the vehicle body
const float FrontWheelXOffs = Buggy_bodyHalfWidth;
const float FrontWheelYOffs = -0.34 * Buggy_bodyHalfHeight;
const float FrontWheelZOffs = -0.63 * Buggy_bodyHalfDepth;
const float RearWheelXOffs = Buggy_bodyHalfWidth + 0.2;
const float RearWheelYOffs = -0.25 * Buggy_bodyHalfHeight;
const float RearWheelZOffs = 0.75 * Buggy_bodyHalfDepth;

btVector3 Vehicle::GetExtents()
{
    // Return the outer extents of the vehicle body / wheels
    return btVector3(Buggy_bodyHalfWidth + 2 * 2 *
                     (Buggy_rear_wheelHalfWidth + 0.2),
                     Buggy_bodyHalfHeight + 0.10,
                     Buggy_bodyHalfDepth);
}

void Vehicle::SetupTransforms()
{
    MatrixCreateRotation(m_yRotation, M_PI, 0.0, 1.0, 0.0);

    MatrixCreateTranslation(m_frontRightWheelTranslation,
                            FrontWheelXOffs, FrontWheelYOffs, FrontWheelZOffs);
    MatrixCreateTranslation(m_rearRightWheelTranslation,
                            RearWheelXOffs, RearWheelYOffs, RearWheelZOffs);
    MatrixCreateTranslation(m_frontLeftWheelTranslation,
                            -FrontWheelXOffs, FrontWheelYOffs, FrontWheelZOffs);
    MatrixCreateTranslation(m_rearLeftWheelTranslation,
                            -RearWheelXOffs, RearWheelYOffs, RearWheelZOffs);
}

void Vehicle::UpdateWheelTransforms(float rotation)
{
    float xRotation[16];
    MatrixCreateRotation(xRotation, rotation, 1.0, 0.0, 0.0);

    // Left side wheels transform order: yrotate, xrotate, translate
    MatrixMultiply(m_yRotation, xRotation, m_frontLeftWheelTransform);
    MatrixMultiply(m_frontLeftWheelTransform, m_frontLeftWheelTranslation,
                   m_frontLeftWheelTransform);

    MatrixMultiply(m_yRotation, xRotation, m_rearLeftWheelTransform);
    MatrixMultiply(m_rearLeftWheelTransform, m_rearLeftWheelTranslation,
                   m_rearLeftWheelTransform);

    // Right side wheels transform order: xrotate, translate
    MatrixMultiply(xRotation, m_frontRightWheelTranslation,
                   m_frontRightWheelTransform);
    MatrixMultiply(xRotation, m_rearRightWheelTranslation,
                   m_rearRightWheelTransform);
}

bool Vehicle::Setup()
{
    SetupTransforms();

    // Vehicle body
    VertexAttribsTangent* bodyVertices =
            new VertexAttribsTangent[Buggy_bodyNumVertices];
    CalculateTangentVectors(Buggy_body_vertices, bodyVertices,
                            Buggy_bodyNumPolygons);
    glGenBuffers(1, &m_bodyVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_bodyVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VertexAttribsTangent) * Buggy_bodyNumVertices,
                 bodyVertices, GL_STATIC_DRAW);
    delete bodyVertices;

    // Vehicle front wheel
    VertexAttribsTangent* fwVertices =
            new VertexAttribsTangent[Buggy_front_wheelNumVertices];
    CalculateTangentVectors(Buggy_front_wheel_vertices, fwVertices,
                            Buggy_front_wheelNumPolygons);
    glGenBuffers(1, &m_frontWheelVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_frontWheelVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VertexAttribsTangent) * Buggy_front_wheelNumVertices,
                 fwVertices, GL_STATIC_DRAW);
    delete fwVertices;

    // Vehicle rear wheel
    VertexAttribsTangent* rwVertices =
            new VertexAttribsTangent[Buggy_rear_wheelNumVertices];
    CalculateTangentVectors(Buggy_rear_wheel_vertices, rwVertices,
                            Buggy_rear_wheelNumPolygons);
    glGenBuffers(1, &m_rearWheelVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_rearWheelVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VertexAttribsTangent) * Buggy_rear_wheelNumVertices,
                 rwVertices, GL_STATIC_DRAW);
    delete rwVertices;

    return (glGetError() == GL_NO_ERROR);
}

Vehicle* Vehicle::Create()
{
    Vehicle* vehicle = new Vehicle();
    if ( !vehicle->Setup() )
    {
        delete vehicle;
        LOG_DEBUG("Vehicle::Setup() failed");
        return NULL;
    }
    else
    {
        return vehicle;
    }
}

void Vehicle::RenderBody()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_bodyVertexBuffer);
    SetVertexAttribsTangentPointers();
    glDrawArrays(GL_TRIANGLES, 0, Buggy_bodyNumVertices);
}

void Vehicle::PrepareRenderFrontWheel()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_frontWheelVertexBuffer);
    SetVertexAttribsTangentPointers();
}

void Vehicle::RenderFrontWheel()
{
    glDrawArrays(GL_TRIANGLES, 0, Buggy_front_wheelNumVertices);
}

void Vehicle::PrepareRenderRearWheel()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_rearWheelVertexBuffer);
    SetVertexAttribsTangentPointers();
}

void Vehicle::RenderRearWheel()
{
    glDrawArrays(GL_TRIANGLES, 0, Buggy_rear_wheelNumVertices);
}

