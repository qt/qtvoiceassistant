TEMPLATE = lib
TARGET = alexaauth
QT += qml quick

# Is Qt Application manager compiled with 'enable-widgets' configuration.
# Do not change without recompiling the Qt Application Manger.
QAPPMAN_ENABLES_WIDGETS = 0

equals(QAPPMAN_ENABLES_WIDGETS, 1) {
    qtHaveModule(webenginewidgets) {
        QT += webenginewidgets
        DEFINES += ALEXA_QT_WEBENGINE
    }
    else {
        message("Qt module webenginewidgets is not available.")
    }
}

CONFIG += plugin c++14

TARGET = $$qtLibraryTarget($$TARGET)
uri = alexaauth

HEADERS += \
    alexaauth_plugin.h \
    alexaauth.h

SOURCES += \
    alexaauth_plugin.cpp \
    alexaauth.cpp

MOC_DIR = .moc
OBJECTS_DIR = .obj

DISTFILES = qmldir

qmldir.files = qmldir

unix {
    installPath = /apps/com.luxoft.alexa/imports/alexaauth
    qmldir.path = $$installPath
    target.path = $$installPath

    INSTALLS += target qmldir
}
