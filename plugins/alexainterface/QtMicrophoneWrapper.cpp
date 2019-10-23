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

#include "QtMicrophoneWrapper.h"
#include <QtEndian>
#include <QDebug>

using alexaClientSDK::avsCommon::avs::AudioInputStream;

static const int NUM_INPUT_CHANNELS = 1;
static const int SAMPLE_RATE = 16000;
static const int SAMPLE_SIZE = 16;

static const double LATENCY = 0.2; //seconds

std::unique_ptr<QtMicrophoneWrapper> QtMicrophoneWrapper::create(std::shared_ptr<AudioInputStream> stream,
                                                                 const QString &deviceName)
{
    if (!stream) {
        qWarning() << "QtMicrophoneWrapper: Invalid stream passed to QtMicrophoneWrapper";
        return nullptr;
    }
    std::unique_ptr<QtMicrophoneWrapper> qtMicrophoneWrapper(new QtMicrophoneWrapper(stream));
    if (!qtMicrophoneWrapper->initialize(deviceName)) {
        qWarning() << "QtMicrophoneWrapper: Failed to initialize QtMicrophoneWrapper";
        return nullptr;
    }
    return qtMicrophoneWrapper;
}

QtMicrophoneWrapper::QtMicrophoneWrapper(std::shared_ptr<AudioInputStream> stream) : m_audioInputStream{stream}
{
}

QtMicrophoneWrapper::~QtMicrophoneWrapper() {
}

bool QtMicrophoneWrapper::initialize(const QString &deviceName) {
    m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        qWarning("QtMicrophoneWrapper: Failed to create stream writer");
        return false;
    }

    setAudioDevice(deviceName);

    return true;
}

bool QtMicrophoneWrapper::startStreamingMicrophoneData() {
    m_readAudioData.clear();
    m_readAudioDataBytes = 0;

    m_audioInputIODevice = m_audioInput->start();

    if (m_audioInput->error() != QAudio::NoError) {
        qWarning() << "Start stream error:" << m_audioInput->error();
        return false;
    }

    if (m_audioInput->state() != QAudio::ActiveState
        && m_audioInput->state() != QAudio::IdleState) {
        qWarning() << "Wrong input state:" << m_audioInput->state();
        return false;
    }

    QByteArray readBytes = m_audioInputIODevice->readAll();

    m_readAudioData.append(readBytes);
    m_readAudioDataBytes += readBytes.count();

    QObject::connect( m_audioInputIODevice, &QIODevice::readyRead, this, [this](){
        QByteArray readBytes = m_audioInputIODevice->readAll();
        m_readAudioData.append(readBytes);
        m_readAudioDataBytes += readBytes.count();
    });
    return true;
}

bool QtMicrophoneWrapper::stopStreamingMicrophoneData() {
    m_audioInput->stop();
    return true;
}

void QtMicrophoneWrapper::setAudioDevice(const QString &deviceName) {
    qDebug() << "Trying to select input device: " << deviceName;

    QAudioFormat format;
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(NUM_INPUT_CHANNELS);
    format.setSampleSize(SAMPLE_SIZE);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo audioInfo = QAudioDeviceInfo::defaultInputDevice();

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    qDebug() << "Available capture devices:" << devices.size();
    for (QAudioDeviceInfo &device : devices) {
        qDebug() << "     device name: " << device.deviceName();
        if (device.deviceName() == deviceName) {
            audioInfo = device;
        }
    }

    qDebug() << "Selected capture device:" << audioInfo.deviceName();
    qDebug() << "Requested format" << format;

    if (!audioInfo.isFormatSupported(format)) {
        qWarning() << "QtMicrophoneWrapper: Default format not supported, trying to use the nearest.";
        format = audioInfo.nearestFormat(format);
        qWarning() << "QtMicrophoneWrapper: Nearest format" << format;
    }

    m_audioInput = new QAudioInput(audioInfo, format, this);

    m_audioLevelInfo.init(m_audioInput->format());

    QObject::connect(m_audioInput, &QAudioInput::notify, this, [this](){

        QByteArray readBytes = m_audioInputIODevice->readAll();
        m_readAudioData.append(readBytes);
        m_readAudioDataBytes += readBytes.count();

        size_t nWords = m_writer->getWordSize() != 0 ?
                static_cast<size_t>(m_readAudioDataBytes)/m_writer->getWordSize() :
                static_cast<size_t>(m_readAudioDataBytes);
        m_writer->write(m_readAudioData.data(), nWords);

        if (m_levelProcess) {
            m_audioLevel = m_audioLevelInfo.processBuffer(m_readAudioData);
            Q_EMIT audioLevelChanged();
        }

        m_readAudioData.clear();
        m_readAudioDataBytes = 0;
    });

    int latency = static_cast<int>(LATENCY * 1000);
    m_audioInput->setNotifyInterval(latency);
    qDebug("QtMicrophoneWrapper: Latency is configured to: %d ms", m_audioInput->notifyInterval());
}

QStringList QtMicrophoneWrapper::deviceList() const
{
    QStringList deviceNames;

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    for (QAudioDeviceInfo &device : devices) {
        deviceNames.append(device.deviceName());
    }

    return deviceNames;
}

AudioLevelInfo::AudioLevelInfo(const QAudioFormat &format)
{
    init(format);
}

