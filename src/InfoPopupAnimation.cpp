#include "InfoPopupAnimation.h"

InfoPopupAnimation::InfoPopupAnimation(float initialDelay, float transitionDuration, 
                                       float displayDuration, 
                                       float hideHeight, float displayHeight, float* height)
    : BaseAnimation(initialDelay, transitionDuration * 2 + displayDuration),
      m_appearAnimation(hideHeight, displayHeight, initialDelay, 
                        transitionDuration, height),
      m_disappearAnimation(displayHeight, hideHeight, 
                           initialDelay + transitionDuration + displayDuration, 
                           transitionDuration, height),
      m_appearCompleted(false)
{
}

InfoPopupAnimation::~InfoPopupAnimation()
{
}

bool InfoPopupAnimation::Animate(const TimeSample& time)
{
    if ( !m_appearCompleted ) 
    {
        m_appearCompleted = m_appearAnimation.Animate(time);
        return m_appearCompleted;
    }
    else
    {
        return m_disappearAnimation.Animate(time);
    }
}

bool InfoPopupAnimation::HasCompleted(const TimeSample& time) const
{
    return ( m_appearCompleted && m_disappearAnimation.HasCompleted(time) );
}

void InfoPopupAnimation::UpdateHeights(float hideHeight, float displayHeight)
{
    m_appearAnimation.UpdateValues(hideHeight, displayHeight);
    m_disappearAnimation.UpdateValues(displayHeight, hideHeight);
}

void InfoPopupAnimation::ResetTime()
{
    BaseAnimation::ResetTime();
    m_appearAnimation.ResetTime();
    m_disappearAnimation.ResetTime();
}

