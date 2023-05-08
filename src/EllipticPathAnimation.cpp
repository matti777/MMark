#include <math.h>

#include "EllipticPathAnimation.h"

EllipticPathAnimation::EllipticPathAnimation(float a, float b, float z,
                                             float iterationDuration,
                                             float* location)
    : BaseAnimation(0.0, iterationDuration),
      m_radiusX(a),
      m_radiusY(b),
      m_z(z),
      m_location(location)
{
}

bool EllipticPathAnimation::HasCompleted(const TimeSample& /*time*/) const
{
    return false;
}

bool EllipticPathAnimation::Animate(const TimeSample& time)
{
    float elapsed = time.ElapsedTimeSince(m_baseTime);
    while ( elapsed >= m_duration )
    {
        elapsed -= m_duration;
    }

    float t = (elapsed / m_duration) * 2 * M_PI;
    float acos_t = m_radiusX * cosf(t);
    float bsin_t = m_radiusY * sinf(t);
    m_location[0] = acos_t - bsin_t;
    m_location[1] = acos_t + bsin_t;
    m_location[2] = m_z;

    return false;
}

