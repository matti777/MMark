#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QFile>

#include <QInputDialog>
#include <QMessageBox>

#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QApplication>
#include <QDesktopWidget>

#if defined(__BUILD_DEVICE__) && !defined(__BLACKBERRY__)
#include <QSystemDeviceInfo>
#include <QSystemInfo>
#endif

#include <math.h>

#include "GLWidget.h"
#include "CommonFunctions.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent),
    m_timer(NULL)
{
    LOG_DEBUG("GLWidget::GLWidget()");

    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAutoBufferSwap(false);

    // Enable grabbing key events
    setFocusPolicy(Qt::StrongFocus);

    setSizePolicy(QSizePolicy::MinimumExpanding,
                  QSizePolicy::MinimumExpanding);

    // Force alpha channel. This is required for glReadPixels() to return
    // proper alpha values
    QGLFormat fmt = format();
    fmt.setAlpha(true);
    setFormat(fmt);

    m_networkManager = new QNetworkAccessManager(this);

    // Callbacks from the HTTP interface
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(HttpResult(QNetworkReply*)));
}

GLWidget::~GLWidget()
{
    delete m_networkManager;

    // Stop the timer
    if ( m_timer != NULL )
    {
        m_timer->stop();
        delete m_timer;
    }
}

void GLWidget::SetPaused(bool paused)
{
    if ( paused )
    {
        m_timer->stop();
    }
    else
    {
        m_timer->start();
    }
}

void GLWidget::Redraw()
{
    updateGL();
}

void GLWidget::Quit()
{
    if ( m_timer != NULL )
    {
        m_timer->stop();
    }
    QCoreApplication::quit();
}

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

void GLWidget::ReadDisplayInfo(DeviceInfo& info) const
{
    // Calculate the physical screen size and use it to determine device type
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    float displayDiagonalInch = 0;
    int dpiX = QApplication::desktop()->physicalDpiX();
    int dpiY = QApplication::desktop()->physicalDpiY();
    if ( (dpiX > 0) && (dpiY > 0) )
    {
        float displayWidthInch = screenGeometry.width() / dpiX;
        float displayHeightInch = screenGeometry.height() / dpiY;
        displayDiagonalInch = sqrt(displayWidthInch * displayWidthInch +
                                   displayHeightInch * displayHeightInch);
    }
    LOG_DEBUG("Display size: %f inches", displayDiagonalInch);

    if ( (displayDiagonalInch <= 1.0) || (displayDiagonalInch >= 15.0) )
    {
        // In case DPI / resolution not available for some reason
        info.m_deviceType = "other";
    }
    else if ( displayDiagonalInch <= 6.3 )
    {
        info.m_deviceType = "mobilephone";
    }
    else if ( displayDiagonalInch <= 8.0 )
    {
        info.m_deviceType = "minitablet";
    }
    else
    {
        info.m_deviceType = "tablet";
    }
}

#ifdef __BLACKBERRY__
DeviceInfo GLWidget::GetDeviceInfo() const
{
    DeviceInfo info = DeviceInfo();

    info.m_numCpuCores = 0; //TODO
    // Check this http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine

    // General device info
    info.m_manufacturer = "Blackberry";
    info.m_model = ""; //TODO
    info.m_productName = ""; //TODO
    info.m_osVersion = "10"; //TODO

    // Populate display related information
    ReadDisplayInfo(info);

    info.m_cpuType = ""; //TODO
    info.m_cpuFrequency = 0; //TODO
    info.m_totalRam = 0; //TODO

    return info;
}

#endif

#ifdef __LINUXBASED_PLATFORM__
DeviceInfo GLWidget::GetDeviceInfo() const
{
    DeviceInfo info = DeviceInfo();

    // Get the number of CPU cores
    info.m_numCpuCores = sysconf(_SC_NPROCESSORS_ONLN);

#ifdef __BUILD_DEVICE__
    QtMobility::QSystemDeviceInfo deviceinfo;
    QtMobility::QSystemInfo systeminfo;
    info.m_manufacturer = deviceinfo.manufacturer().toStdString();
    info.m_model = deviceinfo.model().toStdString();
    info.m_productName = deviceinfo.productName().toStdString();
    info.m_osVersion =
            systeminfo.version(QtMobility::QSystemInfo::Os).toStdString();
#endif

    // Populate display related information
    ReadDisplayInfo(info);

    // Read processor info from /proc/cpuinfo if it exists
    info.m_cpuType = ReadCpuinfo();

    // Read CPU max frequency if the file exists
    info.m_cpuFrequency = ReadCpuMaxFreq();

    // Get the amount of physical RAM
    info.m_totalRam = ReadMemTotal();

    return info;
}
#endif

void GLWidget::OpenInBrowser(std::string url) const
{
    QDesktopServices::openUrl(QUrl(QString::fromStdString(url)));
}

void GLWidget::ShowMessage(std::string /*msg*/) const
{
//    QMessageBox::critical(const_cast<GLWidget*>(this),
//                          "Error", QString::fromStdString(msg));
}

