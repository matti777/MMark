#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QNetworkAccessManager>

// include OpenGL API
#include "OpenGLAPI.h"

#include <QTime>
#include <QGLWidget>
#include <QKeyEvent>

#include "MMarkController.h"

/**
 * This class inherits from QGLWidget to provide the Qt platform support for
 * system specific APIs.
 *
 * @author Matti Dahlbom
 * @since 0.1
 */
class GLWidget : public QGLWidget, MMarkController
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent);
    virtual ~GLWidget();

protected:
    /** Request a redraw. */
    virtual void Redraw();

    /** Pauses or resumes the rendering timer. */
    virtual void SetPaused(bool paused);

    // From QGLWidget
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    // From QWidget
#ifdef __BUILD_DESKTOP__
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
#endif
    bool event(QEvent* event);

protected: // From MMarkController
    virtual void OpenInBrowser(std::string url) const;
    virtual void ShowMessage(std::string msg) const;
    virtual void SubmitScore(const std::string& json,
                             const std::string& signature);
    void ReadDisplayInfo(DeviceInfo& info) const;
    virtual DeviceInfo GetDeviceInfo() const;
//    virtual std::string QueryUserName();

private:
    void Quit();

private slots:
    void HttpResult(QNetworkReply* reply);

private: // Data
    // Timer for drawing
    QTimer* m_timer;

    // Time of the previous left mouse button press - used for detecting
    // "tap" clicks
    QTime m_lastLeftMouseDown;
    int m_lastLeftMouseX;
    int m_lastLeftMouseY;

    // HTTP
    QNetworkAccessManager* m_networkManager;
};

#endif // GLWIDGET_H
