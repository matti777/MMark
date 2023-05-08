#include <math.h>

#include "ChesspieceAnimation.h"
#include "MatrixOperations.h"
#include "ChesspieceInstance.h"

static const float MaxHeightRatio = 0.1;

ChesspieceAnimation::ChesspieceAnimation(const PiecePosition& pos1,
                                         const PiecePosition& pos2,
                                         float initialDelay,
                                         ChesspieceInstance& instance)
    : TranslationAnimation(initialDelay, 0.0, instance.PositionPtr()),
      m_instance(instance),
      m_dx(pos2.x - pos1.x),
      m_dz(pos2.z - pos1.z)
{
    m_initialLocation[0] = pos1.x;
    m_initialLocation[2] = pos1.z;
    m_destinationLocation[0] = pos2.x;
    m_destinationLocation[2] = pos2.z;

    // Calculate the max height of the trajectory based on
    // destination - initial span length
    float length = sqrtf(m_dx*m_dx + m_dz*m_dz);
    m_maxHeight = length * MaxHeightRatio;

    // Calculate duration from the length of the transition
    m_duration = length * 0.75;
    m_timeScaler = M_PI / m_duration;
}

ChesspieceAnimation::~ChesspieceAnimation()
{
}

bool ChesspieceAnimation::Animate(const TimeSample& time)
{
    float elapsed = time.ElapsedTimeSince(m_baseTime);
    bool completed = false;

    if ( elapsed > (m_initialDelay + m_duration) )
    {
        // Animation duration expired; animation has completed
        CopyVector(m_destinationLocation, m_location);
        completed = true;
    }
    else if ( elapsed >= m_initialDelay )
    {
        // Ongoing; interpolate X/Z position based on the time value
        float factor = (elapsed - m_initialDelay) / m_duration;
        m_location[0] = m_initialLocation[0] + (m_dx * factor);
        m_location[2] = m_initialLocation[2] + (m_dz * factor);

        // Calculate Y trajectory from sin(x) where x = scaled time value
        // with range shift [0..duration] --> [0..PI]
        float x = (elapsed - m_initialDelay) * m_timeScaler;
        m_location[1] = sinf(x) * m_maxHeight;
    }

    return completed;
}
