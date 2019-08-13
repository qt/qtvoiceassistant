
Alexa voice client for Neptune3.

*** Compile the package ***

 Compile the package com.luxoft.alexa.pkg by running 'qmake; make package'. More details about packages are explained on
 https://doc.qt.io/QtApplicationManager/appman-packager.html and https://doc.qt.io/archives/QtApplicationManager/package-format.html

 In an ideal case compilation is not needed, but due to several libraries and dependencies, version numbers might differ.
 The current package in the repository is compiled using Qt 5.13.


*** Download and run in Neptune3 ***

 The compiled com.luxoft.alexa.pkg has to be loaded from a local directory to Neptune3.
 It can be done for example by adding a path to com.luxoft.alexa in Neptune3 Download Center.
 The package is not available on the server with other Neptune3 applications Spotify, Netflix, ...


*** Alexa authorization ***

 Create Amazon developer account on https://developer.amazon.com/ if you don't have one.
 When launching Alexa application first time, the application has to be authorized.

 For automatic authorization is needed QtWebEngine and Qt Application Manager compiled with
 widgets support '-config enable-widgets'. By default Qt Application Manager is compiled without widgets support
 and that's why AlexaAuth plugin assumes widgets are not supported.

 In case automatic authorization is not used, the application opens a web view for manual authorization. Input your email,
 password and authorization code which the application shows.

 Steps for the automatic authorization:
 1) Compile Qt Application manager with widgets support 'qmake -config enable-widgets'
 2) In com.luxoft.alexa/plugins/AlexaAuth/alexaauth.pro change QAPPMAN_ENABLES_WIDGETS to true.
 3) Compile Alexa application 'qmake; make package'
 4) Install Alexa application to Neptune3. Make sure to remove previous version of the application.


*** Alexa usage ***

 Alexa can be called by pressing the microphone button.

 By default wake up word 'Alexa' is disabled because it requires GLIBC 2.27.
 The wake up word can be enabled in com.luxoft.alexa/plugins/alexainterface/alexainterface.pro by setting
 USE_WAKEUP_WORD to 1 and recompiling the application.

 The application supports display cards for weather and general knowledge.
 Those can be tested by saying for example: "Alexa, what is the weather in Stockholm" or "Alexa, tell me about Sweden"

 The application can be added as a widget in Neptune3.


*** Alexa Client SDK ***

 Download and compile PortAudio http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
 with "./congigure --without-jack"

 If you want to use Wake Up word, download Kitt-AI Snowboy https://github.com/Kitt-AI/snowboy/

 Clone and compile Alexa Client SDK with cmake. git://github.com/alexa/avs-device-sdk.git
 1.12 version is required. Commit hash is ea3ebdb6e75e04ad59d260607aea209c17423682

 Please refer to https://github.com/alexa/avs-device-sdk/wiki/Ubuntu-Linux-Quick-Start-Guide for linux
 build instructions


*** Future development ***

 - Support for Mac and Windows
 - For custom skills is needed an AWS account.


