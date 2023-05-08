#include <FBase.h>
#include <FSystem.h>
#include <FApp.h>
#include <FNet.h>

#include "MMarkTizenController.h"
#include "CommonFunctions.h"

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::System;
using namespace Tizen::Net::Http;
using namespace Tizen::Base::Utility;

MMarkTizenController::MMarkTizenController()
: MMarkController(),
  m_touchCount(0)
{
    LOG_DEBUG("Constructing MMarkTizenController()");
}

MMarkTizenController::~MMarkTizenController()
{
    LOG_DEBUG("Destroying MMarkTizenController()");
}

void MMarkTizenController::SetPaused(bool paused)
{
    //TODO pause the timer in the player
}

void MMarkTizenController::Redraw()
{
    // No action
}

void MMarkTizenController::OpenInBrowser(std::string url) const
{
    String uri(url.data());
    String app(L"tizen.internet");
    String operation("http://tizen.org/appcontrol/operation/view");

    AppControl* pAc = AppManager::FindAppControlN(app, operation);

    if ( pAc != NULL )
    {
       pAc->Start(&uri, null, null, null);
       delete pAc;
    }
    else
    {
	LOG_INFO("Failed to open the URL in browser!");
    }
}

void MMarkTizenController::ShowMessage(std::string) const
{
    // No implementation
}

void MMarkTizenController::OnTransactionCompleted(HttpSession&,
                                                  HttpTransaction& httpTransaction) {
    int status = httpTransaction.GetResponse()->GetHttpStatusCode();
    LOG_DEBUG("OnTransactionCompleted(): status: %d", status);
    ByteBuffer* responseBody = httpTransaction.GetResponse()->ReadBodyN();
    std::string responseString;
    if ( responseBody != NULL )
    {
	responseString = (const char*)responseBody->GetPointer();
	delete responseBody;
    }
    ScoreSubmitted((status == 200), responseString);
}

void MMarkTizenController::OnTransactionReadyToWrite(Tizen::Net::Http::HttpSession &httpSession,
                               Tizen::Net::Http::HttpTransaction &httpTransaction,
                               int recommendedChunkSize)
{
    AppLogDebug("OnTransactionReadyToWrite()");
    // No implementation
}

void MMarkTizenController::OnTransactionReadyToRead(Tizen::Net::Http::HttpSession &httpSession,
                              Tizen::Net::Http::HttpTransaction &httpTransaction,
                              int availableBodyLen)
{
    AppLogDebug("OnTransactionReadyToRead()");
    // No implementation
}

void MMarkTizenController::OnTransactionHeaderCompleted(Tizen::Net::Http::HttpSession &httpSession,
                                  Tizen::Net::Http::HttpTransaction &httpTransaction,
                                  int headerLen, bool bAuthRequired)
{
    AppLogDebug("OnTransactionHeaderCompleted()");
    // No implementation
}

void MMarkTizenController::OnTransactionCertVerificationRequiredN(Tizen::Net::Http::HttpSession &httpSession,
                                            Tizen::Net::Http::HttpTransaction &httpTransaction,
                                            Tizen::Base::String *pCert)
{
    AppLogDebug("OnTransactionHeaderCompleted()");
    // No implementation
}

