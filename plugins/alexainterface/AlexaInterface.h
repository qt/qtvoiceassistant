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

#ifndef ALEXA_INTERFACE_H_
#define ALEXA_INTERFACE_H_

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "DefaultClient/EqualizerRuntimeSetup.h"
#include "GuiRenderer.h"
#include "InteractionManager.h"
#include "AuthManager.h"
#include "ConnectionManager.h"
#include "DialogStateManager.h"
#include "CapabilitiesManager.h"
#include "QtMicrophoneWrapper.h"

#ifdef KWD
#include <KWD/AbstractKeywordDetector.h>
#endif

#ifdef GSTREAMER_MEDIA_PLAYER
#include <MediaPlayer/MediaPlayer.h>
#elif defined(ANDROID_MEDIA_PLAYER)
#include <AndroidSLESMediaPlayer/AndroidSLESMediaPlayer.h>
#endif

#include <CapabilitiesDelegate/CapabilitiesDelegate.h>
#include <ExternalMediaPlayer/ExternalMediaPlayer.h>

#include <QObject>
#include <QUrl>
#include <QQmlEngine>
#include <QSettings>

#include "WeatherCard.h"
#include "InfoCard.h"
#include "vehicleintentcard.h"

using namespace alexaClientSDK;

#ifdef GSTREAMER_MEDIA_PLAYER
using ApplicationMediaPlayer = mediaPlayer::MediaPlayer;
#elif defined(ANDROID_MEDIA_PLAYER)
using ApplicationMediaPlayer = mediaPlayer::android::AndroidSLESMediaPlayer;
#endif

/// Class to manage the top-level components of the AVS Client Application
class AlexaInterface: public QObject {
    Q_OBJECT

    Q_PROPERTY(DialogStateManager::DialogState dialogState READ dialogState NOTIFY dialogStateChanged)
    Q_PROPERTY(AuthManager::AuthState authState READ authState NOTIFY authStateChanged)
    Q_PROPERTY(AuthManager::AuthError authError READ authError NOTIFY authErrorChanged)
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY loggedInChanged)
    Q_PROPERTY(QUrl authUrl READ authUrl NOTIFY authUrlChanged)
    Q_PROPERTY(QString authCode READ authCode NOTIFY authCodeChanged)
    Q_PROPERTY(ConnectionManager::ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel NOTIFY logLevelChanged)
    Q_PROPERTY(qreal audioLevel READ audioLevel NOTIFY audioLevelChanged)
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)

