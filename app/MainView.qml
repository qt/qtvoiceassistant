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

    Header {
        id: header
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(80)
        width: parent.width
        height: Sizes.dp(356)
        anchors.horizontalCenter: parent.horizontalCenter
        visible: root.neptuneState === "Maximized"
    }

    ToolsColumn {
        id: sectionsColumn

        anchors.left: parent.left
        width: Sizes.dp(264)
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: Sizes.dp(53)

        onClicked: {
            applyIndex(currentIndex);
        }

        // not to have settings all the time and have alexa view all the time
        // we use StackView, function pushes settings and pops it
        function applyIndex(index) {
            //show main, pop settings
            if (index === 0) {
                if (stack.depth > 1) {
                    stack.pop();
                }
            }
            //show settings, push settings
            if (index === 1) {
                if (stack.depth === 1) {
                    stack.push(settingsView);
                }
            }
        }

        ListModel {
            id: toolsModel

            Component.onCompleted: {
                //fill list and set model to ToolsColumn
                append({ "sourceOn": Qt.resolvedUrl("assets/ic-logo_ON.png"),
                         "sourceOff": Qt.resolvedUrl("assets/ic-logo_OFF.png"),
                         "text": qsTr("Alexa") });
                append({"sourceOn": Qt.resolvedUrl("assets/ic-settings_ON.png"),
                        "sourceOff": Qt.resolvedUrl("assets/ic-settings_OFF.png"),
                         "text": qsTr("Settings")});
                sectionsColumn.model = toolsModel
            }
        }
    }

    StackView {
        id: stack

        anchors.top: header.bottom
        anchors.left: sectionsColumn.right
        anchors.right: parent.right
        height: parent.height - header.height - Sizes.dp(50)
        initialItem: mainView
        pushEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1.0; duration: 200 }
        }
        pushExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1.0; to: 0; duration: 200 }
        }
        popEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1.0; duration: 200 }
        }
        popExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1.0; to: 0; duration: 200 }
        }
    }

    Component{
        id: mainView

        Item {
            Connections {
                target: AlexaInterface
                onAuthCodeChanged: {
                    if (AlexaInterface.authCode !== "") {
                        alexaAuth.authCode = AlexaInterface.authCode
                    }
                }
                onAuthUrlChanged: { alexaAuth.authUrl = AlexaInterface.authUrl }
                onAuthStateChanged: {
                    if (AlexaInterface.authState === Alexa.Uninitialized) {
                        alexaAuth.reset()
                    }
                }
                Component.onCompleted: { AlexaInterface.logLevel = Alexa.Debug9 }
            }

            AuthWebPageInteraction {
                id: alexaAuth
                onErrorChanged: {
                    if (error === AlexaAuth.AutomaticAuthFailed){
                        authView.state = "manual_auth"
                    }
                }
            }

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
    }

    Component{
        id: settingsView
        SettingsView{
            onResetAccountClicked: {
                sectionsColumn.applyIndex(0)
            }
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
