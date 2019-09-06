TEMPLATE = lib
TARGET = alexaauth
QT += qml quick webengine

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
