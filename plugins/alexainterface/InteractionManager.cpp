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

#include "ESP/ESPDataProviderInterface.h"
#include "RegistrationManager/CustomerDataManager.h"
#include "InteractionManager.h"

#include <QDebug>

using namespace alexaClientSDK::avsCommon::avs;

InteractionManager::InteractionManager(
        QObject* parent,
        std::shared_ptr<defaultClient::DefaultClient> client,
        std::shared_ptr<applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
        std::shared_ptr<ObserverManager> userInterface,
        capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
        capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
        std::shared_ptr<GuiRenderer> guiRenderer,
        capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
        std::shared_ptr<esp::ESPDataProviderInterface> espProvider,
        std::shared_ptr<esp::ESPDataModifierInterface> espModifier,
        std::shared_ptr<avsCommon::sdkInterfaces::CallManagerInterface> callManager) :
    QObject(parent),
    RequiresShutdown{"InteractionManager"},
    m_client{client},
    m_micWrapper{micWrapper},
    m_userInterface{userInterface},
    m_guiRenderer{guiRenderer},
    m_espProvider{espProvider},
    m_espModifier{espModifier},
    m_callManager{callManager},
    m_holdToTalkAudioProvider{holdToTalkAudioProvider},
    m_tapToTalkAudioProvider{tapToTalkAudioProvider},
    m_wakeWordAudioProvider{wakeWordAudioProvider},
    m_isHoldOccurring{false},
    m_isTapOccurring{false},
    m_isMicOn{true} {
    m_micWrapper->startStreamingMicrophoneData();
};

void InteractionManager::begin() {
    m_executor.submit([this]() {
        Q_EMIT interactionStarted();
    });
}



void InteractionManager::changeSetting(const std::string& key, const std::string& value) {
    m_executor.submit([this, key, value]() { m_client->changeSetting(key, value); });
}

void InteractionManager::microphoneToggle() {
    m_executor.submit([this]() {
        if (!m_wakeWordAudioProvider) {
            return;
        }
        if (m_isMicOn) {
            m_isMicOn = false;
            m_micWrapper->stopStreamingMicrophoneData();
        } else {
            m_isMicOn = true;
            m_micWrapper->startStreamingMicrophoneData();
        }
    });
}

void InteractionManager::holdToggled() {
    m_executor.submit([this]() {
        if (!m_isMicOn) {
            return;
        }
        if (!m_isHoldOccurring) {
            if (m_client->notifyOfHoldToTalkStart(m_holdToTalkAudioProvider).get()) {
                m_isHoldOccurring = true;
            }
        } else {
            m_isHoldOccurring = false;
            m_client->notifyOfHoldToTalkEnd();
        }
    });
}

void InteractionManager::tap() {
    m_executor.submit([this]() {
        if (!m_isMicOn) {
            return;
        }
        if (!m_isTapOccurring) {
            if (m_client->notifyOfTapToTalk(m_tapToTalkAudioProvider).get()) {
                m_isTapOccurring = true;
            }
        } else {
            m_isTapOccurring = false;
            m_client->notifyOfTapToTalkEnd();
        }
    });
}

void InteractionManager::stopForegroundActivity() {
    m_executor.submit([this]() { m_client->stopForegroundActivity(); });
}

void InteractionManager::playbackPlay() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::PLAY); });
}

void InteractionManager::playbackPause() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::PAUSE); });
}

void InteractionManager::playbackNext() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::NEXT); });
}

void InteractionManager::playbackPrevious() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::PREVIOUS); });
}

void InteractionManager::playbackSkipForward() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::SKIP_FORWARD); });
}

void InteractionManager::playbackSkipBackward() {
    m_executor.submit([this]() { m_client->getPlaybackRouter()->buttonPressed(PlaybackButton::SKIP_BACKWARD); });
}

void InteractionManager::playbackShuffle() {
    sendGuiToggleEvent(GuiRenderer::TOGGLE_NAME_SHUFFLE, PlaybackToggle::SHUFFLE);
    //TODO:
}

void InteractionManager::playbackLoop() {
    sendGuiToggleEvent(GuiRenderer::TOGGLE_NAME_LOOP, PlaybackToggle::LOOP);
    //TODO:
}

void InteractionManager::playbackRepeat() {
    sendGuiToggleEvent(GuiRenderer::TOGGLE_NAME_REPEAT, PlaybackToggle::REPEAT);
    //TODO:
}

void InteractionManager::playbackThumbsUp() {
    sendGuiToggleEvent(GuiRenderer::TOGGLE_NAME_THUMBSUP, PlaybackToggle::THUMBS_UP);
    //TODO:
}

void InteractionManager::playbackThumbsDown() {
    sendGuiToggleEvent(GuiRenderer::TOGGLE_NAME_THUMBSDOWN, PlaybackToggle::THUMBS_DOWN);
    //TODO:
}

//TODO:

