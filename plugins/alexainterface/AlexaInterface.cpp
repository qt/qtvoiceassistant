/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

#include <ContextManager/ContextManager.h>
#include <ACL/Transport/HTTP2TransportFactory.h>
#include <ACL/Transport/PostConnectSynchronizer.h>
#include <AVSCommon/Utils/LibcurlUtils/LibcurlHTTP2ConnectionFactory.h>
#include <QSettings>

#include "KeywordObserver.h"
#include "AlexaInterface.h"

#ifdef KWD
#include <KWDProvider/KeywordDetectorProvider.h>
#endif

#ifdef ENABLE_ESP
#include <ESP/ESPDataProvider.h>
#else
#include <ESP/DummyESPDataProvider.h>
#endif

#include "QtMicrophoneWrapper.h"

#ifdef GSTREAMER_MEDIA_PLAYER
#include <MediaPlayer/MediaPlayer.h>
#endif

#ifdef ANDROID
#if defined(ANDROID_MEDIA_PLAYER) || defined(ANDROID_MICROPHONE)
#include <AndroidUtilities/AndroidSLESEngine.h>
#endif

#ifdef ANDROID_MEDIA_PLAYER
#include <AndroidSLESMediaPlayer/AndroidSLESMediaPlayer.h>
#include <AndroidSLESMediaPlayer/AndroidSLESSpeaker.h>
#endif

#ifdef ANDROID_MICROPHONE
#include <AndroidUtilities/AndroidSLESMicrophone.h>
#endif

#ifdef ANDROID_LOGGER
#include <AndroidUtilities/AndroidLogger.h>
#endif

#endif

#include <QCoreApplication>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPut.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Logger/LoggerSinkManager.h>
#include <AVSCommon/Utils/Network/InternetConnectionMonitor.h>
#include <Alerts/Storage/SQLiteAlertStorage.h>
#include <Audio/AudioFactory.h>
#include <Bluetooth/SQLiteBluetoothStorage.h>
#include <CBLAuthDelegate/CBLAuthDelegate.h>
#include <CBLAuthDelegate/SQLiteCBLAuthDelegateStorage.h>
#include <CapabilitiesDelegate/CapabilitiesDelegate.h>
#include <Notifications/SQLiteNotificationsStorage.h>
#include <SQLiteStorage/SQLiteMiscStorage.h>
#include <Settings/Storage/SQLiteDeviceSettingStorage.h>
#include <Settings/SQLiteSettingStorage.h>

#include <EqualizerImplementations/EqualizerController.h>
#include <EqualizerImplementations/InMemoryEqualizerConfiguration.h>
#include <EqualizerImplementations/MiscDBEqualizerStorage.h>
#include <EqualizerImplementations/SDKConfigEqualizerConfiguration.h>

#include <algorithm>
#include <cctype>
#include <csignal>
#include <fstream>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

#include "SampleEqualizerModeController.h"

using namespace alexaClientSDK;

/// The sample rate of microphone audio data.
static const unsigned int SAMPLE_RATE_HZ = 16000;

/// The number of audio channels.
static const unsigned int NUM_CHANNELS = 1;

/// The size of each word within the stream.
static const size_t WORD_SIZE = 2;

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

/// The size of the ring buffer.
static const size_t BUFFER_SIZE_IN_SAMPLES = (SAMPLE_RATE_HZ)*AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count();

/// Key for the root node value containing configuration values for Alexa Interface.
static const std::string ALEXA_INTERFACE_CONFIG_KEY("sampleApp");

/// Key for the root node value containing configuration values for Equalizer.
static const std::string EQUALIZER_CONFIG_KEY("equalizer");

/// Key for the @c firmwareVersion value under the @c  configuration node.
static const std::string FIRMWARE_VERSION_KEY("firmwareVersion");

/// Key for the @c endpoint value under the @c  configuration node.
static const std::string ENDPOINT_KEY("endpoint");

/// Key for setting if display cards are supported or not under the @c  configuration node.
static const std::string DISPLAY_CARD_KEY("displayCardsSupported");

using namespace capabilityAgents::externalMediaPlayer;

/// The @c m_playerToMediaPlayerMap Map of the adapter to their speaker-type and MediaPlayer creation methods.
std::unordered_map<std::string, AlexaInterface::SpeakerTypeAndCreateFunc>
AlexaInterface::m_playerToMediaPlayerMap;

/// The singleton map from @c playerId to @c ExternalMediaAdapter creation functions.
std::unordered_map<std::string, ExternalMediaPlayer::AdapterCreateFunction> AlexaInterface::m_adapterToCreateFuncMap;

/// String to identify log entries originating from this file.
static const std::string TAG("AlexaInterface");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// A set of all log levels.
static const std::set<alexaClientSDK::avsCommon::utils::logger::Level> allLevels = {
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG8,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG7,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG6,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG5,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG4,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG3,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG2,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG1,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG0,
    alexaClientSDK::avsCommon::utils::logger::Level::INFO,
    alexaClientSDK::avsCommon::utils::logger::Level::WARN,
    alexaClientSDK::avsCommon::utils::logger::Level::ERROR,
    alexaClientSDK::avsCommon::utils::logger::Level::CRITICAL,
    alexaClientSDK::avsCommon::utils::logger::Level::NONE};

