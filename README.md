
# Alexa Voice Assistant app for Neptune 3 UI

Alexa Voice Assistant is a Qt Qml app for Neptune 3 UI (https://doc.qt.io/neptune3ui/neptune3ui-voice-assistant.html)
<br>
It uses AVS Device SDK to connect to Alexa Amazon services. App provides
access to Alexa skills like weather and common search questions. It is also
connected to pre-defined 'cute car' skill, which can open Neptune 3 UI apps
and control the Vehicle app.
<br>
Alexa Voice Assistant is developed with 1.12 SDK version (commit hash ea3ebdb6e75e04ad59d260607aea209c17423682)
<br>
To run Alexa Voice Assistant, do the following:
1. build and install [AVS Device SDK](.#AVS-Device-SDK)
2. build and install [Alexa Voice Assistant app](.#Download-and-run-in-Neptune-3-UI)

Useful links:
https://developer.amazon.com/en-US/alexa/alexa-voice-service/sdk
https://github.com/alexa/avs-device-sdk


## Download and run in Neptune 3 UI

### Install by Qt Application Manager Package

 Compile the package **com.luxoft.alexa.pkg** by running:
 ```
 qmake; make package
 ```
 For more information about packages see
 https://doc.qt.io/QtApplicationManager/appman-packager.html and https://doc.qt.io/archives/QtApplicationManager/package-format.html

 **com.luxoft.alexa.pkg** package could be installed to Neptune 3 UI by running **appman-controller**, a tool from Qt Application Manager
 https://doc.qt.io/QtApplicationManager/appman-controller.html.
 <br>
 Upload package to the device if working with **boot2qt** image by running:
 ```
 scp com.luxoft.alexa.pkg root@device_ip_address:/tmp
 ```
 Then you need to install the package to Neptune 3 UI. To do this, run the following command (on the device
 if working with ***boot2qt***):
```
appman-controller install-package /tmp/com.luxoft.alexa.pkg -a
 ```
 Alexa app should appear in Neptune 3 UI's Launcher.

### Pre-installed Alexa app

To install Alexa app without using packages run qmake with ***INSTALL_PREFIX*** definition:
```
qmake INSTALL_PREFIX=/opt/neptune3 com.luxoft.alexa.pro
```
Thereby, the installation path for Alexa will be set as: */opt/neptune3/apps/com.luxoft.alexa*
<br>
Then run:
```
make -j4; make install
```

## Alexa authorization

 Alexa app comes with pre-paired account. It doesn't allow running multiple instances of Alexa simultaneously.

 To pair your device with your Amazon account, do the following:

 1. Create Amazon developer account on https://developer.amazon.com/ if you don't have one.
 2. Go to com.luxoft.alexa app install path (e.g. /opt/neptune3/apps/com.luxoft.alexa or
    ~/.local/share/Luxoft\ Sweden\ AB/Neptune\ UI/neptune3-ui/apps/com.luxoft.alexa/)
 3. Remove file "database/cblAuthDelegate.db"
 4. Restart Alexa app
 5. Authorization page will be displayed, type in Amazon login and password. App will automatically
    login, receive pairing PIN, type it into the form and pair the device.
 6. Now it is possible to use Alexa app


## Alexa usage

 Alexa can be called by pressing the microphone button.

 By default Wake-Up-Word 'Alexa' is disabled. To enable Wake-Up-Word set USE_WAKEUP_WORD to 0
 in plugins/alexainterface/alexainterface.pro and rebuild the app.

 The app supports display cards for weather and general knowledge.
 Those can be tested by saying for example: "Alexa, what is the weather in Stockholm" or "Alexa, tell me about Sweden"

 The app is compatible with Alexa Skills. Demo skill is created in Alexa developer console. Invocation phrase is: "cute car".
 You can say: "Open cute car skill". Then Alexa should answer "Go on!". After that you get inside Alexa intents of the skill.
 Available intents are:
  - openinig app by the name: "open {app_name} app" where app_name is name of an app with implemented "activate_app" Qt Application Manager intent
  - control vehicle: "open/close {car_part}" where car_part can be right/left window, trunk or sunroof

## AVS Device SDK

### Linux
1. Download and unzip 1.12 version of AVS Device SDK:
```
wget https://github.com/alexa/avs-device-sdk/archive/v1.12.zip
```
2. Download and compile PortAudio:
```
wget -c http://www.portaudio.com/archives/a_stable_v190600_20161030.tgz
tar zxf pa_stable_v190600_20161030.tgz
cd portaudio
./configure --without-jack --host x86_64 --target x86_64
make -j8
```
3. For keyword support download Snowboy:
```
git clone https://github.com/Kitt-AI/snowboy/
```
4. Use cmake to configure AVS Device SDK and build:
```
cmake $path_to_avs/avs-device-sdk-1.12 -DSENSORY_KEY_WORD_DETECTOR=OFF -DGSTREAMER_MEDIA_PLAYER=ON -DPORTAUDIO=ON -DPORTAUDIO_LIB_PATH=$path_to_avs/third-party/portaudio/lib/.libs/libportaudio.a -DPORTAUDIO_INCLUDE_DIR=$path_to_avs/third-party/portaudio/include
make -j8
make DESTDIR=some_build_path install
```

For more information please refer to:
 * https://github.com/alexa/avs-device-sdk/wiki/Ubuntu-Linux-Quick-Start-Guide

### macOS
* https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/mac-os.html


## Future development

 - Support for Windows