bool AudioLevelInfo::init(const QAudioFormat &format)
{
    m_valid = true;
    m_maxAmplitude = 0;

    switch (format.sampleSize()) {
    case 8:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;
    case 32:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            m_maxAmplitude = 0x7fffffff;
            break;
        default:
            break;
        }
        break;
    default:
        m_valid = false;
        return false;
    }

    if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
        m_getAudioLevelValue = &AudioLevelInfo::processUnSignedInt8;
    } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
        m_getAudioLevelValue = &AudioLevelInfo::processSignedInt8;
    } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
        if (format.byteOrder() == QAudioFormat::LittleEndian) {
            m_getAudioLevelValue = &AudioLevelInfo::processUnSignedInt16LE;
        }
        else {
            m_getAudioLevelValue = &AudioLevelInfo::processUnSignedInt16BE;
        }
    } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
        if (format.byteOrder() == QAudioFormat::LittleEndian) {
            m_getAudioLevelValue = &AudioLevelInfo::processSignedInt16LE;
        }
        else{
            m_getAudioLevelValue = &AudioLevelInfo::processSignedInt16BE;
        }
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::UnSignedInt) {
        if (format.byteOrder() == QAudioFormat::LittleEndian) {
            m_getAudioLevelValue = &AudioLevelInfo::processUnSignedInt32LE;
        }
        else {
            m_getAudioLevelValue = &AudioLevelInfo::processUnSignedInt32BE;
        }
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::SignedInt) {
        if (format.byteOrder() == QAudioFormat::LittleEndian) {
            m_getAudioLevelValue = &AudioLevelInfo::processSignedInt32LE;
        }
        else {
            m_getAudioLevelValue = &AudioLevelInfo::processSignedInt32BE;
        }
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
        m_getAudioLevelValue = &AudioLevelInfo::processFloat;
    } else {
        m_getAudioLevelValue = &AudioLevelInfo::processDefault;
        m_valid = false;
    }

    if (format.sampleSize() % 8 != 0) {
        m_valid = false;
    }

    m_channelBytes = format.sampleSize() / 8;
    m_sampleBytes = m_channelBytes * format.channelCount();
    m_channelCount = format.channelCount();

    return m_valid;
}

qreal AudioLevelInfo::processBuffer(const QByteArray &ba) const
{
    if (ba.size() == 0) {
        return 0.0;
    }

    if (m_valid) {
        if (ba.size() % m_sampleBytes != 0)
            return 0.0;

        const int numSamples = ba.size() / m_sampleBytes;
        const int step = numSamples / 50 + 1; //check 50 samples from buffer, skip all other info

        quint32 maxValue = 0;
        const char *ptr = ba.constData();

        for (int i = 0; i < numSamples; i += step) {
            for (int j = 0; j < m_channelCount; ++j) {
                quint32 value = (*m_getAudioLevelValue)(ptr);
                maxValue = qMax(value, maxValue);
                ptr += m_channelBytes;
            }
            ptr += m_channelBytes * (m_channelCount) * (step - 1);
        }

        maxValue = qMin(maxValue, m_maxAmplitude);
        return qreal(maxValue) / m_maxAmplitude;
    }

    return 0.0;
}

quint32 AudioLevelInfo::processUnSignedInt8(const char *ptr)
{
    return *reinterpret_cast<const quint8*>(ptr);
}

quint32 AudioLevelInfo::processSignedInt8(const char *ptr)
{
    return static_cast<quint32>(qAbs(*reinterpret_cast<const qint8*>(ptr)));
}

quint32 AudioLevelInfo::processUnSignedInt16LE(const char *ptr)
{
    return qFromLittleEndian<quint16>(ptr);
}

quint32 AudioLevelInfo::processUnSignedInt16BE(const char *ptr)
{
    return qFromBigEndian<quint16>(ptr);
}

quint32 AudioLevelInfo::processSignedInt16LE(const char *ptr)
{
    return static_cast<quint32>(qAbs(qFromLittleEndian<qint16>(ptr)));
}

quint32 AudioLevelInfo::processSignedInt16BE(const char *ptr)
{
    return static_cast<quint32>(qAbs(qFromBigEndian<qint16>(ptr)));
}

quint32 AudioLevelInfo::processUnSignedInt32LE(const char *ptr)
{
    return qFromLittleEndian<quint32>(ptr);
}

quint32 AudioLevelInfo::processUnSignedInt32BE(const char *ptr)
{
    return qFromBigEndian<quint32>(ptr);
}

quint32 AudioLevelInfo::processSignedInt32LE(const char *ptr)
{
    return static_cast<quint32>(qAbs(qFromLittleEndian<qint32>(ptr)));
}

quint32 AudioLevelInfo::processSignedInt32BE(const char *ptr)
{
    return static_cast<quint32>(qAbs(qFromBigEndian<qint32>(ptr)));
}

quint32 AudioLevelInfo::processFloat(const char *ptr)
{
    return static_cast<quint32>(qAbs(*reinterpret_cast<const float*>(ptr) * 0x7fffffff)); // assumes 0-1.0
}
quint32 AudioLevelInfo::processDefault(const char *ptr)
{
    Q_UNUSED(ptr)
    return 0;
}