/**
 * Gets a log level consumable by the SDK based on the user input string for log level.
 *
 * @param userInputLogLevel The string to be parsed into a log level.
 * @return The log level. This will default to NONE if the input string is not properly parsable.
 */
static alexaClientSDK::avsCommon::utils::logger::Level getLogLevelFromUserInput(std::string userInputLogLevel) {
    std::transform(userInputLogLevel.begin(), userInputLogLevel.end(), userInputLogLevel.begin(), ::toupper);
    return alexaClientSDK::avsCommon::utils::logger::convertNameToLevel(userInputLogLevel);
}

/**
 * Allows the process to ignore the SIGPIPE signal.
 * The SIGPIPE signal may be received when the application performs a write to a closed socket.
 * This is a case that arises in the use of certain networking libraries.
 *
 * @return true if the action for handling SIGPIPEs was correctly set to ignore, else false.
 */
static bool ignoreSigpipeSignals() {
#ifndef NO_SIGPIPE
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        return false;
    }
#endif
    return true;
}

AlexaInterface::AdapterRegistration::AdapterRegistration(
        const std::string& playerId,
        ExternalMediaPlayer::AdapterCreateFunction createFunction) {
    if (m_adapterToCreateFuncMap.find(playerId) != m_adapterToCreateFuncMap.end()) {
        ACSDK_WARN(LX("Adapter already exists").d("playerID", playerId));
    }

    m_adapterToCreateFuncMap[playerId] = createFunction;
}


AlexaInterface::MediaPlayerRegistration::MediaPlayerRegistration(
        const std::string& playerId,
        avsCommon::sdkInterfaces::SpeakerInterface::Type speakerType,
        MediaPlayerCreateFunction createFunction) {
    if (m_playerToMediaPlayerMap.find(playerId) != m_playerToMediaPlayerMap.end()) {
        ACSDK_WARN(LX("MediaPlayer already exists").d("playerId", playerId));
    }

    m_playerToMediaPlayerMap[playerId] =
            std::pair<avsCommon::sdkInterfaces::SpeakerInterface::Type, MediaPlayerCreateFunction>(
                speakerType, createFunction);
}

// TODO: it should be possible to create only one AlexaInterface.
// More instances will cause unexpected behavior
std::unique_ptr<AlexaInterface> AlexaInterface::instance;
int AlexaInterface::instanceCounter = 0;

AlexaInterface::AlexaInterface(QObject *parent) : QObject(parent)
{
    AlexaInterface::instanceCounter += 1;
}

AlexaInterface::~AlexaInterface() {

    if (AlexaInterface::instanceCounter == 1) {
        AlexaInterface::instance.release();

        if (m_capabilitiesDelegate) {
            m_capabilitiesDelegate->shutdown();
        }

        // First clean up anything that depends on the the MediaPlayers.
        m_externalMusicProviderMediaPlayersMap.clear();

        if (m_interactionManager) {
            m_interactionManager->shutdown();
        }

        // Now it's safe to shut down the MediaPlayers.
        for (auto& mediaPlayer : m_adapterMediaPlayers) {
            mediaPlayer->shutdown();
        }
        if (m_speakMediaPlayer) {
            m_speakMediaPlayer->shutdown();
        }
        if (m_audioMediaPlayer) {
            m_audioMediaPlayer->shutdown();
        }
        if (m_alertsMediaPlayer) {
            m_alertsMediaPlayer->shutdown();
        }
        if (m_notificationsMediaPlayer) {
            m_notificationsMediaPlayer->shutdown();
        }
        if (m_bluetoothMediaPlayer) {
            m_bluetoothMediaPlayer->shutdown();
        }
        if (m_ringtoneMediaPlayer) {
            m_ringtoneMediaPlayer->shutdown();
        }

        avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
    }

    AlexaInterface::instanceCounter -= 1;
}

