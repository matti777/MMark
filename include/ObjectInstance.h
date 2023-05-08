#ifndef OBJECTINSTANCE_H
#define OBJECTINSTANCE_H

// Forward declarations 
class btRigidBody;

/**
 * Represents an instance of an visual object, optionally containing 
 * Bullet collision body.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class ObjectInstance 
{
public: // Construction and destruction
    /** 
     * Constructs with a Bullet rigid body and optional freeform type id. 
     * Object's transformation can be obtained from the rigid body's motion 
     * state. Does NOT take ownership of the rigid body.
     */
    ObjectInstance(btRigidBody* rigidBody, int type = 0);
    
    /** 
     * Constructs with a transform and optional freeform type id. 
     *
     * @param transform must point to a float[16]
     */
    ObjectInstance(float* transform, int type = 0);
    virtual ~ObjectInstance();
    
public: // Public API
    int GetType() const { return m_type; }
    btRigidBody* GetBody() { return m_rigidBody; }
    float* GetTransform() { return m_transform; }
    
private: // Data
    btRigidBody* m_rigidBody;
    int m_type;
    float m_transform[16]; // Only used if constructed with transform
};

#endif
