/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 IVI UI.
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

#ifndef VW_PORTAUDIOMICROPHONEWRAPPER_H_
#define VW_PORTAUDIOMICROPHONEWRAPPER_H_

#include <mutex>
#include <thread>

#include <AVSCommon/AVS/AudioInputStream.h>

#include <portaudio.h>
#include <Audio/MicrophoneInterface.h>

using namespace alexaClientSDK;

/// This acts as a wrapper around PortAudio, a cross-platform open-source audio I/O library.
class PortAudioMicrophoneWrapper : public applicationUtilities::resources::audio::MicrophoneInterface {
public:
    /**
     * Creates a @c PortAudioMicrophoneWrapper.
     *
     * @param stream The shared data stream to write to.
     * @return A unique_ptr to a @c PortAudioMicrophoneWrapper if creation was successful and @c nullptr otherwise.
     */
    static std::unique_ptr<PortAudioMicrophoneWrapper> create(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);

    /**
     * Stops streaming from the microphone.
     *
     * @return Whether the stop was successful.
     */
    bool stopStreamingMicrophoneData() override;

    /**
     * Starts streaming from the microphone.
     *
     * @return Whether the start was successful.
     */
    bool startStreamingMicrophoneData() override;

    /**
     * Destructor.
     */
    ~PortAudioMicrophoneWrapper() override;

private:
    /**
     * Constructor.
     *
     * @param stream The shared data stream to write to.
     */
    PortAudioMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);

    /**
     * The callback that PortAudio will issue when audio is available to read.
     *
     * @param inputBuffer The temporary buffer that microphone audio data will be available in.
     * @param outputBuffer Not used here.
     * @param numSamples The number of samples available to consume.
     * @param timeInfo Time stamps indicated when the first sample in the buffer was captured. Not used here.
     * @param statusFlags Flags that tell us when underflow or overflow conditions occur. Not used here.
     * @param userData A user supplied pointer.
     * @return A PortAudio code that will indicate how PortAudio should continue.
     */
    static int PortAudioCallback(
        const void* inputBuffer,
        void* outputBuffer,
        unsigned long numSamples,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    /// Initializes PortAudio
    bool initialize();

    /**
     * Get the optional config parameter from @c AlexaClientSDKConfig.json
     * for setting the PortAudio stream's suggested latency.
     *
     * @param[out] suggestedLatency The latency as it is configured in the file.
     * @return  @c true if the suggestedLatency is defined in the config file, @c false otherwise.
     */
    bool getConfigSuggestedLatency(PaTime& suggestedLatency);

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_audioInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_writer;

    /// The PortAudio stream
    PaStream* m_paStream;

    /**
     * A lock to seralize access to startStreamingMicrophoneData() and stopStreamingMicrophoneData() between different
     * threads.
     */
    std::mutex m_mutex;
};

#endif  // VW_PORTAUDIOMICROPHONEWRAPPER_H_