void AlexaInterface::initAlexaQMLClient()
{
    if (!qEnvironmentVariableIsSet("ALEXA_SDK_CONFIG_FILE")) {
        qCritical() << "Define environment variable ALEXA_SDK_CONFIG_FILE to find AlexaClientSDKConfig.json";
        return;
    }
    if (!QFileInfo::exists(qEnvironmentVariable("ALEXA_SDK_CONFIG_FILE"))) {
        qCritical() << qEnvironmentVariable("ALEXA_SDK_CONFIG_FILE") << "not found";
        return;
    }

    if (AlexaInterface::instanceCounter < 2) {

        std::vector<std::string> configFileStd;
        configFileStd.clear();
        configFileStd.push_back( qEnvironmentVariable("ALEXA_SDK_CONFIG_FILE").toStdString() );

        // If avs-device-sdk is built without keyword support, kwdModelPath remains empty
        QString kwdModelPath;

#ifdef KWD
        if (!qEnvironmentVariableIsSet("ALEXA_KWD_MODEL_PATH")) {
            qCritical() << "ALEXA_KWD_MODEL_PATH not defined";
            return;
        }

        kwdModelPath = qEnvironmentVariable("ALEXA_KWD_MODEL_PATH");

        if (!QFileInfo::exists(kwdModelPath + "/common.res") || !QFileInfo::exists(kwdModelPath + "/alexa.umdl")) {
            qCritical() << "Keyword resource file common.res or voice model alexa.umdl not found, "
                           "please make sure you have ALEXA_KWD_MODEL_PATH/common.res and "
                           "ALEXA_KWD_MODEL_PATH/alexa.umdl";
            return;
        }
#endif
        if (!this->initialize(
                    configFileStd,
                    kwdModelPath.toStdString(),
                    m_logLevelString.toStdString())) {
            qCritical() << "Failed to initialize AlexaInterface.";
#ifdef KWD
            qDebug() << "ALEXA_KWD_MODEL_PATH: " << qEnvironmentVariable("ALEXA_KWD_MODEL_PATH");
#endif
            qDebug() << "ALEXA_SDK_CONFIG_FILE: " << qEnvironmentVariable("ALEXA_SDK_CONFIG_FILE");
        }
        if (!ignoreSigpipeSignals()) {
            qCritical() << "Failed to set a signal handler for SIGPIPE";
        }

        AlexaInterface::instance.reset(this);
    }
}

void AlexaInterface::tapToTalk()
{
    m_interactionManager->tap();
}

void AlexaInterface::stopTalking()
{
    m_interactionManager->stopForegroundActivity();
}

bool AlexaInterface::createMediaPlayersForAdapters(
        std::shared_ptr<avsCommon::utils::libcurlUtils::HTTPContentFetcherFactory> httpContentFetcherFactory,
        std::shared_ptr<defaultClient::EqualizerRuntimeSetup> equalizerRuntimeSetup,
        std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>& additionalSpeakers) {
#ifdef GSTREAMER_MEDIA_PLAYER
    bool equalizerEnabled = nullptr != equalizerRuntimeSetup;
    for (auto& entry : m_playerToMediaPlayerMap) {
        auto mediaPlayer =
                entry.second.second(httpContentFetcherFactory, equalizerEnabled, entry.second.first, entry.first + "MediaPlayer");
        if (mediaPlayer) {
            m_externalMusicProviderMediaPlayersMap[entry.first] = mediaPlayer;
            m_externalMusicProviderSpeakersMap[entry.first] = mediaPlayer;
            additionalSpeakers.push_back(
                        std::static_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface>(mediaPlayer));
            m_adapterMediaPlayers.push_back(mediaPlayer);
            if (equalizerEnabled) {
                equalizerRuntimeSetup->addEqualizer(mediaPlayer);
            }
        } else {
            ACSDK_CRITICAL(LX("Failed to create mediaPlayer").d("playerId", entry.first));
            return false;
        }
    }

    return true;
#else
    if (!m_playerToMediaPlayerMap.empty()) {
        // TODO(ACSDK-1622) Add support to external media players on android.
        ACSDK_CRITICAL(LX("Failed to create media players").d("reason", "unsupportedOperation"));
        return false;
    }
    return true;
#endif
}

