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
         MainView.qml

app.files = $$FILES
app.path = /apps/com.luxoft.alexa

assets.files = ./../gfx/*.png
assets.path = /apps/com.luxoft.alexa/assets

conf.files = ./../conf/AlexaClientSDKConfig.json
conf.path = /apps/com.luxoft.alexa/conf

database.files = ./../database
database.path = /apps/com.luxoft.alexa

INSTALLS += app assets conf database

OTHER_FILES += $$FILES
