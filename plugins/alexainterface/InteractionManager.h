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

#ifndef ALEXAINTERFACE_INTERACTIONMANAGER_H_
#define ALEXAINTERFACE_INTERACTIONMANAGER_H_

#include <memory>

#include <Audio/MicrophoneInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <DefaultClient/DefaultClient.h>
#include <ESP/ESPDataModifierInterface.h>
#include <RegistrationManager/CustomerDataManager.h>

#include "KeywordObserver.h"
#include "ObserverManager.h"
#include "GuiRenderer.h"

#include <QObject>


using namespace alexaClientSDK;

/**
 * This class manages most of the user interaction by taking in commands and notifying the DefaultClient and the
 * userInterface (the view) accordingly.
 */
class InteractionManager
        : QObject
        , public avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public avsCommon::utils::RequiresShutdown {
    Q_OBJECT
public:

    explicit InteractionManager(
            QObject* parent,
            std::shared_ptr<defaultClient::DefaultClient> client,
            std::shared_ptr<applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
            std::shared_ptr<ObserverManager> userInterface, //TODO: Is it really needed here ?
            capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
            capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
            std::shared_ptr<GuiRenderer> guiRenderer = nullptr,
            capabilityAgents::aip::AudioProvider wakeWordAudioProvider = capabilityAgents::aip::AudioProvider::null(),
            std::shared_ptr<esp::ESPDataProviderInterface> espProvider = nullptr,
            std::shared_ptr<esp::ESPDataModifierInterface> espModifier = nullptr,
            std::shared_ptr<avsCommon::sdkInterfaces::CallManagerInterface> callManager = nullptr);

    /**
     * Begins the interaction between the Alexa Interface and the user. This should only be called at startup.
     */
    void begin();



    /**
     * Toggles the microphone state if the Alexa Interface was built with wakeword. When the microphone is turned off, the
     * app enters a privacy mode in which it stops recording audio data from the microphone, thus disabling Alexa waking
     * up due to wake word. Note however that hold-to-talk and tap-to-talk modes will still work by recording
     * microphone data temporarily until a user initiated interacion is complete. If this app was built without wakeword
     * then this will do nothing as the microphone is already off.
     */
    void microphoneToggle();

    /**
     * Should be called whenever a user presses or releases the hold button.
     */
    void holdToggled();

    /**
     * Should be called whenever a user presses and releases the tap button.
     */
    void tap();

    /**
     * Acts as a "stop" button. This stops whatever has foreground focus.
     */
    void stopForegroundActivity();

    /**
     * Should be called whenever a user presses 'PLAY' for playback.
     */
    void playbackPlay();

    /**
     * Should be called whenever a user presses 'PAUSE' for playback.
     */
    void playbackPause();

    /**
     * Should be called whenever a user presses 'NEXT' for playback.
     */
    void playbackNext();

    /**
     * Should be called whenever a user presses 'PREVIOUS' for playback.
     */
    void playbackPrevious();

    /**
     * Should be called whenever a user presses 'SKIP_FORWARD' for playback.
     */
    void playbackSkipForward();

    /**
     * Should be called whenever a user presses 'SKIP_BACKWARD' for playback.
     */
    void playbackSkipBackward();

    /**
     * Should be called whenever a user presses 'SHUFFLE' for playback.
     */
    void playbackShuffle();

    /**
     * Should be called whenever a user presses 'LOOP' for playback.
     */
    void playbackLoop();

    /**
     * Should be called whenever a user presses 'REPEAT' for playback.
     */
    void playbackRepeat();

    /**
     * Should be called whenever a user presses 'THUMBS_UP' for playback.
     */
    void playbackThumbsUp();

    /**
     * Should be called whenever a user presses 'THUMBS_DOWN' for playback.
     */
    void playbackThumbsDown();

    /**
     * Should be called when setting value is selected by the user.
     */
    void changeSetting(const std::string& key, const std::string& value);

    /**
     * Update the firmware version.
     *
     * @param firmwareVersion The new firmware version.
     */
    void setFirmwareVersion(avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion);


    /**
     * Should be called after a user wishes to modify the volume.
     */
    void adjustVolume(avsCommon::sdkInterfaces::SpeakerInterface::Type type, int8_t delta);

    /**
     * Should be called after a user wishes to set mute.
     */
    void setMute(avsCommon::sdkInterfaces::SpeakerInterface::Type type, bool mute);

    /**
     * Reset the device and remove any customer data.
     */
    void resetDevice();


    /**
     * Should be called whenever a user requests for ESP control.
     */
    void espControl();

    /**
     * Should be called whenever a user requests to toggle the ESP support.
     */
    void toggleESPSupport();

    /**
     * Should be called whenever a user requests to set the @c voiceEnergy sent in ReportEchoSpatialPerceptionData
     * event.
     *
     * @param voiceEnergy The voice energy measurement to be set as the ESP measurement.
     */
    void setESPVoiceEnergy(const std::string& voiceEnergy);

    /**
     * Should be called whenever a user requests set the @c ambientEnergy sent in ReportEchoSpatialPerceptionData
     * event.
     *
     * @param ambientEnergy The ambient energy measurement to be set as the ESP measurement.
     */
    void setESPAmbientEnergy(const std::string& ambientEnergy);

    /**
     * Grants the user access to the communications controls.
     */
    void commsControl();

    /**
     * Should be called when the user wants to accept a call.
     */
    void acceptCall();

    /**
     * Should be called when the user wants to stop a call.
     */
    void stopCall();

    /**
     * UXDialogObserverInterface methods
     */
    void onDialogUXStateChanged(DialogUXState newState) override;

Q_SIGNALS:

    void interactionStarted();

private:
    /// The default SDK client.
    std::shared_ptr<defaultClient::DefaultClient> m_client;

    /// The microphone managing object.
    std::shared_ptr<applicationUtilities::resources::audio::MicrophoneInterface> m_micWrapper;

    /// The user interface manager.
    std::shared_ptr<ObserverManager> m_userInterface;

    /// The gui renderer.
    std::shared_ptr<GuiRenderer> m_guiRenderer;

    /// The ESP provider.
    std::shared_ptr<esp::ESPDataProviderInterface> m_espProvider;

    /// The ESP modifier.
    std::shared_ptr<esp::ESPDataModifierInterface> m_espModifier;

    /// The call manager.
    std::shared_ptr<avsCommon::sdkInterfaces::CallManagerInterface> m_callManager;

    /// The hold to talk audio provider.
    capabilityAgents::aip::AudioProvider m_holdToTalkAudioProvider;

    /// The tap to talk audio provider.
    capabilityAgents::aip::AudioProvider m_tapToTalkAudioProvider;

    /// The wake word audio provider.
    capabilityAgents::aip::AudioProvider m_wakeWordAudioProvider;

    /// Whether a hold is currently occurring.
    bool m_isHoldOccurring;

    /// Whether a tap is currently occurring.
    bool m_isTapOccurring;

    /// Whether the microphone is currently turned on.
    bool m_isMicOn;

    /**
     * An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
     */
    avsCommon::utils::threading::Executor m_executor;

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /// sends Gui Toggle event
    void sendGuiToggleEvent(const std::string& toggleName, avsCommon::avs::PlaybackToggle toggleType);
};


#endif  // ALEXAINTERFACE_INTERACTIONMANAGER_H_
