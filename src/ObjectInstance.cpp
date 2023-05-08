#include <btBulletDynamicsCommon.h>

#include "ObjectInstance.h"
#include "MatrixOperations.h"

ObjectInstance::ObjectInstance(btRigidBody* rigidBody, int type)
    : m_rigidBody(rigidBody),
      m_type(type)
{
}

ObjectInstance::ObjectInstance(float* transform, int type)
    : m_rigidBody(NULL),
      m_type(type)
{
    CopyMatrix(transform, m_transform);
}

ObjectInstance::~ObjectInstance()
{
    // Not owned
    m_rigidBody = NULL;
}