public:

    enum LogLevel {
        Debug9,
        Debug8,
        Debug7,
        Debug6,
        Debug5,
        Debug4,
        Debug3,
        Debug2,
        Debug1,
        Debug0,
        Info,
        Warn,
        Error,
        Critical,
        None
    };
    Q_ENUM(LogLevel)

    DialogStateManager::DialogState dialogState() const { return m_dialogState; }
    AuthManager::AuthState authState() const { return m_authState; }
    AuthManager::AuthError authError() const { return m_authError; }
    bool loggedIn() const { return m_loggedIn; }
    QUrl authUrl() const { return m_authUrl; }
    QString authCode() const { return m_authCode; }
    ConnectionManager::ConnectionStatus connectionStatus() const { return m_connectionStatus; }
    LogLevel logLevel() const { return m_logLevel; }
    qreal audioLevel() const { return m_micWrapper ? m_micWrapper->audioLevel() : 0.0; }
    QStringList deviceList() const { return m_micWrapper ? m_micWrapper->deviceList() : QStringList(); }
    QString deviceName() const { return m_micWrapper ? m_micWrapper->deviceName() : ""; }
    void setDeviceName(const QString &audioDeviceName);

    explicit AlexaInterface(QObject* parent = nullptr);
    /// Destructor which manages the @c AlexaInterface shutdown sequence.
    ~AlexaInterface();

    /**
     *  This must be called in QML.
     */
    Q_INVOKABLE void initAlexaQMLClient();
    Q_INVOKABLE void tapToTalk();
    Q_INVOKABLE void stopTalking();

    /**
     * Method to create mediaPlayers for the optional music provider adapters plugged into the SDK.
     *
     * @param httpContentFetcherFactory The HTTPContentFetcherFactory to be used while creating the mediaPlayers.
     * @param equalizerRuntimeSetup Equalizer runtime setup to register equalizers
     * @param additionalSpeakers The speakerInterface to add the created mediaPlayer.
     * @return @c true if the mediaPlayer of all the registered adapters could be created @c false otherwise.
     */
    bool createMediaPlayersForAdapters(
        std::shared_ptr<avsCommon::utils::libcurlUtils::HTTPContentFetcherFactory> httpContentFetcherFactory,
        std::shared_ptr<defaultClient::EqualizerRuntimeSetup> equalizerRuntimeSetup,
        std::vector<std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>& additionalSpeakers);

    /**
     * Instances of this class register ExternalMediaAdapters. Each adapter registers itself by instantiating
     * a static instance of the below class supplying their business name and creator method.
     */
    class AdapterRegistration {
    public:
        /**
         * Register an @c ExternalMediaAdapter for use by @c ExternalMediaPlayer.
         *
         * @param playerId The @c playerId identifying the @c ExternalMediaAdapter to register.
         * @param createFunction The function to use to create instances of the specified @c ExternalMediaAdapter.
         */
        AdapterRegistration(
            const std::string& playerId,
            capabilityAgents::externalMediaPlayer::ExternalMediaPlayer::AdapterCreateFunction createFunction);
    };

    /**
     * Signature of functions to create a MediaPlayer.
     *
     * @param httpContentFetcherFactory The HTTPContentFetcherFactory to be used while creating the mediaPlayers.
     * @param type The type of the SpeakerInterface.
     * @param name The name of the MediaPlayer instance.
     * @return Return shared pointer to the created MediaPlayer instance.
     */
    using MediaPlayerCreateFunction = std::shared_ptr<ApplicationMediaPlayer> (*)(
        std::shared_ptr<avsCommon::sdkInterfaces::HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory,
        bool enableEqualizer,
        avsCommon::sdkInterfaces::SpeakerInterface::Type type,
        std::string name);

    /**
     * Instances of this class register MediaPlayers to be created. Each third-party adapter registers a mediaPlayer
     * for itself by instantiating a static instance of the below class supplying their business name, speaker interface
     * type and creator method.
     */
    class MediaPlayerRegistration {
    public:
        /**
         * Register a @c MediaPlayer for use by a music provider adapter.
         *
         * @param playerId The @c playerId identifying the @c ExternalMediaAdapter to register.
         * @speakerType The SpeakerType of the mediaPlayer to be created.
         * @param createFunction The function to use to create instances of the mediaPlayer to use for the player.
         */
        MediaPlayerRegistration(
            const std::string& playerId,
            avsCommon::sdkInterfaces::SpeakerInterface::Type speakerType,
            MediaPlayerCreateFunction createFunction);
    };

public Q_SLOTS:
    void setLogLevel(LogLevel logLevel);

private Q_SLOTS:
    void onAuthStateChanged();
    void onAuthErrorChanged();
    void onIsLoggedInChanged();
    void onAuthCodeReady(QString authUrl, QString authCode);
    void onConnectionStatusChanged();

Q_SIGNALS:
    void dialogStateChanged();
    void authStateChanged();
    void authErrorChanged();
    void loggedInChanged();
    void authUrlChanged();
    void authCodeChanged();
    void connectionStatusChanged();
    void logLevelChanged();
    void cardReady(BaseCard *card);
    void audioLevelChanged();
    void deviceListChanged();
    void deviceNameChanged();

