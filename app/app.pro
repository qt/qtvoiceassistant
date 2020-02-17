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
         AuthWebPageInteraction.qml \
         SettingsView.qml

app.files = $$FILES
app.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa

stores.files = stores/*
stores.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa/stores

assets.files = ./../gfx/*.png
assets.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa/assets

database.files = ./../database
database.path = $$INSTALL_PREFIX/apps/com.luxoft.alexa

INSTALLS += app stores assets database

OTHER_FILES += $$FILES