bool AlexaInterface::initialize(
        const std::vector<std::string>& configFiles,
        const std::string& pathToInputFolder,
        const std::string& logLevel) {

    avsCommon::utils::logger::Level logLevelValue = avsCommon::utils::logger::Level::UNKNOWN;
    if (!logLevel.empty()) {
        logLevelValue = getLogLevelFromUserInput(logLevel);
        if (alexaClientSDK::avsCommon::utils::logger::Level::UNKNOWN == logLevelValue) {
            for (auto it = allLevels.begin(); it != allLevels.end(); ++it) {
                qDebug() << alexaClientSDK::avsCommon::utils::logger::convertLevelToName(*it).c_str();
            }
            return false;
        }
    }

    std::vector<std::shared_ptr<std::istream>> configJsonStreams;

    for (auto configFile : configFiles) {
        if (configFile.empty()) {
            qWarning() << "Config filename is empty!";
            //TODO: error signal
            return false;
        }

        auto configInFile = std::shared_ptr<std::ifstream>(new std::ifstream(configFile));
        if (!configInFile->good()) {
            qWarning() << "Failed to read config file " << configFile.c_str();
            //TODO: error signal
            return false;
        }

        configJsonStreams.push_back(configInFile);
    }

    if (!avsCommon::avs::initialization::AlexaClientSDKInit::initialize(configJsonStreams)) {
        ACSDK_CRITICAL(LX("Failed to initialize SDK!"));
        //TODO: error signal
        return false;
    }

    auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();
    auto alexaInterfaceConfig = config[ALEXA_INTERFACE_CONFIG_KEY];

    auto httpContentFetcherFactory = std::make_shared<avsCommon::utils::libcurlUtils::HTTPContentFetcherFactory>();

    // Creating the misc DB object to be used by various components.
    std::shared_ptr<alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage> miscStorage =
        alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage::create(config);

    /*
     * Creating Equalizer specific implementations
     */
    auto equalizerConfigBranch = config[EQUALIZER_CONFIG_KEY];
    auto equalizerConfiguration = equalizer::SDKConfigEqualizerConfiguration::create(equalizerConfigBranch);
    std::shared_ptr<defaultClient::EqualizerRuntimeSetup> equalizerRuntimeSetup = nullptr;

    bool equalizerEnabled = false;
    if (equalizerConfiguration && equalizerConfiguration->isEnabled()) {
        equalizerEnabled = true;
        equalizerRuntimeSetup = std::make_shared<defaultClient::EqualizerRuntimeSetup>();
        auto equalizerStorage = equalizer::MiscDBEqualizerStorage::create(miscStorage);
        //auto equalizerModeController = AlexaInterface::SampleEqualizerModeController::create();
        auto equalizerModeController = alexaClientSDK::SampleEqualizerModeController::create();

        equalizerRuntimeSetup->setStorage(equalizerStorage);
        equalizerRuntimeSetup->setConfiguration(equalizerConfiguration);
        equalizerRuntimeSetup->setModeController(equalizerModeController);
    }

#if defined(ANDROID_MEDIA_PLAYER) || defined(ANDROID_MICROPHONE)
    m_openSlEngine = applicationUtilities::androidUtilities::AndroidSLESEngine::create();
    if (!m_openSlEngine) {
        ACSDK_ERROR(LX("createAndroidMicFailed").d("reason", "failed to create engine"));
        return false;
    }
#endif

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> speakSpeaker;
    std::tie(m_speakMediaPlayer, speakSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                false,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                "SpeakMediaPlayer");
    if (!m_speakMediaPlayer || !speakSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for speech!"));
        return false;
    }

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> audioSpeaker;
    std::tie(m_audioMediaPlayer, audioSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                equalizerEnabled,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                "AudioMediaPlayer");
    if (!m_audioMediaPlayer || !audioSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for content!"));
        return false;
    }

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> notificationsSpeaker;
    std::tie(m_notificationsMediaPlayer, notificationsSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                false,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                "NotificationsMediaPlayer");
    if (!m_notificationsMediaPlayer || !notificationsSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for notifications!"));
        return false;
    }

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> bluetoothSpeaker;
    std::tie(m_bluetoothMediaPlayer, bluetoothSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                false,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                "BluetoothMediaPlayer");

    if (!m_bluetoothMediaPlayer || !bluetoothSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for bluetooth!"));
        return false;
    }

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> ringtoneSpeaker;
    std::tie(m_ringtoneMediaPlayer, ringtoneSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                false,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                "RingtoneMediaPlayer");
    if (!m_ringtoneMediaPlayer || !ringtoneSpeaker) {
        qWarning() << "Failed to create media player for ringtones!";
        return false;
    }

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> alertsSpeaker;
    std::tie(m_alertsMediaPlayer, alertsSpeaker) = createApplicationMediaPlayer(
                httpContentFetcherFactory,
                false,
                avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_ALERTS_VOLUME,
                "AlertsMediaPlayer");
    if (!m_alertsMediaPlayer || !alertsSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for alerts!"));
        return false;
    }

#ifdef ENABLE_PCC
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> phoneSpeaker;
    std::shared_ptr<ApplicationMediaPlayer> phoneMediaPlayer;
    std::tie(phoneMediaPlayer, phoneSpeaker) = createApplicationMediaPlayer(
        httpContentFetcherFactory,
        false,
        avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
        "PhoneMediaPlayer");

    if (!phoneMediaPlayer || !phoneSpeaker) {
        ACSDK_CRITICAL(LX("Failed to create media player for phone!"));
        return false;
    }
#endif

    std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>> additionalSpeakers;

    if (!createMediaPlayersForAdapters(httpContentFetcherFactory, equalizerRuntimeSetup, additionalSpeakers)) {
        ACSDK_CRITICAL(LX("Could not create mediaPlayers for adapters"));
        return false;
    }

    auto audioFactory = std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::AudioFactory>();

    // Creating equalizers
    if (nullptr != equalizerRuntimeSetup) {
        equalizerRuntimeSetup->addEqualizer(m_audioMediaPlayer);
    }

    // Creating the alert storage object to be used for rendering and storing alerts.
    auto alertStorage =
            alexaClientSDK::capabilityAgents::alerts::storage::SQLiteAlertStorage::create(config, audioFactory->alerts());

    // Creating the message storage object to be used for storing message to be sent later.
    auto messageStorage = alexaClientSDK::certifiedSender::SQLiteMessageStorage::create(config);

    /*
     * Creating notifications storage object to be used for storing notification indicators.
     */
    auto notificationsStorage =
            alexaClientSDK::capabilityAgents::notifications::SQLiteNotificationsStorage::create(config);

    /*
     * Creating settings storage object to be used for storing <key, value> pairs of AVS Settings.
     */
    auto settingsStorage = alexaClientSDK::capabilityAgents::settings::SQLiteSettingStorage::create(config);

    /*
     * Creating new device settings storage object to be used for storing AVS Settings.
     */
    auto deviceSettingsStorage = alexaClientSDK::settings::storage::SQLiteDeviceSettingStorage::create(config);

    // Create HTTP Put handler
    std::shared_ptr<avsCommon::utils::libcurlUtils::HttpPut> httpPut =
            avsCommon::utils::libcurlUtils::HttpPut::create();

    /*
     * Creating bluetooth storage object to be used for storing uuid to mac mappings for devices.
     */
    auto bluetoothStorage = alexaClientSDK::capabilityAgents::bluetooth::SQLiteBluetoothStorage::create(config);

    /*
     * Creating the UI component that observes various components and prints to the console accordingly.
     */
    m_userInterfaceManager = std::make_shared<ObserverManager>(this);

    /*
     * AuthManager - component that observes the application authorization process.
     */
    m_authManager = std::make_shared<AuthManager>(this);

    QObject::connect(m_authManager.get(), &AuthManager::authStateChanged, this,
                     &AlexaInterface::onAuthStateChanged, Qt::QueuedConnection);
    QObject::connect(m_authManager.get(), &AuthManager::authErrorChanged, this,
                     &AlexaInterface::onAuthErrorChanged, Qt::QueuedConnection);
    QObject::connect(m_authManager.get(), &AuthManager::isLoggedInChanged, this,
                     &AlexaInterface::onIsLoggedInChanged, Qt::QueuedConnection);
    QObject::connect(m_authManager.get(), &AuthManager::authCodeReady, this,
                     &AlexaInterface::onAuthCodeReady, Qt::QueuedConnection);

    /*
     * ConnectionManager - component that observes the application connection process.
     */
    m_connectionManager = std::make_shared<ConnectionManager>(this);

    QObject::connect(m_connectionManager.get(), &ConnectionManager::connectionStatusChanged, this,
                     &AlexaInterface::onConnectionStatusChanged, Qt::QueuedConnection);

    /*
     * DialogStateManager - component that observes the application dialog states.
     */
    m_dialogStateManager = std::make_shared<DialogStateManager>(this);

    QObject::connect(m_dialogStateManager.get(), &DialogStateManager::dialogStateChanged, this, [=](){
        m_dialogState = m_dialogStateManager->dialogState();
        Q_EMIT dialogStateChanged();
    } );

    /*
     * CapabilitiesManager - a component that observers a state of the CapabilitiesDelegate
    */
    m_capabilitiesManager = std::make_shared<CapabilitiesManager>(this);

    /*
     * Creating customerDataManager which will be used by the registrationManager and all classes that extend
     * CustomerDataHandler
     */
    auto customerDataManager = std::make_shared<registrationManager::CustomerDataManager>();

    /*
     * Creating the deviceInfo object
     */
    std::shared_ptr<avsCommon::utils::DeviceInfo> deviceInfo = avsCommon::utils::DeviceInfo::create(config);
    if (!deviceInfo) {
        ACSDK_CRITICAL(LX("Creation of DeviceInfo failed!"));
        return false;
    }

    /*
     * Creating the AuthDelegate - this component takes care of LWA and authorization of the client.
     */
    auto authDelegateStorage = authorization::cblAuthDelegate::SQLiteCBLAuthDelegateStorage::create(config);
    std::shared_ptr<avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate =
            authorization::cblAuthDelegate::CBLAuthDelegate::create(config, customerDataManager,
                                                                    std::move(authDelegateStorage), m_authManager,
                                                                    nullptr, deviceInfo);

    if (!authDelegate) {
        ACSDK_CRITICAL(LX("Creation of AuthDelegate failed!"));
        return false;
    }

    /*
     * Creating the CapabilitiesDelegate - This component provides the client with the ability to send messages to the
     * Capabilities API.
     */
    m_capabilitiesDelegate = alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate::create(
                authDelegate, miscStorage, httpPut, customerDataManager, config, deviceInfo);

    if (!m_capabilitiesDelegate) {
        qWarning() << "Creation of CapabilitiesDelegate failed!";
        return false;
    }

    authDelegate->addAuthObserver(m_authManager);
    m_capabilitiesDelegate->addCapabilitiesObserver(m_capabilitiesManager);

    // INVALID_FIRMWARE_VERSION is passed to @c getInt() as a default in case FIRMWARE_VERSION_KEY is not found.
    int firmwareVersion = static_cast<int>(avsCommon::sdkInterfaces::softwareInfo::INVALID_FIRMWARE_VERSION);
    alexaInterfaceConfig.getInt(FIRMWARE_VERSION_KEY, &firmwareVersion, firmwareVersion);

    /*
     * Check to see if displayCards is supported on the device. The default is supported unless specified otherwise in
     * the configuration.
     */
    bool displayCardsSupported;
    config[ALEXA_INTERFACE_CONFIG_KEY].getBool(DISPLAY_CARD_KEY, &displayCardsSupported, true);

    /*
     * Creating the InternetConnectionMonitor that will notify observers of internet connection status changes.
     */
    auto internetConnectionMonitor =
            avsCommon::utils::network::InternetConnectionMonitor::create(httpContentFetcherFactory);
    if (!internetConnectionMonitor) {
        ACSDK_CRITICAL(LX("Failed to create InternetConnectionMonitor"));
        return false;
    }

    /*
     * Creating the Context Manager - This component manages the context of each of the components to update to AVS.
     * It is required for each of the capability agents so that they may provide their state just before any event is
     * fired off.
     */
    auto contextManager = contextManager::ContextManager::create();
    if (!contextManager) {
        ACSDK_CRITICAL(LX("Creation of ContextManager failed."));
        return false;
    }

    /*
     * Create a factory for creating objects that handle tasks that need to be performed right after establishing
     * a connection to AVS.
     */
    auto postConnectSynchronizerFactory = acl::PostConnectSynchronizerFactory::create(contextManager);

    /*
     * Create a factory to create objects that establish a connection with AVS.
     */
    auto transportFactory = std::make_shared<acl::HTTP2TransportFactory>(
        std::make_shared<avsCommon::utils::libcurlUtils::LibcurlHTTP2ConnectionFactory>(),
        postConnectSynchronizerFactory);

    /*
     * Create the BluetoothDeviceManager to communicate with the Bluetooth stack.
     */
    std::unique_ptr<avsCommon::sdkInterfaces::bluetooth::BluetoothDeviceManagerInterface> bluetoothDeviceManager;

#ifdef BLUETOOTH_BLUEZ
    auto eventBus = std::make_shared<avsCommon::utils::bluetooth::BluetoothEventBus>();

#ifdef BLUETOOTH_BLUEZ_PULSEAUDIO_OVERRIDE_ENDPOINTS
    /*
     * Create PulseAudio initializer object. Subscribe to BLUETOOTH_DEVICE_MANAGER_INITIALIZED event before we create
     * the BT Device Manager, otherwise may miss it.
     */
    m_pulseAudioInitializer = bluetoothImplementations::blueZ::PulseAudioBluetoothInitializer::create(eventBus);
#endif

    bluetoothDeviceManager = bluetoothImplementations::blueZ::BlueZBluetoothDeviceManager::create(eventBus);
#endif


    /*
     * Creating the DefaultClient - this component serves as an out-of-box default object that instantiates and "glues"
     * together all the modules.
     */
    std::shared_ptr<alexaClientSDK::defaultClient::DefaultClient> client =
            alexaClientSDK::defaultClient::DefaultClient::create(
                deviceInfo,
                customerDataManager,
                m_externalMusicProviderMediaPlayersMap,
                m_externalMusicProviderSpeakersMap,
                m_adapterToCreateFuncMap,
                m_speakMediaPlayer,
                m_audioMediaPlayer,
                m_alertsMediaPlayer,
                m_notificationsMediaPlayer,
                m_bluetoothMediaPlayer,
                m_ringtoneMediaPlayer,
                speakSpeaker,
                audioSpeaker,
                alertsSpeaker,
                notificationsSpeaker,
                bluetoothSpeaker,
                ringtoneSpeaker,
                additionalSpeakers,
#ifdef ENABLE_PCC
                phoneSpeaker,
                phoneCaller,
#endif
                equalizerRuntimeSetup,
                audioFactory,
                authDelegate,
                std::move(alertStorage),
                std::move(messageStorage),
                std::move(notificationsStorage),
                std::move(settingsStorage),
                std::move(deviceSettingsStorage),
                std::move(bluetoothStorage),
                {m_dialogStateManager},
                {m_connectionManager},
                std::move(internetConnectionMonitor),
                displayCardsSupported,
                m_capabilitiesDelegate,
                contextManager,
                transportFactory,
                firmwareVersion,
                true,
                nullptr,
                std::move(bluetoothDeviceManager));

    if (!client) {
        ACSDK_CRITICAL(LX("Failed to create default SDK client!"));
        return false;
    }

    // Add userInterfaceManager as observer of locale setting.
    client->addSettingObserver("locale", m_userInterfaceManager);

    client->addSpeakerManagerObserver(m_userInterfaceManager);

    client->addNotificationsObserver(m_userInterfaceManager);

    /*
     * Add GUI Renderer as an observer if display cards are supported.
     */
    if (displayCardsSupported) {
        m_guiRenderer = std::make_shared<GuiRenderer>(this);
        client->addTemplateRuntimeObserver(m_guiRenderer);
    }

    QObject::connect(m_guiRenderer.get(), &GuiRenderer::templateCardContentReady, this, [=](QString jsonString) {

        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        if (type == "WeatherTemplate") {
            QObject *p = qobject_cast<QObject*>(this);
            if (p) {
                WeatherCard *weatherCard = new WeatherCard(p);
                weatherCard->setJsonDocument(doc);
                Q_EMIT cardReady(weatherCard);
            } else {
                qWarning() << "Failed to create a weather card!";
            }
        } else if ((type == "BodyTemplate1") || (type == "BodyTemplate2")) {
            QObject *p = qobject_cast<QObject*>(this);
            if (p) {
                QString title = obj["title"].toObject()["mainTitle"].toString();
                if (title == "amzn1.ask.skill.245b836a-df7a-4407-a6a9-ccc2afd2c73e") {
                    VehicleIntentCard *card = new VehicleIntentCard(p);
                    card->setJsonDocument(doc);
                    Q_EMIT cardReady(card);
                } else {
                    InfoCard *infoCard = new InfoCard(p);
                    infoCard->setJsonDocument(doc);
                    Q_EMIT cardReady(infoCard);
                }
            } else {
                qWarning() << "Failed to create an info card!";
            }
        }
        else qWarning() << "Unknown card type";

    });


    /*
     * Creating the buffer (Shared Data Stream) that will hold user audio data. This is the main input into the SDK.
     */
    size_t bufferSize = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize(
                BUFFER_SIZE_IN_SAMPLES, WORD_SIZE, MAX_READERS);
    auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>(bufferSize);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> sharedDataStream =
            alexaClientSDK::avsCommon::avs::AudioInputStream::create(buffer, WORD_SIZE, MAX_READERS);

    if (!sharedDataStream) {
        ACSDK_CRITICAL(LX("Failed to create shared data stream!"));
        return false;
    }

    alexaClientSDK::avsCommon::utils::AudioFormat compatibleAudioFormat;
    compatibleAudioFormat.sampleRateHz = SAMPLE_RATE_HZ;
    compatibleAudioFormat.sampleSizeInBits = WORD_SIZE * CHAR_BIT;
    compatibleAudioFormat.numChannels = NUM_CHANNELS;
    compatibleAudioFormat.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
    compatibleAudioFormat.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;

    /*
     * Creating each of the audio providers. An audio provider is a simple package of data consisting of the stream
     * of audio data, as well as metadata about the stream. For each of the three audio providers created here, the same
     * stream is used since this alexa interface will only have one microphone.
     */

    // Creating tap to talk audio provider
    bool tapAlwaysReadable = true;
    bool tapCanOverride = true;
    bool tapCanBeOverridden = true;

    alexaClientSDK::capabilityAgents::aip::AudioProvider tapToTalkAudioProvider(
                sharedDataStream,
                compatibleAudioFormat,
                alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD,
                tapAlwaysReadable,
                tapCanOverride,
                tapCanBeOverridden);

    // Creating hold to talk audio provider
    bool holdAlwaysReadable = false;
    bool holdCanOverride = true;
    bool holdCanBeOverridden = false;

    alexaClientSDK::capabilityAgents::aip::AudioProvider holdToTalkAudioProvider(
                sharedDataStream,
                compatibleAudioFormat,
                alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK,
                holdAlwaysReadable,
                holdCanOverride,
                holdCanBeOverridden);

    /*
     * Read device name to change system default microphone input
     */
    QSettings settings(QStringLiteral("Luxoft Sweden AB"), QStringLiteral("AlexaApp"));
    QString captureDeviceName = settings.value(QStringLiteral("capture/device_name"),
                                               QStringLiteral("default")).toString();
    std::shared_ptr<QtMicrophoneWrapper> m_micWrapper = QtMicrophoneWrapper::create(sharedDataStream,
                                                                                  captureDeviceName);
    if (!m_micWrapper) {
        ACSDK_CRITICAL(LX("Failed to create QtMicrophoneWrapper!"));
        return false;
    }

    // Creating wake word audio provider, if necessary
#ifdef KWD
    bool wakeAlwaysReadable = true;
    bool wakeCanOverride = false;
    bool wakeCanBeOverridden = true;

    alexaClientSDK::capabilityAgents::aip::AudioProvider wakeWordAudioProvider(
                sharedDataStream,
                compatibleAudioFormat,
                alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD,
                wakeAlwaysReadable,
                wakeCanOverride,
                wakeCanBeOverridden);

#ifdef ENABLE_ESP
    // Creating ESP connector
    std::shared_ptr<esp::ESPDataProviderInterface> espProvider = esp::ESPDataProvider::create(wakeWordAudioProvider);
    std::shared_ptr<esp::ESPDataModifierInterface> espModifier = nullptr;
#else
    // Create dummy ESP connector
    auto dummyEspProvider = std::make_shared<esp::DummyESPDataProvider>();
    std::shared_ptr<esp::ESPDataProviderInterface> espProvider = dummyEspProvider;
    std::shared_ptr<esp::ESPDataModifierInterface> espModifier = dummyEspProvider;
#endif

    // This observer is notified any time a keyword is detected and notifies the DefaultClient to start recognizing.
    auto keywordObserver =
            std::make_shared<KeywordObserver>(this, client, wakeWordAudioProvider, espProvider);

    m_keywordDetector = alexaClientSDK::kwd::KeywordDetectorProvider::create(
                sharedDataStream,
                compatibleAudioFormat,
    {keywordObserver},
                std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>(),
                pathToInputFolder);
    if (!m_keywordDetector) {
        ACSDK_CRITICAL(LX("Failed to create keyword detector!"));
    }

    // If wake word is enabled, then creating the interaction manager with a wake word audio provider.
    m_interactionManager = std::make_shared<InteractionManager>(
                this,
                client,
                m_micWrapper,
                m_userInterfaceManager,
                holdToTalkAudioProvider,
                tapToTalkAudioProvider,
                m_guiRenderer,
                wakeWordAudioProvider,
                espProvider,
                espModifier);

#else
    // If wake word is not enabled, then creating the interaction manager without a wake word audio provider.
    m_interactionManager = std::make_shared<InteractionManager>(
                this, client, m_micWrapper, m_userInterfaceManager, holdToTalkAudioProvider, tapToTalkAudioProvider, m_guiRenderer);
#endif

    client->addAlexaDialogStateObserver(m_interactionManager);

    authDelegate->addAuthObserver(m_authManager);
    client->getRegistrationManager()->addObserver(m_authManager);
    m_capabilitiesDelegate->addCapabilitiesObserver(m_capabilitiesManager);
    m_capabilitiesDelegate->addCapabilitiesObserver(client);


    // Connect once configuration is all set.
    std::string endpoint;
    alexaInterfaceConfig.getString(ENDPOINT_KEY, &endpoint);

    client->connect(m_capabilitiesDelegate, endpoint);

    // Send default settings set by the user to AVS.
    client->sendDefaultSettings();

    return true;
}

