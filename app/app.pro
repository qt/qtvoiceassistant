TEMPLATE = aux

QT += quick widgets

FILES += info.yaml \
         icon.png \
         main.qml \
         Header.qml \
         AlexaView.qml \
         AuthView.qml \
         BasicCard.qml \
         Footer.qml \
         WeatherCard.qml \
         InfoCard.qml \
         MainView.qml \
         AuthWebPageInteraction.qml

app.files = $$FILES
app.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa

assets.files = ./../gfx/*.png
assets.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa/assets

conf.files = ./../conf/AlexaClientSDKConfig.json
conf.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa/conf

database.files = ./../database
database.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa

INSTALLS += app assets conf database

OTHER_FILES += $$FILES
