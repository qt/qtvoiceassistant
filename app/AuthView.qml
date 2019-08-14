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

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0

import QtWebView 1.1

import alexainterface 1.0
import alexaauth 1.0

import shared.utils 1.0
import shared.controls 1.0
import shared.Style 1.0
import shared.Sizes 1.0

Control {
    id: root

    property var alexaAuth
    property bool authorizationRequested: false

    Item {
        id: initStateView
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(500)
        width: 0.5 * root.width
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0
        visible: opacity > 0

        DropShadow {
            anchors.fill: emailField
            horizontalOffset: Sizes.dp(1)
            verticalOffset: Sizes.dp(2)
            radius: 6
            color: "#80000000"
            source: emailField.background
        }

        TextField {
            id: emailField
            width: parent.width
            height: Sizes.dp(54)
            color: "gray"
            font.pixelSize: Sizes.fontSizeS
            placeholderText: "email"
            background: Rectangle {
                anchors.fill: parent
                radius: 4
            }
        }

        DropShadow {
            anchors.fill: passwordField
            horizontalOffset: Sizes.dp(1)
            verticalOffset: Sizes.dp(2)
            radius: 6
            color: "#80000000"
            source: passwordField.background
        }

        TextField {
            id: passwordField
            width: parent.width
            height: Sizes.dp(54)
            anchors.top: emailField.bottom
            anchors.topMargin: Sizes.dp(25)
            color: "gray"
            font.pixelSize: Sizes.fontSizeS
            placeholderText: "password"
            echoMode: TextInput.Password
            passwordCharacter: '*'
            background: Rectangle {
                anchors.fill: parent
                radius: 4
            }
        }
    }

    Item {
        id: manualAuthorization
        anchors.horizontalCenter: parent.horizontalCenter
        width: 0.65 * parent.width
        opacity: 0
        visible: opacity > 0

        Label {
            id: authCode
            anchors.bottom: webView.top
            anchors.horizontalCenter: webView.horizontalCenter
            height: Sizes.dp(60)
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Sizes.fontSizeM
            text: "Your code: " + AlexaInterface.authCode
            visible: AlexaInterface.authCode !== ""
        }

        WebView {
            id: webView
            y: Sizes.dp(436) + Sizes.dp(100)
            width: parent.width
            height: Sizes.dp(700)
            anchors.horizontalCenter: parent.horizontalCenter
            url: AlexaInterface.authUrl
            visible: url !== ""
        }

        Label {
            id: helpText
            anchors.top: webView.bottom
            anchors.topMargin: Sizes.dp(15)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: Sizes.fontSizeS
            text: "Couldn't authorize automatically. Proceed by filling your email, password and authorization code in the web form."
            visible: webView.url.toString() !== ""
        }

        Label {
            id: errorText
            anchors.centerIn: parent
            anchors.verticalCenterOffset: Sizes.dp(600)
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: Sizes.fontSizeS
            text: "Cannot authorize due to invalid client id. Check the client id in the AlexaClientSDKConfig.json and restart the Alexa application"
            visible: webView.url.toString() === ""
        }
    }

    // The item holds the same position than the interaction button in AlexaView
    // The purpose is to prevent a jump when moved from AuthView to AlexaView
    Item {
        id: authButtonItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(600)
        height: parent.height/2 > Sizes.dp(270) ? Sizes.dp(270) : parent.height/2
        width: height
        visible: opacity > 0
        opacity: 1

        DropShadow {
            anchors.fill: authButton
            horizontalOffset: Sizes.dp(3)
            verticalOffset: Sizes.dp(3)
            radius: 8.0
            samples: 17
            color: "#80000000"
            source: authButton.background
        }

        Button {
            id: authButton
            anchors.centerIn: parent
            width: parent.width
            height: Sizes.dp(70)
            enabled: !alexaAuth.isAuthorizing && emailField.text !== "" && passwordField.text !== ""
            text: "Authorize"
            font.pixelSize: Sizes.fontSizeL
            icon.color: enabled ? Style.contrastColor : "black"
            background: ButtonBackground {
                border.color: parent.enabled ? "#5FCAF4" : "lightgray"
                color: parent.enabled ? "#5FCAF4" : "lightgray"

            }
            onClicked: {
                alexaAuth.email = emailField.text
                alexaAuth.password = passwordField.text
                alexaAuth.authorize()
                root.authorizationRequested = true
            }
        }
    }

    Row {
        anchors.top: authButtonItem.bottom
        anchors.topMargin: Sizes.dp(40)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Sizes.dp(20)

        Item {
            id: spinner
            width: Sizes.dp(50)
            height: width
            anchors.verticalCenter: parent.verticalCenter
            opacity: 0
            visible: opacity > 0

            Image {
                id: spinnerImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: "assets/spinner.png"
            }

            ColorOverlay {
                id: overlay
                anchors.fill: spinnerImage
                source: spinnerImage
                color: "#63abc8"
                visible: spinnerImage.opacity > 0
                RotationAnimation on rotation {
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                    duration: 1000
                    running: overlay.visible
                }
            }
        }

        Label {
            id: authAppText
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Authorizing application...")
            font.pixelSize: Sizes.fontSizeL
            opacity: 0
            visible: opacity > 0
        }
    }

    states: [
        State {
            name: "initial_state"
            PropertyChanges { target: initStateView; opacity: 1 }
        },
        State {
            name: "automatic_auth"
            PropertyChanges { target: initStateView; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0.8 }
            PropertyChanges { target: spinner; opacity: 1 }
        },
        State {
            name: "manual_auth"
            PropertyChanges { target: initStateView; opacity: 0 }
            PropertyChanges { target: manualAuthorization; opacity: 1 }
            PropertyChanges { target: spinner; opacity: 0 }
            PropertyChanges { target: authButtonItem; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0 }
        },
        State {
            name: "complete_auth"
            PropertyChanges { target: manualAuthorization; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0.8; text: "Authorization completed" }
            PropertyChanges { target: spinner; opacity: 0 }
            PropertyChanges { target: authButtonItem; opacity: 0 }
        }
    ]

    state: {
        if (!alexaAuth.authorizationSucceed) {
            if (!root.authorizationRequested) {
                return "initial_state"
            } else if (alexaAuth.isAuthorizing) {
                return "automatic_auth"
            } else {
                return "manual_auth"
            }
        } else {
            return "complete_auth"
        }
    }

    transitions: [
        Transition {
            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
