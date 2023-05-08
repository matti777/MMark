#ifndef __MMARKTIZENCONTROLLER_H__
#define __MMARKTIZENCONTROLLER_H__

#include <FBase.h>
#include <FNet.h>
#include <FGrpIGlRenderer.h>
#include <FUi.h>

#include "MMarkController.h"

/**
 * Tizen specific implementations of the controller features.
 *
 * @author Matti Dahlbom
 * @since 1.0.3
 */
class MMarkTizenController : public MMarkController,
    public Tizen::Net::Http::IHttpTransactionEventListener,
    public Tizen::Ui::ITouchEventListener
{
public: // Constructors and destructor
    MMarkTizenController();
    virtual ~MMarkTizenController();

public: // From GLController
    virtual void SetPaused(bool paused);
    virtual void Redraw();

public: // From ITouchEventListener
    void OnTouchPressed(const Tizen::Ui::Control& source,
                        const Tizen::Graphics::Point& currentPosition,
                        const Tizen::Ui::TouchEventInfo & touchInfo);
    void OnTouchReleased(const Tizen::Ui::Control& source,
                         const Tizen::Graphics::Point& currentPosition,
                         const Tizen::Ui::TouchEventInfo& touchInfo);
    void OnTouchMoved(const Tizen::Ui::Control& source,
                      const Tizen::Graphics::Point& currentPosition,
                      const Tizen::Ui::TouchEventInfo& touchInfo);
    void OnTouchCanceled(const Tizen::Ui::Control& source,
                         const Tizen::Graphics::Point& currentPosition,
                         const Tizen::Ui::TouchEventInfo& touchInfo);
    void OnTouchFocusIn(const Tizen::Ui::Control& /*source*/,
                        const Tizen::Graphics::Point& /*currentPosition*/,
                        const Tizen::Ui::TouchEventInfo& /*touchInfo*/) {}
    void OnTouchFocusOut(const Tizen::Ui::Control& /*source*/,
                         const Tizen::Graphics::Point& /*currentPosition*/,
                         const Tizen::Ui::TouchEventInfo& /*touchInfo*/) {}

public: // From IHttpTransactionEventListener
    void OnTransactionCompleted(Tizen::Net::Http::HttpSession& httpSession,
                                Tizen::Net::Http::HttpTransaction& httpTransaction);
    void OnTransactionReadyToWrite(Tizen::Net::Http::HttpSession &httpSession,
                                   Tizen::Net::Http::HttpTransaction &httpTransaction,
                                   int recommendedChunkSize);
    void OnTransactionReadyToRead(Tizen::Net::Http::HttpSession& httpSession,
                                  Tizen::Net::Http::HttpTransaction& httpTransaction,
                                  int availableBodyLen);
    void OnTransactionHeaderCompleted(Tizen::Net::Http::HttpSession &httpSession,
                                      Tizen::Net::Http::HttpTransaction &httpTransaction,
                                      int headerLen, bool bAuthRequired);
    void OnTransactionCertVerificationRequiredN(Tizen::Net::Http::HttpSession &httpSession,
                                                Tizen::Net::Http::HttpTransaction &httpTransaction,
                                                Tizen::Base::String *pCert);
    void OnTransactionAborted(Tizen::Net::Http::HttpSession &httpSession,
                              Tizen::Net::Http::HttpTransaction &httpTransaction,
                               result r);

protected: // From MMarkController
    void OpenInBrowser(std::string url) const;
    void ShowMessage(std::string msg) const;
    void SubmitScore(const std::string& json,
                     const std::string& signature);
    DeviceInfo GetDeviceInfo() const;

private: // Data
    // Number of active touches
    int m_touchCount;
};

#endif
