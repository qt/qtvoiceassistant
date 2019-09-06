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

import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import alexainterface 1.0
import alexaauth 1.0

import application.windows 1.0

import shared.Sizes 1.0
import shared.Style 1.0
import shared.controls 1.0
import shared.utils 1.0


Item {
    id: root

    property string neptuneState: "Maximized"

    AuthWebPageInteraction {
        id: alexaAuth
        onErrorChanged: {
            if (error === AlexaAuth.AutomaticAuthFailed){
                authView.state = "manual_auth"
            }
        }
    }

    Connections {
        target: AlexaInterface
        onAuthCodeChanged: {
            if (AlexaInterface.authCode !== "") {
                alexaAuth.authCode = AlexaInterface.authCode
            }
        }
        onAuthUrlChanged: {
            alexaAuth.authUrl = AlexaInterface.authUrl
        }
        Component.onCompleted: {
            AlexaInterface.logLevel = Alexa.Debug9
        }
    }

    Header {
        id: header
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(80)
        width: parent.width
        height: Sizes.dp(356)
        anchors.horizontalCenter: parent.horizontalCenter
        unfoldHeader: alexaView.visible || authView.visible
        visible: root.neptuneState === "Maximized"
    }

    Item {
        id: paneMainView
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // to not overlap content with on-top widget
        height: parent.height - (header.y + header.height)

        AlexaView {
            id: alexaView
            anchors.fill: parent
            visible: AlexaInterface.authState === Alexa.Refreshed
            neptuneState: root.neptuneState
        }

        AuthView {
            id: authView
            anchors.fill: parent
            alexaAuth: alexaAuth
            visible: AlexaInterface.authState !== Alexa.Refreshed
        }
    }

    property NeptuneWindow statusBar: NeptuneWindow {
        width: Sizes.dp(Config.statusBarHeight)
        height: Sizes.dp(Config.statusBarHeight)
        Component.onCompleted: {
            setWindowProperty("windowType", "statusbar")
        }
        Item {
            id: interactionPane
            anchors.fill: parent
            anchors.margins: parent.width * 0.2
            Rectangle {
                width: interactionButton.width * ( 1.0 + 0.3 * AlexaInterface.audioLevel )
                height: width
                anchors.centerIn: interactionButton
                radius: width / 2
                color: Style.accentColor
            }
            RoundButton {
                id: interactionButton
                visible: opacity > 0
                anchors.horizontalCenter: parent.horizontalCenter
                width: height
                height: parent.height
                background: Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: AlexaInterface.connectionStatus === Alexa.Connected ?
                               "#00caff" :
                               "lightgrey";
                }
                icon.height: interactionButton.height/2
                icon.width: interactionButton.width/2
                icon.source: {
                    if (AlexaInterface.dialogState === Alexa.Speaking) {
                        return Qt.resolvedUrl("assets/ic_speaking.png")
                    } else if (AlexaInterface.dialogState === Alexa.Thinking) {
                        return Qt.resolvedUrl("assets/ic_thinking.png")
                    } else {
                        return Qt.resolvedUrl("assets/ic_microphone.png")
                    }
                }
                enabled: AlexaInterface.connectionStatus === Alexa.Connected
                onClicked: {
                    var dialogState = AlexaInterface.dialogState;

                    if (dialogState === Alexa.Idle) {
                        AlexaInterface.tapToTalk();
                    } else if ( (dialogState === Alexa.Listening)
                               || (dialogState === Alexa.Speaking) ) {
                        AlexaInterface.stopTalking();
                    }
                }
                Image {
                    id: busyIndicator
                    anchors.centerIn: parent
                    width: parent.width * 1.075
                    height: parent.height * 1.075
                    source: "assets/spinner.png"
                    visible: false
                }
                ColorOverlay {
                    id: overlay
                    anchors.fill: busyIndicator
                    source: busyIndicator
                    color: "#0071ff"
                    visible: (AlexaInterface.dialogState === Alexa.Listening)
                             || (AlexaInterface.dialogState === Alexa.Thinking)
                    RotationAnimation on rotation {
                        loops: Animation.Infinite
                        from: 0
                        to: 360
                        duration: 2000
                        running: overlay.visible
                    }
                }
            }
        }
    }
}
