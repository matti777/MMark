#include "mainwindow.h"
#include "GLWidget.h"

#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication::setGraphicsSystem("opengl");
    QApplication app(argc, argv);

#ifdef __BUILD_DESKTOP__
    MainWindow mainWindow;
    mainWindow.setOrientation(MainWindow::ScreenOrientationLockLandscape);
    mainWindow.showExpanded();
#else
    GLWidget widget(NULL);
    widget.showFullScreen();
#endif

    /*
To lock the app into landscape on android:

Go to Android directory (in your project),open AndroidManifest.xml and
add next information into activity tag:
<activity android:name=”org.kde.necessitas.origo.QtActivity”
android:configChanges=”orientation|locale|fontScale|keyboard|keyboardHidden”
android:label=”@string/app_name” android:screenOrientation=”portrait” >
*/

    return app.exec();
}
