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

#include <iostream>
#include <sstream>

#include <AVSCommon/Utils/SDKVersion.h>

#include "ObserverManager.h"

#include <QDebug>

using namespace alexaClientSDK::avsCommon::sdkInterfaces;

ObserverManager::ObserverManager(QObject *parent) : QObject (parent)
{
}

void ObserverManager::onSettingChanged(const std::string& key, const std::string& value) {
    m_executor.submit([key, value]() {
        std::string msg = key + " set to " + value;
        qDebug("SETTINGS: %s set to %s", key.c_str(), value.c_str());
        //TODO: Add a signal here
        //TODO: should be move in a separate class
    });
}

void ObserverManager::onSpeakerSettingsChanged( const SpeakerManagerObserverInterface::Source& source,
                                                const SpeakerInterface::Type& type,
                                                const SpeakerInterface::SpeakerSettings& settings) {
    m_executor.submit([source, type, settings]() {
        std::ostringstream oss;
        oss << "SOURCE:" << source << " TYPE:" << type << " VOLUME:" << static_cast<int>(settings.volume) << " MUTE:" << settings.mute;
        qDebug() << oss.str().c_str();
        //TODO: Add a signal here
        //TODO: should be move in a separate class
    });
}

void ObserverManager::onSetIndicator(avsCommon::avs::IndicatorState state) {
    m_executor.submit([state]() {
        qDebug("Notification indicator state: %d", state);
        //TODO: Emit a signal if this state is needed
    });
}

//TODO: Would be good to split this into many different classes which inherit every single interface from the SDK
//or at least two or three with similar meanings but not a bunch of those in the same class as it was done in the Alexa Interface
