TEMPLATE = subdirs

SUBDIRS += plugins \
           app

app.depends = plugins

AM_MANIFEST = $$PWD/app/info.yaml
AM_PACKAGE_DIR = /apps/com.luxoft.alexa

!packagesExist(AlexaClientSDK) {
    log("$$escape_expand(\\n\\n) *** No Alexa Client SDK package available: Make sure Alexa Client SDK is installed and configured correctly ***")
    log("$$escape_expand(\n) *** Please refer to README.md for more information and instructions  ***$$escape_expand(\\n\\n)")
}

load(am-app)
