#ifndef CHESSPIECEANIMATION_H
#define CHESSPIECEANIMATION_H

#include "TranslationAnimation.h"
#include "ChesspieceInstance.h"

/**
 * Provides time-based, interpolated animation for translating a chesspiece
 * from an initial location to another.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class ChesspieceAnimation : public TranslationAnimation
{
public: // Construction and destruction
    /**
     * Constructs the animation.
     *
     * @param location where the result is written - must point to float[3]
     * or bigger area
     */
//    ChesspieceAnimation(float* initialLocation, float* destinationLocation,
//                        float initialDelay, float duration, float* location);
    ChesspieceAnimation(const PiecePosition& pos1, const PiecePosition& pos2,
                        float initialDelay, ChesspieceInstance& instance);
    virtual ~ChesspieceAnimation();

public: // Public API
    bool Animate(const TimeSample& time);

private: // Data
    ChesspieceInstance& m_instance;
    float m_maxHeight;
    float m_timeScaler;
    float m_dx;
    float m_dz;
};

#endif // CHESSPIECEANIMATION_H