void MMarkTizenController::OnTransactionAborted(Tizen::Net::Http::HttpSession &httpSession,
                          Tizen::Net::Http::HttpTransaction &httpTransaction,
                           result r)
{
    /*
     * E_INVALID_ARG	A specified input parameter is invalid.
E_OUT_OF_MEMORY	The memory is insufficient.
E_IO	The method has failed to read the data.
E_TIMEOUT	An attempt to connect to the server has timed out.
E_NETWORK_UNAVAILABLE	The network is unavailable.
E_HOST_UNREACHABLE	The network cannot be reached from this host at this time.
E_SYSTEM	An internal error has occurred.
E_UNKNOWN	An unknown error has occurred.
E_NOT_RESPONDING	There is no response.
E_INVALID_CONTENT	The content is invalid.
E_CONNECTION_RESET	The network connection has been reset.
E_HTTP_USER	The HTTP user is canceled.
E_NO_CERTIFICATE	The client certificate is required to connect to the server.
E_UNSUPPORTED_SERVICE	The service is not allowed.
E_USER_AGENT_NOT_ALLOWED	The user agent is not allowed.
E_RESOURCE_UNAVAILABLE	The network resource is unavailable.
     */
    switch ( r )
    {
    case E_INVALID_ARG:
	AppLogDebug("E_INVALID_ARG");
	break;
    case E_OUT_OF_MEMORY:
	AppLogDebug("E_OUT_OF_MEMORY");
	break;
    case E_IO:
	AppLogDebug("E_IO");
	break;
    case E_TIMEOUT:
	AppLogDebug("E_TIMEOUT");
	break;
    case E_NETWORK_UNAVAILABLE:
	AppLogDebug("E_NETWORK_UNAVAILABLE");
	break;
    case E_HOST_UNREACHABLE:
	AppLogDebug("E_HOST_UNREACHABLE");
	break;
    case E_SYSTEM:
	AppLogDebug("E_SYSTEM");
	break;
    case E_UNKNOWN:
	AppLogDebug("E_UNKNOWN");
	break;
    case E_NOT_RESPONDING:
	AppLogDebug("E_NOT_RESPONDING");
	break;
    case E_INVALID_CONTENT:
	AppLogDebug("E_INVALID_CONTENT");
	break;
    case E_CONNECTION_RESET:
	AppLogDebug("E_CONNECTION_RESET");
	break;
    case E_HTTP_USER:
	AppLogDebug("E_HTTP_USER");
	break;
    case E_NO_CERTIFICATE:
	AppLogDebug("E_NO_CERTIFICATE");
	break;
    case E_UNSUPPORTED_SERVICE:
	AppLogDebug("E_UNSUPPORTED_SERVICE");
	break;
    case E_USER_AGENT_NOT_ALLOWED:
	AppLogDebug("E_USER_AGENT_NOT_ALLOWED");
	break;
    case E_RESOURCE_UNAVAILABLE:
	AppLogDebug("E_RESOURCE_UNAVAILABLE");
	break;
    }

    AppLogDebug("OnTransactionAborted(), r: %ul", r);
    ScoreSubmitted(false, std::string());
}

void MMarkTizenController::SubmitScore(const std::string& json,
	const std::string& signature)
{
    result r = E_SUCCESS;
    //TODO make these class members?
    HttpSession* pSession = null;
    HttpTransaction* pTransaction = null;
    String hostAddr(BASE_URL);

    pSession = new HttpSession();
    r = pSession->Construct(NET_HTTP_SESSION_MODE_MULTIPLE_HOST/*NET_HTTP_SESSION_MODE_NORMAL*/,
                            NULL, hostAddr, NULL, NET_HTTP_COOKIE_FLAG_ALWAYS_AUTOMATIC);
    if ( IsFailed(r) )
    {
	AppLogDebug("Failed to Construct HttpSession");
    }

    pTransaction = pSession->OpenTransactionN();
    pTransaction->AddHttpTransactionListener(*this);
    HttpRequest* pRequest = pTransaction->GetRequest();
    pRequest->SetMethod(NET_HTTP_METHOD_POST);
    pRequest->SetUri(ScoreSubmitURL);
    AppLogDebug("Submitting to URL: %s", ScoreSubmitURL);

    // Set the POST body (json)
    ByteBuffer bodyBuffer;
    r = bodyBuffer.Construct((const byte*)json.data(), 0,
                             json.length(), json.length());
    if ( IsFailed(r) )
    {
	LOG_INFO("Failed to construct body buffer: %d", r);
	ScoreSubmitted(false, std::string());
	return;
    }

    r = pRequest->WriteBody(bodyBuffer);
    if ( IsFailed(r) )
    {
	LOG_INFO("Failed to write request body: %d", r);
	ScoreSubmitted(false, std::string());
	return;
    }

    // Add the Content-Length / Content-Type headers
    HttpHeader* pHeader = pRequest->GetHeader();
    String lengthString;
    lengthString.Append((int)json.length());
    r = pHeader->AddField(L"Content-Length", lengthString);
    r = pHeader->AddField(L"Content-Type", L"application/json; charset=utf-8");

    // Add the signature header
    r = pHeader->AddField(String(SignatureHeader), String(signature.data()));

    String concatenated = String(JsonApiUsername) + ":" +
	    String(JsonApiPassword);
    ByteBuffer* utf8Buffer = StringUtil::StringToUtf8N(concatenated);

    // Remove the null terminator from the end
    utf8Buffer->ShiftLimit(-1);

    String b64String;
    r = StringUtil::EncodeToBase64String(*utf8Buffer, b64String);
    delete utf8Buffer;
    if ( IsFailed(r) )
    {
	LOG_INFO("Failed to Base64 encode: %d", r);
	ScoreSubmitted(false, std::string());
	return;
    }

    String authHeader = String("Basic ") + b64String;
    r = pHeader->AddField(L"Authorization", authHeader);

    AppLogDebug("calling pTransaction->Submit()!");
    pTransaction->Submit();
}

std::string StringToStdString(String s)
{
    ByteBuffer* buf = StringUtil::StringToUtf8N(s);
    std::string stdstr((const char*)buf->GetPointer());
    delete buf;
    return stdstr;
}