void GLWidget::HttpResult(QNetworkReply* reply)
{
    LOG_DEBUG("GLWidget::HttpResult()");

    if ( reply->error() == QNetworkReply::NoError )
    {
       LOG_DEBUG("Request successful");
        QString data = (QString) reply->readAll();
        ScoreSubmitted(true, data.toStdString());
    }
    else
    {
        LOG_DEBUG("Error in request: %d: %s", reply->error(),
                  reply->errorString().toUtf8().data());

        // Try to handle the error the best we can
//        QString errorMsg("Check your network connection.");

//        if ( (reply->error() == QNetworkReply::ContentNotFoundError) ||
//             (reply->error() == QNetworkReply::AuthenticationRequiredError) ||
//             (reply->error() == QNetworkReply::ContentAccessDenied) ||
//             (reply->error() == QNetworkReply::UnknownContentError) ||
//             (reply->error() == QNetworkReply::ProtocolFailure) )
//        {
//            errorMsg = "You might be using an old version of the software.";
//        }

//        QMessageBox::critical(this, "Submission failed", errorMsg);

        ScoreSubmitted(false, std::string());
    }
}

void GLWidget::SubmitScore(const std::string& json,
                           const std::string& signature)
{
    QString body = QString::fromUtf8(json.c_str());
    QByteArray data = body.toUtf8();
    QNetworkRequest request;
    request.setUrl(QUrl(ScoreSubmitURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json; charset=utf-8");
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    request.setRawHeader(QString(SignatureHeader).toUtf8(),
                         QString::fromUtf8(signature.c_str()).toUtf8());

    // Add basic auth as raw header to skip the authentication callback step
    QString concatenated = QString(JsonApiUsername) + ":" +
                           QString(JsonApiPassword);
    QByteArray authData = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + authData;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    m_networkManager->post(request, data);
}

void GLWidget::paintGL()
{
    // call superclass to draw the world
    if ( !Draw() )
    {
        LOG_DEBUG("Draw() failed, quitting application.");
        Quit();
        return;
    }

    // make the drawn stuff visible
    swapBuffers();
}

void GLWidget::initializeGL()
{
    LOG_DEBUG("GLWidget::initializeGL()");

#ifdef __BUILD_DESKTOP__
    // init GLew extension library
    GLenum err = glewInit();
    if ( err != GLEW_OK )
    {
        qWarning() << "glewInit() failed: '" << glewGetErrorString(err)
                   << "', exiting..";
        Quit();
        return;
    }
    else
    {
        qDebug() << "glewInit() OK!";
    }
#endif

//    LOG_DEBUG("extensions = %s", glGetString(GL_EXTENSIONS));

    // initialize the parent controller
    if ( !InitController() )
    {
        qWarning() << "InitController() failed, exiting..";
        Quit();
        return;
    }

    // Create a timer and connect it to the rendering method
    m_timer = new QTimer();
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    m_timer->start(16);
}

void GLWidget::resizeGL(int width, int height)
{
    // call superclass
    ViewportResized(width, height);
}

bool GLWidget::event(QEvent* event)
{
    switch ( event->type() )
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        if ( touchEvent->touchPoints().size() >= 3 )
        {
            // Send notification about 3+ fingers touch
            TripleTouch();
        }

        // Process all the touch points of this touch event
        for ( int i = 0; i < touchEvent->touchPoints().size(); i++ )
        {
            QTouchEvent::TouchPoint touchPoint = touchEvent->touchPoints()[0];
            int x = floor(touchPoint.pos().x());
            int y = floor(touchPoint.pos().y());

            switch ( touchPoint.state() )
            {
            case Qt::TouchPointPressed:
                TouchStarted((void*)((size_t)touchPoint.id()), x, y);
                break;
            case Qt::TouchPointMoved:
                TouchMoved((void*)((size_t)touchPoint.id()), x, y);
                break;
            case Qt::TouchPointReleased:
                TouchEnded((void*)((size_t)touchPoint.id()), x, y);
                break;
            default:
                // No action!
                break;
            }
        }

        return true;
        break;
    }
    default:
        return QGLWidget::event(event);
        break;
    }
}

// Mouse/keyboard event handling only present in desktop builds!
#ifdef __BUILD_DESKTOP__
void GLWidget::keyPressEvent(QKeyEvent* event)
{
    if ( event->key() == Qt::Key_Escape )
    {
        QCoreApplication::quit();
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    if ( event->buttons() & Qt::LeftButton )
    {
        m_lastLeftMouseDown.start();
        m_lastLeftMouseX = event->x();
        m_lastLeftMouseY = event->y();
        TouchStarted(event, event->x(), event->y());

        // Also simulate triple touch every time
        LOG_DEBUG("Sending simulated TripleTouch()");
        TripleTouch();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    TouchEnded(event, m_lastLeftMouseX, m_lastLeftMouseY);
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if ( event->buttons() & Qt::LeftButton )
    {
        TouchMoved(event, event->x(), event->y());
    }
}
#endif // #ifdef __BUILD_DESKTOP__

