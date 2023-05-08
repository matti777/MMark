# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

# Smart Installer package's UID
# This UID is from the protected range 
# and therefore the package will fail to install if self-signed
# By default qmake uses the unprotected range value if unprotected UID is defined for the application
# and 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
CONFIG += mobility
MOBILITY +=

# Emit "DEBUG" preprocessor macro for debug builds
debug:DEFINES += DEBUG

QT += opengl network

# include the Bullet Physics engine
include(../BulletPhysics_2.80/BulletPhysics.pri)

# include the jsoncpp JSON library
include(../jsoncpp/jsoncpp.pri)

#QMAKE_CXXFLAGS += -Werror -Wall

blackberry-* {
     DEFINES += __BLACKBERRY__
     DEFINES += __BUILD_DEVICE__
     message(Blackberry build)
} else {
     # Other Qt devices are considered Linux-based
     DEFINES += __LINUXBASED_PLATFORM__

     # Others will also use systeminfo module
     MOBILITY += systeminfo
}

win32|linux-g++|linux-g++-64 {
     LIBS += -lGLEW
     DEFINES += __BUILD_DESKTOP__
     message(Desktop build)
} android {
     # Add android (Necessitas) stuff here
     DEFINES += __BUILD_DEVICE__
     DEFINES += __ANDROID__
     message(Android Device build)
} else {
#     DEFINES += __BUILD_DEVICE__
#     message(Device build)
}

DEPENDPATH += . include ../include ../../../CommonGL/include/
DEPENDPATH += ./resources/shaders ./resources/textures
INCLUDEPATH += . include ../include ../md5 ../../../CommonGL/include/

SOURCES += src/main.cpp src/mainwindow.cpp \
           ../../../CommonGL/src/MatrixOperations.cpp \
           ../../../CommonGL/src/GLController.cpp \
    ../../../CommonGL/src/Camera.cpp \
    src/GLWidget.cpp \
    ../src/Chessboard.cpp \
    ../src/MMarkController.cpp \
    ../src/BaseStage.cpp \
    ../src/ChessboardStage.cpp \
    ../../../CommonGL/src/CommonFunctions.cpp \
    ../../../CommonGL/src/CommonFunctionsQT.cpp \
    ../src/Chesspiece.cpp \
    ../src/ChesspieceInstance.cpp \
    ../src/Skybox.cpp \
    ../../../CommonGL/src/BaseAnimation.cpp \
    ../../../CommonGL/src/TranslationAnimation.cpp \
    ../src/ChesspieceAnimation.cpp \
    ../../../CommonGL/src/ScalarAnimation.cpp \
    ../../../CommonGL/src/TextRenderer.cpp \
    ../src/InfoPopupAnimation.cpp \
    ../src/FractalStage.cpp \
    ../src/PhysicsStageStatics.cpp \
    ../src/PhysicsStage.cpp \
    ../src/Pillar.cpp \
    ../../../CommonGL/src/ObjectMotionState.cpp \
    ../../../CommonGL/src/TimeSample.cpp \
    ../src/ObjectInstance.cpp \
    ../../../CommonGL/src/Rect.cpp \
    ../src/Vehicle.cpp \
    ../../../CommonGL/src/RotationAnimation.cpp \
    ../../../CommonGL/src/SimpleTimer.cpp \
    ../../../CommonGL/src/BSplineAnimation.cpp \
    ../../../CommonGL/src/SplineCameraPathAnimation.cpp \
    ../src/FillrateStage.cpp \
    ../../../CommonGL/src/Torus.cpp \
    ../src/EllipticPathAnimation.cpp \
    ../src/ScoreTextRenderer.cpp \
    ../md5/md5.cpp \
    ../src/ChessboardDemoMode.cpp \
    ../../../CommonGL/src/FpsMeter.cpp \
    ../../../CommonGL/src/BaseWidget.cpp \
    ../../../CommonGL/src/Button.cpp \
    ../../../CommonGL/src/Container.cpp
