# The Alexa SDK must be built before building this plugin
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0
PKGCONFIG += glib-2.0

packagesExist(AlexaClientSDK) {
    PKGCONFIG += AlexaClientSDK
}

# Use this only if SDK is built with KWD support
USE_WAKEUP_WORD = 1

# Specify if the libraries headers use the old or new ABI
DEFINES += _GLIBCXX_USE_CXX11_ABI=1

#-----------------------------
TEMPLATE = lib
QT += qml quick multimedia widgets network
CONFIG += plugin c++11 no_keywords

include(myfeatures.prf)


printLine()
equals(USE_WAKEUP_WORD, 0) {
    printLine("Use a wakeup word", "Yes")
    DEFINES += KWD
} else {
    DEFINES -= KWD
    printLine("Use a wakeup word", "No")
}


INCLUDEPATH += SampleApp/


TARGET = $$qtLibraryTarget($$TARGET)
uri = alexainterface

MOC_DIR = .moc
OBJECTS_DIR = .obj

DEFINES += RAPIDJSON_HAS_STDSTRING

SOURCES += \
        AuthManager.cpp \
        ConnectionManager.cpp \
        alexainterface_plugin.cpp \
        InteractionManager.cpp \
        KeywordObserver.cpp \
        GuiRenderer.cpp \
        AlexaInterface.cpp \
        ObserverManager.cpp \
        BaseCard.cpp \
        WeatherCard.cpp \
        InfoCard.cpp \
        SampleEqualizerModeController.cpp \
        DialogStateManager.cpp \
        CapabilitiesManager.cpp \
        QtMicrophoneWrapper.cpp \
        VehicleIntentCard.cpp

HEADERS += \
        AuthManager.h \
        ConnectionManager.h \
        alexainterface_plugin.h \
        InteractionManager.h \
        KeywordObserver.h \
        GuiRenderer.h \
        AlexaInterface.h \
        ObserverManager.h \
        BaseCard.h \
        WeatherCard.h \
        InfoCard.h \
        SampleEqualizerModeController.h \
        DialogStateManager.h \
        CapabilitiesManager.h \
        QtMicrophoneWrapper.h \
        VehicleIntentCard.h

DISTFILES = qmldir


qmldir.files = qmldir

unix {
    DEFINES += GSTREAMER_MEDIA_PLAYER

    installPath = $$INSTALL_PREFIX/apps/com.luxoft.alexa/imports/alexainterface
    qmldir.path = $$installPath
    target.path = $$installPath

    kwd.files += ./../../kwd/alexa.umdl \
                 ./../../kwd/common.res

    kwd.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa/kwd

    INSTALLS += target qmldir kwd
}

printLine()
