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

#ifndef ALEXAINTERFACE_QTMICROPHONEWRAPPER_H
#define ALEXAINTERFACE_QTMICROPHONEWRAPPER_H

#include <AVSCommon/AVS/AudioInputStream.h>
#include <Audio/MicrophoneInterface.h>

#include <QAudioInput>
#include <QIODevice>

using namespace alexaClientSDK;

/**
 * @brief The AudioLevelInfo class
 *
 * provides processBuffer() function to get avg audio level from QByteArray data
 * should be initialized with QAudioFormat in constructor or by init() function
 *
 */
class AudioLevelInfo
{
public:
    explicit AudioLevelInfo(const QAudioFormat &format);
    AudioLevelInfo() {}
    bool init(const QAudioFormat &format);

    qreal processBuffer(const QByteArray &ba) const;
private:
    bool m_valid = false;

    /// format-specific for processing speed-up
    quint32 m_maxAmplitude = 0;
    int m_sampleBytes = 0;
    int m_channelBytes = 0;
    int m_channelCount = 0;

    /// pointer to static member function chosen for format
    quint32 (*m_getAudioLevelValue)(const char *) = &AudioLevelInfo::processDefault;

    /// buffer process functions
    static quint32 processUnSignedInt8(const char *ptr);
    static quint32 processSignedInt8(const char *ptr);
    static quint32 processUnSignedInt16LE(const char *ptr);
    static quint32 processUnSignedInt16BE(const char *ptr);
    static quint32 processSignedInt16LE(const char *ptr);
    static quint32 processSignedInt16BE(const char *ptr);
    static quint32 processUnSignedInt32LE(const char *ptr);
    static quint32 processUnSignedInt32BE(const char *ptr);
    static quint32 processSignedInt32LE(const char *ptr);
    static quint32 processSignedInt32BE(const char *ptr);
    static quint32 processFloat(const char *ptr);
    static quint32 processDefault(const char *ptr);
};

class QtMicrophoneWrapper
      : public QObject
      , public applicationUtilities::resources::audio::MicrophoneInterface {
    Q_OBJECT
public:
    /**
     * Creates a @c QtMicrophoneWrapper.
     *
     * @param stream The shared data stream to write to.
     * @return A unique_ptr to a @c QtMicrophoneWrapper if creation was successful and @c nullptr otherwise.
     */
    static std::unique_ptr<QtMicrophoneWrapper> create(std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
                                                       const QString &deviceName);

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

    virtual ~QtMicrophoneWrapper() override;

    qreal audioLevel() const { return m_audioLevel; }
    void setLevelProcess(bool enable) { m_levelProcess = enable; }
    QStringList deviceList() const;
    QString deviceName() const { return m_deviceName; }
    void setDeviceName(const QString &audioDeviceName);

Q_SIGNALS:
    void audioLevelChanged();
    void deviceNameChanged();

private:
    /**
     * Constructor.
     * @param stream The shared data stream to write to.
     */
    QtMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);

    QAudioInput *m_audioInput = nullptr;
    QIODevice *m_audioInputIODevice = nullptr;
    int m_readAudioDataBytes = 0;
    QByteArray m_readAudioData;
    qreal m_audioLevel = 0.0; // 0.0 <= m_audioLevel <= 1.0
    bool m_levelProcess = false;
    AudioLevelInfo m_audioLevelInfo;
    QString m_deviceName;


    /// Initializes Audio
    bool initialize(const QString &deviceName);
    void setAudioDevice(const QString &audioDeviceName);

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_audioInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_writer;
};

#endif  // ALEXAINTERFACE_QTMICROPHONEWRAPPER_H