HEADERS += include/mainwindow.h \
           ../../CommonGL/include/MatrixOperations.h \
           ../../CommonGL/include/GLController.h \
    ../../../CommonGL/include/Rect.h \
    ../../../CommonGL/include/OpenGLAPI.h \
    ../../../CommonGL/include/Camera.h \
    include/GLWidget.h \
    ../include/Chessboard.h \
    ../include/MMarkController.h \
    ../include/BaseStage.h \
    ../include/ChessboardStage.h \
    ../../../CommonGL/include/CommonFunctions.h \
    ../include/Chesspiece.h \
    ../include/Skybox.h \
    ../include/ChesspieceInstance.h \
    ../../../CommonGL/include/BaseAnimation.h \
    ../../../CommonGL/include/TranslationAnimation.h \
    ../include/ChesspieceAnimation.h \
    ../../../CommonGL/include/ScalarAnimation.h \
    ../../../CommonGL/include/TextRenderer.h \
    ../include/InfoPopupAnimation.h \
    ../FractalStage.h \
    ../include/FractalStage.h \
    ../include/PhysicsStageStatics.h \
    ../include/PhysicsStage.h \
    ../include/Pillar.h \
    ../../../CommonGL/include/ObjectMotionState.h \
    ../../../CommonGL/include/TimeSample.h \
    ../include/ObjectInstance.h \
    ../include/Vehicle.h \
    ../../../CommonGL/include/RotationAnimation.h \
    ../../../CommonGL/include/SimpleTimer.h \
    ../../../CommonGL/include/BSplineAnimation.h \
    ../../../CommonGL/include/SplineCameraPathAnimation.h \
    ../include/DeviceInfo.h \
    ../include/FillrateStage.h \
    ../../../CommonGL/include/Torus.h \
    ../include/EllipticPathAnimation.h \
    ../include/ScoreTextRenderer.h \
    ../md5/md5.h \
    ../include/ChessboardDemoMode.h \
    ../../../CommonGL/include/FpsMeter.h \
    ../../../CommonGL/include/BaseWidget.h \
    ../../../CommonGL/include/Button.h \
    ../../../CommonGL/include/Container.h
FORMS += mainwindow.ui

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

# Symbian build specific things
symbian {
    ICON = mmark.svg

        #DEPLOYMENT.installer_header = 0x2002CCCF
    TARGET.UID3 = 0xkalakala
    TARGET.CAPABILITY += NetworkServices ReadDeviceData

    # Bigger heap and stack are needed for Bullet and graphics
    TARGET.EPOCHEAPSIZE = 0x100000 0x2000000
    TARGET.EPOCSTACKSIZE = 0x14000

    # Enable hardware floats
    MMP_RULES += "OPTION gcce -march=armv6"
    MMP_RULES += "OPTION gcce -mfpu=vfp"
    MMP_RULES += "OPTION gcce -mfloat-abi=softfp"
    MMP_RULES += "OPTION gcce -marm"
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    ../shaders/Chessboard.fsh \
    ../shaders/Chessboard.vsh \
    ../shaders/Chesspiece.vsh \
    ../shaders/Chesspiece.fsh \
    ../shaders/Skybox.vsh \
    ../shaders/Skybox.fsh \
    ../shaders/ChesspieceReflection.vsh \
    ../shaders/ChesspieceReflection.fsh \
    ../shaders/SimpleColor.vsh \
    ../shaders/SimpleColor.fsh \
    ../shaders/SimpleTexture.vsh \
    ../shaders/SimpleTexture.fsh \
    ../shaders/PhysicsStageDefault.vsh \
    ../shaders/PhysicsStageDefault.fsh \
    ../shaders/Terrain.vsh \
    ../shaders/Terrain.fsh \
    ../shaders/ShadowMap.fsh \
    ../shaders/ShadowMap.vsh \
    ../shaders/ShadowMapTransparent.fsh \
    ../shaders/ShadowMapTransparent.vsh \
    ../shaders/Vehicle.fsh \
    ../shaders/Vehicle.vsh \
    ../shaders/VertexLight.vsh \
    ../shaders/VertexLight.fsh \
    ../shaders/PixelLight.fsh \
    ../shaders/PixelLight.vsh \
    ../shaders/CombineBlur.fsh \
    ../shaders/CombineBlur.vsh \
    ../shaders/YBlur.vsh \
    ../shaders/YBlur.fsh \
    ../shaders/XBlur.vsh \
    ../shaders/XBlur.fsh \
    ../shaders/MappedLight.vsh \
    ../shaders/MappedLight.fsh \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/AndroidManifest.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-it/strings.xml \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-ro/strings.xml \
    android/res/drawable-mdpi/icon.png \
    android/res/values-id/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-rs/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/values-fr/strings.xml \
    android/version.xml \
    ../shaders/ImageWidget.vsh \
    ../shaders/ImageWidget.fsh \
    blackberry/bar-descriptor.xml

RESOURCES += \
    resources/shaders.qrc \
    resources/textures.qrc
