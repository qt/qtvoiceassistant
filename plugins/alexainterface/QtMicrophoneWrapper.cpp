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

    m_audioInfo = QAudioDeviceInfo::defaultInputDevice();

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    qDebug() << "Available capture devices:" << devices.size();
    for (QAudioDeviceInfo &device : devices) {
        qDebug() << "     device name: " << device.deviceName();
        if (device.deviceName() == deviceName) {
            m_audioInfo = device;
        }
    }

    qDebug() << "Selected capture device:" << m_audioInfo.deviceName();
    qDebug() << "Requested format" << format;

    if (!m_audioInfo.isFormatSupported(format)) {
        qWarning() << "QtMicrophoneWrapper: Default format not supported, trying to use the nearest.";
        format = m_audioInfo.nearestFormat(format);
        qWarning() << "QtMicrophoneWrapper: Nearest format" << format;
    }

    m_audioInput = new QAudioInput(m_audioInfo, format, this);
    QObject::connect(m_audioInput, &QAudioInput::notify, this, [this](){

        QByteArray readBytes = m_audioInputIODevice->readAll();
        m_readAudioData.append(readBytes);
        m_readAudioDataBytes += readBytes.count();

        size_t nWords = m_writer->getWordSize() != 0 ?
                static_cast<size_t>(m_readAudioDataBytes)/m_writer->getWordSize() :
                static_cast<size_t>(m_readAudioDataBytes);
        m_writer->write(m_readAudioData.data(), nWords);
        m_readAudioData.clear();
        m_readAudioDataBytes = 0;
    });

    int latency = static_cast<int>(LATENCY * 1000);
    m_audioInput->setNotifyInterval(latency);
    qDebug("QtMicrophoneWrapper: Latency is configured to: %d ms", m_audioInput->notifyInterval());
}
