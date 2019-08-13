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

#ifndef ALEXAINTERFACE_GUIRENDERER_H
#define ALEXAINTERFACE_GUIRENDERER_H

#include <AVSCommon/SDKInterfaces/TemplateRuntimeObserverInterface.h>
#include <map>
#include <string>

#include <QObject>

using namespace alexaClientSDK;

/**
 * A class that implements the TemplateRuntimeObserverInterface.  Instead of rendering the
 * display cards, this class will print out some useful information (e.g. JSON payload)
 * when the renderTemplateCard or renderPlayerInfoCard callbacks are called.
 *
 * It is also used to track the PlayerInfo controls, and provide the PlayerInfo toggle states.
 *
 * @note Due to the payload in RenderTemplate may contain sensitive information, the
 * payload will only be printed if @c ACSDK_EMIT_SENSITIVE_LOGS is ON.
 */
class GuiRenderer
        : public QObject
        , public avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface {
    Q_OBJECT
public:
    explicit GuiRenderer(QObject* parent = nullptr);

    /// @name TemplateRuntimeObserverInterface Functions
    /// @{

    void renderTemplateCard(const std::string& jsonPayload, avsCommon::avs::FocusState focusState) override;
    void clearTemplateCard() override;

    void renderPlayerInfoCard(const std::string& jsonPayload, TemplateRuntimeObserverInterface::AudioPlayerInfo info,
                              avsCommon::avs::FocusState focusState) override;
    void clearPlayerInfoCard() override;

    /// @}

    /// returns the PlayerInfo toggle state
    bool getGuiToggleState(const std::string& toggleName);

    /// String to identify the AVS action SELECTED string.
    static const std::string TOGGLE_ACTION_SELECTED;
    /// String to identify the AVS action DESELECTED string.
    static const std::string TOGGLE_ACTION_DESELECTED;
    /// String to identify the AVS name SHUFFLE string.
    static const std::string TOGGLE_NAME_SHUFFLE;
    /// String to identify the AVS name LOOP string.
    static const std::string TOGGLE_NAME_LOOP;
    /// String to identify the AVS name REPEAT string.
    static const std::string TOGGLE_NAME_REPEAT;
    /// String to identify the AVS name THUMBS_UP string.
    static const std::string TOGGLE_NAME_THUMBSUP;
    /// String to identify the AVS name THUMBS_DOWN string.
    static const std::string TOGGLE_NAME_THUMBSDOWN;

private:
    // Last known toggle states
    std::map<std::string, bool> m_guiToggleStateMap;

Q_SIGNALS:
    void templateCardContentReady(QString jsonString);
    void playerCardContentReady(QString jsonString);

};

#endif  // ALEXAINTERFACE_GUIRENDERER_H