std::pair<std::shared_ptr<ApplicationMediaPlayer>, std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>
AlexaInterface::createApplicationMediaPlayer(
        std::shared_ptr<avsCommon::utils::libcurlUtils::HTTPContentFetcherFactory> httpContentFetcherFactory,
        bool enableEqualizer,
        avsCommon::sdkInterfaces::SpeakerInterface::Type type,
        const std::string& name) {
#ifdef GSTREAMER_MEDIA_PLAYER
    /*
     * For the SDK, the MediaPlayer happens to also provide volume control functionality.
     * Note the externalMusicProviderMediaPlayer is not added to the set of SpeakerInterfaces as there would be
     * more actions needed for these beyond setting the volume control on the MediaPlayer.
     */
    auto mediaPlayer = alexaClientSDK::mediaPlayer::MediaPlayer::create(httpContentFetcherFactory, enableEqualizer, type, name);
    return {mediaPlayer,
                std::static_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface>(mediaPlayer)};
#elif defined(ANDROID_MEDIA_PLAYER)
    auto mediaPlayer =
            mediaPlayer::android::AndroidSLESMediaPlayer::create(httpContentFetcherFactory, m_openSlEngine, type, name);
    auto speaker = mediaPlayer->getSpeaker();
    return {std::move(mediaPlayer), speaker};
#endif
}

