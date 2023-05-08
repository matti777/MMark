#ifndef ELLIPTICPATHANIMATION_H
#define ELLIPTICPATHANIMATION_H

#include "BaseAnimation.h"

/**
 * Creates a looped elliptic path in the XY plane (with constant Z)
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class EllipticPathAnimation : public BaseAnimation
{
public:
    /**
     * Constructs the animation.
     *
     * @param a radius in the X direction
     * @param b radius in the Y direction
     * @param z constant value of z
     * @param location where the result is written - must point to float[3]
     * or bigger area
     */
    EllipticPathAnimation(float a, float b, float z, float iterationDuration,
                          float* location);

public: // From BaseAnimation
    bool Animate(const TimeSample& time);
    bool HasCompleted(const TimeSample& time) const;

private: // Data
    float m_radiusX;
    float m_radiusY;
    float m_z;
    float* m_location;
};

#endif // ELLIPTICPATHANIMATION_H
