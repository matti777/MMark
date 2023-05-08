#ifndef INFOPOPUPANIMATION_H
#define INFOPOPUPANIMATION_H

#include "BaseAnimation.h"
#include "ScalarAnimation.h"

/**
 * Animates an appearing stage information popup position.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class InfoPopupAnimation : public BaseAnimation
{
public: // Constructors and destructor
    InfoPopupAnimation(float initialDelay, float transitionDuration, 
                       float displayDuration, 
                       float hideHeight, float displayHeight, float* height);
    ~InfoPopupAnimation();
    
public: // Public API
    void UpdateHeights(float hideHeight, float displayHeight);

public: // From BaseAnimation
    bool Animate(const TimeSample& time);
    bool HasCompleted(const TimeSample& time) const;
    void ResetTime();
    
private: // Data
    ScalarAnimation m_appearAnimation;
    ScalarAnimation m_disappearAnimation;
    bool m_appearCompleted;
};

#endif
