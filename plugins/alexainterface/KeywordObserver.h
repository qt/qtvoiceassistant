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

#ifndef VW_KEYWORDOBSERVER_H_
#define VW_KEYWORDOBSERVER_H_

#include <memory>
#include <string>

#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <DefaultClient/DefaultClient.h>
#include <ESP/ESPDataProviderInterface.h>

#include <QObject>

using namespace alexaClientSDK;

/**
 * Observes callbacks from keyword detections and notifies the DefaultClient that a wake word has occurred.
 */
class KeywordObserver
        : public QObject
        , public avsCommon::sdkInterfaces::KeyWordObserverInterface {
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * @param client The default SDK client.
     * @param audioProvider The audio provider from which to stream audio data from.
     * @parm espProvider The ESP provider to calculate the Ambient and Voice energy from the audio stream.
     */
    explicit KeywordObserver(
        QObject* parent,
        std::shared_ptr<defaultClient::DefaultClient> client,
        capabilityAgents::aip::AudioProvider audioProvider,
        std::shared_ptr<esp::ESPDataProviderInterface> espProvider = nullptr);

    /// @name KeyWordObserverInterface Functions
    /// @{
    void onKeyWordDetected(
        std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
        std::string keyword,
        avsCommon::avs::AudioInputStream::Index beginIndex = UNSPECIFIED_INDEX,
        avsCommon::avs::AudioInputStream::Index endIndex = UNSPECIFIED_INDEX,
        std::shared_ptr<const std::vector<char>> KWDMetadata = nullptr) override;
    /// @}

Q_SIGNALS:
    void keyWordDetected();
    void tapToTalkDetected();

private:
    /// The default SDK client.
    std::shared_ptr<defaultClient::DefaultClient> m_client;

    /// The audio provider.
    capabilityAgents::aip::AudioProvider m_audioProvider;

    /// Echo Spatial Perception (ESP) provider.
    std::shared_ptr<esp::ESPDataProviderInterface> m_espProvider;
};


#endif  // VW_KEYWORDOBSERVER_H_