void InteractionManager::sendGuiToggleEvent(const std::string& toggleName, PlaybackToggle toggleType) {
    bool action = false;
    if (m_guiRenderer) {
        action = !m_guiRenderer->getGuiToggleState(toggleName);
    }
    m_executor.submit(
                [this, toggleType, action]() { m_client->getPlaybackRouter()->togglePressed(toggleType, action); });
}



void InteractionManager::setFirmwareVersion(avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) {
    m_executor.submit([this, firmwareVersion]() { m_client->setFirmwareVersion(firmwareVersion); });
}


void InteractionManager::adjustVolume(avsCommon::sdkInterfaces::SpeakerInterface::Type type, int8_t delta) {
    m_executor.submit([this, type, delta]() {
        /*
         * Group the unmute action as part of the same affordance that caused the volume change, so we don't
         * send another event. This isn't a requirement by AVS.
         */
        std::future<bool> unmuteFuture = m_client->getSpeakerManager()->setMute(type, false, true);
        if (!unmuteFuture.valid()) {
            return;
        }
        unmuteFuture.get();

        std::future<bool> future = m_client->getSpeakerManager()->adjustVolume(type, delta);
        if (!future.valid()) {
            return;
        }
        future.get();
    });
}

void InteractionManager::setMute(avsCommon::sdkInterfaces::SpeakerInterface::Type type, bool mute) {
    m_executor.submit([this, type, mute]() {
        std::future<bool> future = m_client->getSpeakerManager()->setMute(type, mute);
        future.get();
    });
}


void InteractionManager::resetDevice() {
    // This is a blocking operation. No interaction will be allowed during / after resetDevice
    auto result = m_executor.submit([this]() {
        m_client->getRegistrationManager()->logout();
    });
    result.wait();
}

void InteractionManager::espControl() {
    m_executor.submit([this]() {
        if (m_espProvider) {
            auto espData = m_espProvider->getESPData();
            qDebug() << Q_FUNC_INFO;
            qDebug() << "ESP Enabled:" << m_espProvider->isEnabled();
            qDebug() << "ESP: Voice energy" << espData.getVoiceEnergy().c_str();
            qDebug() << "ESP: Ambieny energy" << espData.getAmbientEnergy().c_str();
        } else {
            qWarning() << Q_FUNC_INFO << "ESP is not supported in this device";
        }
    });
}

void InteractionManager::toggleESPSupport() {
    m_executor.submit([this]() {
        if (m_espProvider) {
            m_espProvider->isEnabled() ? m_espProvider->disable() : m_espProvider->enable();
        } else {
            qWarning() << Q_FUNC_INFO << "ESP is not supported in this device";
        }
    });
}

void InteractionManager::setESPVoiceEnergy(const std::string& voiceEnergy) {
    m_executor.submit([this, voiceEnergy]() {
        if (m_espProvider) {
            if (m_espModifier) {
                m_espModifier->setVoiceEnergy(voiceEnergy);
            } else {
                qWarning() << Q_FUNC_INFO << "Cannot override ESP Value in this device.";
            }
        } else {
            qWarning() << Q_FUNC_INFO << "ESP is not supported in this device";
        }
    });
}

void InteractionManager::setESPAmbientEnergy(const std::string& ambientEnergy) {
    m_executor.submit([this, ambientEnergy]() {
        if (m_espProvider) {
            if (m_espModifier) {
                m_espModifier->setAmbientEnergy(ambientEnergy);
            } else {
                qWarning() << Q_FUNC_INFO << "Cannot override ESP Value in this device.";
            }
        } else {
            qWarning() << Q_FUNC_INFO << "ESP is not supported in this device";
        }
    });
}

void InteractionManager::commsControl() {
    m_executor.submit([this]() {
        if (m_client->isCommsEnabled()) {
            //m_userInterface->printCommsControlScreen();
            //Press 'a' followed by Enter to accept an incoming call.
            //Press 's' followed by Enter to stop an ongoing call.
        } else {
            qWarning() << Q_FUNC_INFO << "Comms is not supported in this device.";
        }
    });
}

void InteractionManager::acceptCall() {
    m_executor.submit([this]() {
        if (m_client->isCommsEnabled()) {
            m_client->acceptCommsCall();
        } else {
            qWarning() << Q_FUNC_INFO << "Comms is not supported in this device.";
        }
    });
}

void InteractionManager::stopCall() {
    m_executor.submit([this]() {
        if (m_client->isCommsEnabled()) {
            m_client->stopCommsCall();
        } else {
            qWarning() << Q_FUNC_INFO << "Comms is not supported in this device.";
        }
    });
}

void InteractionManager::onDialogUXStateChanged(DialogUXState state) {
    // reset tap-to-talk state
    if (DialogUXState::LISTENING != state) {
        m_isTapOccurring = false;
    }
}

void InteractionManager::doShutdown() {
    m_client.reset();
}

