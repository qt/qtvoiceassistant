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

import application.windows 1.0

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import alexaauth 1.0
import alexainterface 1.0

import shared.Sizes 1.0
import shared.Style 1.0
import shared.controls 1.0
import shared.utils 1.0

ApplicationCCWindow {
    id: root

    Image {
        id: headerImage
        x: root.exposedRect.x
        y: 0
        width: root.exposedRect.width
        height: Sizes.dp(436) + exposedRect.y
        fillMode: Image.Pad

        source: "assets/alexa_background.png"
        asynchronous: true
        opacity: root.neptuneState === "Maximized" ? 1 : 0
        visible: opacity > 0
    }

    Item {
        id: mainWindow
        x: root.exposedRect.x
        y: root.exposedRect.y
        width: root.exposedRect.width
        height: root.exposedRect.height

        AlexaAuth {
            id: alexaAuth
            httpUserAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.119 Safari/537.36"
        }

        AlexaInterface {
            id: alexa
            logLevel: AlexaInterface.Info
            onAuthCodeChanged: {
                if (authCode !== "") {
                    alexaAuth.authCode = authCode
                }
            }
            onAuthUrlChanged: {
                alexaAuth.authUrl = authUrl
            }
        }

        Header {
            anchors.top: parent.top
            anchors.topMargin: Sizes.dp(80)
            anchors.horizontalCenter: parent.horizontalCenter
            unfoldHeader: alexaView.visible || authView.visible
            alexaInterface: alexa
            visible: root.neptuneState === "Maximized"
        }

        Item {
            id: paneMainView
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - Sizes.dp(50)

            AlexaView {
                id: alexaView
                anchors.fill: parent
                alexaInterface: alexa
                visible: alexaInterface.authState === AlexaInterface.Refreshed
                neptuneState: root.neptuneState
            }

            AuthView {
                id: authView
                anchors.fill: parent
                alexaInterface: alexa
                alexaAuth: alexaAuth
                visible: alexaInterface.authState !== AlexaInterface.Refreshed
            }
        }

        Component.onCompleted: alexa.initAlexaQMLClient()
    }
}