/*
std::string ReadCpuinfo()
{
    QFile file("/proc/cpuinfo");
    std::string cpuType;

    if ( file.exists() )
    {
        if ( file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QTextStream in(&file);
            QString line = in.readLine();
            while ( !line.isNull() ) {
                QStringList s = line.split(":");
                QString key = s[0].trimmed().toLower();
                QString value = s[1].trimmed();

                if ( (key == "model name") || (key == "processor") )
                {
                    if ( value.length() > 2 )
                    {
                        cpuType = value.toStdString();
                        break;
                    }
                }

                // Read next line
                line = in.readLine();
            }
            file.close();
        }
    }

    return cpuType;
}

int ReadMemTotal()
{
    QFile file("/proc/meminfo");

    if ( file.exists() )
    {
        if ( file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QTextStream in(&file);
            QString line = in.readLine();
            while ( !line.isNull() ) {
                QStringList s = line.split(":");
                QString key = s[0].trimmed().toLower();
                QString value = s[1].trimmed();

                if ( key == "memtotal" )
                {
                    QStringList values = value.split(" ");
                    if ( values.count() == 2 ) {
                        return values[0].toInt();
                    }
                }

                // Read next line
                line = in.readLine();
            }
            file.close();
        }
    }

    LOG_DEBUG("MemTotal: could not be read!");
    return 0;
}

int ReadCpuMaxFreq()
{
    QFile file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    int maxFreq = 0;

    if ( file.exists() )
    {
        if ( file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QString line = file.readLine().trimmed();
            maxFreq = line.toInt();
            maxFreq /= 1000; // Convert into MHz
            file.close();
        }
    }

    return maxFreq;
}
 */

DeviceInfo MMarkTizenController::GetDeviceInfo() const
{
    DeviceInfo info;

    String platformVersion;
    SystemInfo::GetPlatformVersion(platformVersion);
    AppLogDebug("platformVersion = %ls", platformVersion.GetPointer());

    info.m_osVersion = StringToStdString(platformVersion);
    String cpu;
    SystemInfo::GetValue("http://tizen.org/system/platform.processor", cpu);
    AppLogDebug("cpu = %ls", cpu.GetPointer());
    info.m_cpuType = StringToStdString(cpu);

    String model;
    SystemInfo::GetValue("http://tizen.org/system/model_name", model);
    AppLogDebug("model = %ls", model.GetPointer());
    info.m_model = StringToStdString(model);

    String platformName;
    SystemInfo::GetValue("http://tizen.org/system/platform.name", platformName);
    info.m_productName = StringToStdString(platformName);

    String buildInfo;
    SystemInfo::GetBuildInfo(buildInfo);
    AppLogDebug("buildInfo = %ls", buildInfo.GetPointer());

    info.m_numCpuCores = sysconf(_SC_NPROCESSORS_ONLN);

    //TODO get this from screen size like with Qt?
    info.m_deviceType = "mobilephone";

    long long ram = 0;
    RuntimeInfo::GetValue("http://tizen.org/runtime/memory.available", ram);
    ram /= 1024; // into kB
    info.m_totalRam = (int)ram;

    //TODO
    info.m_cpuFrequency = 0;
    info.m_manufacturer = "Samsung";

    return info;
}

void MMarkTizenController::OnTouchPressed(const Tizen::Ui::Control& source,
                    const Tizen::Graphics::Point& currentPosition,
                    const Tizen::Ui::TouchEventInfo & touchInfo)
{
    m_touchCount++;
    LOG_DEBUG("m_touchCount = %d", m_touchCount);
    TouchStarted(&touchInfo, currentPosition.x, currentPosition.y);

    if ( m_touchCount >= 3 )
    {
	TripleTouch();
    }
}

void MMarkTizenController::OnTouchReleased(const Tizen::Ui::Control& source,
                     const Tizen::Graphics::Point& currentPosition,
                     const Tizen::Ui::TouchEventInfo& touchInfo)
{
    m_touchCount--;
    TouchEnded(&touchInfo, currentPosition.x, currentPosition.y);
}

void MMarkTizenController::OnTouchMoved(const Tizen::Ui::Control& source,
                  const Tizen::Graphics::Point& currentPosition,
                  const Tizen::Ui::TouchEventInfo& touchInfo)
{
    TouchMoved(&touchInfo, currentPosition.x, currentPosition.y);
}

void MMarkTizenController::OnTouchCanceled(const Tizen::Ui::Control& source,
                     const Tizen::Graphics::Point& currentPosition,
                     const Tizen::Ui::TouchEventInfo& touchInfo)
{
    m_touchCount--;
    TouchEnded(&touchInfo, currentPosition.x, currentPosition.y);
}