private:
    static std::unique_ptr<AlexaInterface> instance;
    static int instanceCounter;

    DialogStateManager::DialogState m_dialogState = DialogStateManager::DialogState::Idle;
    AuthManager::AuthState m_authState = AuthManager::AuthState::Uninitialized;
    AuthManager::AuthError m_authError = AuthManager::AuthError::Success;
    bool m_loggedIn = false;
    QUrl m_authUrl;
    QString m_authCode;
    ConnectionManager::ConnectionStatus m_connectionStatus = ConnectionManager::ConnectionStatus::Disconnected;
    LogLevel m_logLevel = LogLevel::Debug9;
    QString m_logLevelString = "DEBUG9";

    /**
     * Initialize a AlexaInterface.
     *
     * @param consoleReader The @c ConsoleReader to read inputs from console.
     * @param configFiles The vector of configuration files.
     * @param pathToInputFolder The path to the inputs folder containing data files needed by this application.
     * @param logLevel The level of logging to enable.  If this parameter is an empty string, the SDK's default
     *     logging level will be used.
     * @return @c true if initialization succeeded, else @c false.
     */
    bool initialize(
        const std::vector<std::string>& configFiles,
        const std::string& pathToInputFolder,
        const std::string& logLevel);

    /**
     * Create an application media player.
     *
     * @param contentFetcherFactory Used to create objects that can fetch remote HTTP content.
     * @param type The type used to categorize the speaker for volume control.
     * @param name The media player instance name used for logging purpose.
     * @return A pointer to the @c ApplicationMediaPlayer and to its speaker if it succeeds; otherwise, return @c
     * nullptr.
     */
    std::pair<std::shared_ptr<ApplicationMediaPlayer>, std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>
    createApplicationMediaPlayer(
        std::shared_ptr<avsCommon::utils::libcurlUtils::HTTPContentFetcherFactory> httpContentFetcherFactory,
        bool enableEqualizer,
        avsCommon::sdkInterfaces::SpeakerInterface::Type type,
        const std::string& name);

    /// The @c InteractionManager which perform user requests.
    std::shared_ptr<InteractionManager> m_interactionManager;

    std::shared_ptr<ObserverManager> m_userInterfaceManager;

    // Authorization Manager
    std::shared_ptr<AuthManager> m_authManager;

    // Connection Manager
    std::shared_ptr<ConnectionManager> m_connectionManager;

    // Dialog State Manager
    std::shared_ptr<DialogStateManager> m_dialogStateManager;

    // Capabilities Manager
    std::shared_ptr<CapabilitiesManager> m_capabilitiesManager;

    // Microphone Wrapper
    std::shared_ptr<QtMicrophoneWrapper> m_micWrapper;

    /// The @c GuiRender which provides an abstraction to visual rendering
    std::shared_ptr<GuiRenderer> m_guiRenderer;

    /// The map of the adapters and their mediaPlayers.
    std::unordered_map<std::string, std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerInterface>>
        m_externalMusicProviderMediaPlayersMap;

    /// The map of the adapters and their mediaPlayers.
    std::unordered_map<std::string, std::shared_ptr<avsCommon::sdkInterfaces::SpeakerInterface>>
        m_externalMusicProviderSpeakersMap;

    /// The vector of mediaPlayers for the adapters.
    std::vector<std::shared_ptr<ApplicationMediaPlayer>> m_adapterMediaPlayers;

    /// The @c MediaPlayer used by @c SpeechSynthesizer.
    std::shared_ptr<ApplicationMediaPlayer> m_speakMediaPlayer;

    /// The @c MediaPlayer used by @c AudioPlayer.
    std::shared_ptr<ApplicationMediaPlayer> m_audioMediaPlayer;

    /// The @c MediaPlayer used by @c Alerts.
    std::shared_ptr<ApplicationMediaPlayer> m_alertsMediaPlayer;

    /// The @c MediaPlayer used by @c NotificationsCapabilityAgent.
    std::shared_ptr<ApplicationMediaPlayer> m_notificationsMediaPlayer;

    /// The @c MediaPlayer used by @c Bluetooth.
    std::shared_ptr<ApplicationMediaPlayer> m_bluetoothMediaPlayer;

    /// The @c CapabilitiesDelegate used by the client.
    std::shared_ptr<alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate> m_capabilitiesDelegate;

    /// The @c MediaPlayer used by @c NotificationsCapabilityAgent.
    std::shared_ptr<ApplicationMediaPlayer> m_ringtoneMediaPlayer;

    using SpeakerTypeAndCreateFunc =
        std::pair<avsCommon::sdkInterfaces::SpeakerInterface::Type, MediaPlayerCreateFunction>;

    /// The singleton map from @c playerId to @c MediaPlayerCreateFunction.
    static std::unordered_map<std::string, SpeakerTypeAndCreateFunc> m_playerToMediaPlayerMap;

    /// The singleton map from @c playerId to @c ExternalMediaAdapter creation functions.
    static capabilityAgents::externalMediaPlayer::ExternalMediaPlayer::AdapterCreationMap m_adapterToCreateFuncMap;

#ifdef KWD
    /// The Wakeword Detector which can wake up the client using audio input.
    std::unique_ptr<kwd::AbstractKeywordDetector> m_keywordDetector;
#endif

#if defined(ANDROID_MEDIA_PLAYER) || defined(ANDROID_MICROPHONE)
    /// The android OpenSL ES engine used to create media players and microphone.
    std::shared_ptr<applicationUtilities::androidUtilities::AndroidSLESEngine> m_openSlEngine;
#endif
};

static QObject *alexaInterfaceSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    AlexaInterface *singletonObject = new AlexaInterface();
    singletonObject->initAlexaQMLClient();
    return singletonObject;
}

#endif  // ALEXA_INTERFACE_H_