void AlexaInterface::setLogLevel(AlexaInterface::LogLevel logLevel)
{
    if (m_logLevel == logLevel)
        return;

    if (m_logLevel == LogLevel::Debug0) {
        m_logLevelString = "DEBUG0";
    } else if (m_logLevel == LogLevel::Debug1) {
        m_logLevelString = "DEBUG1";
    } else if (m_logLevel == LogLevel::Debug2) {
        m_logLevelString = "DEBUG2";
    } else if (m_logLevel == LogLevel::Debug3) {
        m_logLevelString = "DEBUG3";
    } else if (m_logLevel == LogLevel::Debug4) {
        m_logLevelString = "DEBUG4";
    } else if (m_logLevel == LogLevel::Debug5) {
        m_logLevelString = "DEBUG5";
    } else if (m_logLevel == LogLevel::Debug6) {
        m_logLevelString = "DEBUG6";
    } else if (m_logLevel == LogLevel::Debug7) {
        m_logLevelString = "DEBUG7";
    } else if (m_logLevel == LogLevel::Debug8) {
        m_logLevelString = "DEBUG8";
    } else if (m_logLevel == LogLevel::Debug9) {
        m_logLevelString = "DEBUG9";
    } else if (m_logLevel == LogLevel::Info) {
        m_logLevelString = "INFO";
    } else if (m_logLevel == LogLevel::Warn) {
        m_logLevelString = "WARN";
    } else if (m_logLevel == LogLevel::Error) {
        m_logLevelString = "ERROR";
    } else if (m_logLevel == LogLevel::Critical) {
        m_logLevelString = "CRITICAL";
    } else if (m_logLevel == LogLevel::None) {
        m_logLevelString = "NONE";
    }

    m_logLevel = logLevel;
    Q_EMIT logLevelChanged();
}

void AlexaInterface::onAuthStateChanged() {
    m_authState = m_authManager->authState();
    Q_EMIT authStateChanged();
}

void AlexaInterface::onAuthErrorChanged() {
    m_authError = m_authManager->authError();
    Q_EMIT authErrorChanged();
}

void AlexaInterface::onIsLoggedInChanged() {
    m_loggedIn = m_authManager->isLoggedIn();
    Q_EMIT loggedInChanged();
}

void AlexaInterface::onAuthCodeReady(QString authURL, QString authCode) {
    m_authUrl = authURL;
    Q_EMIT authUrlChanged();

    m_authCode = authCode;
    Q_EMIT authCodeChanged();
}

void AlexaInterface::onConnectionStatusChanged() {
    m_connectionStatus = m_connectionManager->connectionStatus();
    Q_EMIT connectionStatusChanged();
}
