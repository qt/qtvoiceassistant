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
import QtQuick.Layouts 1.13
import QtWebView 1.1

import alexainterface 1.0

import shared.utils 1.0
import shared.controls 1.0
import shared.Style 1.0
import shared.Sizes 1.0

Control {
    id: root

    property var alexaAuth
    property bool authorizationRequested: false

    ColumnLayout {
        id: initStateView
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(200)
        width: 0.4 * root.width
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0
        visible: opacity > 0
        spacing: Sizes.dp(25)

        Label {
            id: loginLabel
            text: qsTr("Amazon account:")
            width: parent.width
            font.pixelSize: Sizes.fontSizeM
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: emailField
            color: "gray"
            font.pixelSize: Sizes.fontSizeS
            placeholderText: "email"
            inputMethodHints: Qt.ImhEmailCharactersOnly
            background: Rectangle {
                anchors.fill: parent
                radius: Sizes.dp(4)
            }
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        TextField {
            id: passwordField
            height: Sizes.dp(54)
            color: "gray"
            font.pixelSize: Sizes.fontSizeS
            placeholderText: "password"
            echoMode: TextInput.Password
            passwordCharacter: '*'
            background: Rectangle {
                anchors.fill: parent
                radius: Sizes.dp(4)
            }
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        id: captchaView
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(200)
        width: 0.5 * root.width
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0
        visible: opacity > 0
        spacing: Sizes.dp(25)

        Image {
            id: captchaImage
            width: Sizes.dp(sourceSize.width)
            height: Sizes.dp(sourceSize.height)
            source: alexaAuth.captchaUrl
            onSourceChanged: {
                captchaField.text = ""
            }
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: captchaField
            width: parent.width
            color: "gray"
            text: ""
            placeholderText: "Enter the characters you see"
            font.pixelSize: Sizes.fontSizeS
            background: Rectangle {
                anchors.fill: parent
                radius: Sizes.dp(4)
            }
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        id: manualAuthorization
        anchors.horizontalCenter: parent.horizontalCenter
        width: 0.65 * parent.width
        opacity: 0
        visible: opacity > 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: Sizes.dp(15)

        Label {
            id: authCode
            font.pixelSize: Sizes.fontSizeM
            text: "Your code: " + AlexaInterface.authCode
            visible: AlexaInterface.authCode !== ""
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            id: errorText
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: Sizes.fontSizeS
            text: "Cannot authorize due to invalid client id. Check the client id in the AlexaClientSDKConfig.json and restart the Alexa application"
            visible: webView.url.toString() === ""
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        WebView {
            id: webView
            height: Sizes.dp(700)
            url: AlexaInterface.authUrl
            visible: url !== ""
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            id: helpText
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            font.pixelSize: Sizes.fontSizeS
            text: "Couldn't authorize automatically. Proceed by filling your email, password and authorization code in the web form."
            visible: webView.url.toString() !== ""
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }
    }


    Button {
        id: authButton

        implicitWidth: Sizes.dp(315)
        implicitHeight: Sizes.dp(64)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(500)
        visible: opacity > 0
        opacity: 1
        enabled: (!alexaAuth.isAuthorizing && emailField.text !== "" && passwordField.text !== "")
                        || (alexaAuth.isFillingCaptcha && captchaField.text !== "")
        text: qsTr("Authorize")
        font.pixelSize: Sizes.fontSizeS
        onClicked: {
            alexaAuth.email = emailField.text
            alexaAuth.password = passwordField.text
            if (alexaAuth.isFillingCaptcha) {
                alexaAuth.captcha = captchaField.text
                alexaAuth.authorizeWithCaptcha()
            } else {
                alexaAuth.authorize()
            }
            root.authorizationRequested = true
        }
    }

    ColumnLayout {
        anchors.top: parent.top
        width: parent.width

        ProgressBar {
            id: progress
            width: parent.width
            implicitHeight: Sizes.dp(8)
            opacity: 0
            visible: opacity > 0
            from: 0
            to: 1
            value: 0
            indeterminate: true
            Layout.fillWidth: true
            SequentialAnimation on value {
                loops: Animation.Infinite
                PropertyAnimation { to: 0; duration: 1500 }
                PropertyAnimation { to: 1; duration: 1500 }
            }
        }

        Label {
            id: authAppText
            text: qsTr("Authorizing device...")
            font.pixelSize: Sizes.fontSizeL
            opacity: 0
            visible: opacity > 0
            Layout.topMargin: Sizes.dp(400)
            Layout.alignment: Qt.AlignHCenter
        }
    }

    states: [
        State {
            name: "initial_state"
            PropertyChanges { target: initStateView; opacity: 1 }
        },
        State {
            name: "captcha"
            PropertyChanges { target: captchaView; opacity: 1; anchors.top: initStateView.bottom;
                                                    anchors.topMargin: Sizes.dp(25) }
            PropertyChanges { target: initStateView; opacity: 1 }
            PropertyChanges { target: progress; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0 }
            PropertyChanges { target: authButton; opacity: 1; anchors.top: captchaView.bottom;
                                                  anchors.topMargin: Sizes.dp(25) }
        },
        State {
            name: "automatic_auth"
            PropertyChanges { target: initStateView; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0.8 }
            PropertyChanges { target: progress; opacity: 1 }
        },
        State {
            name: "manual_auth"
            PropertyChanges { target: initStateView; opacity: 0 }
            PropertyChanges { target: manualAuthorization; opacity: 1 }
            PropertyChanges { target: progress; opacity: 0 }
            PropertyChanges { target: authButton; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0 }
        },
        State {
            name: "complete_auth"
            PropertyChanges { target: manualAuthorization; opacity: 0 }
            PropertyChanges { target: authAppText; opacity: 0.8; text: "Authorization completed" }
            PropertyChanges { target: progress; opacity: 0 }
            PropertyChanges { target: authButton; opacity: 0 }
        }
    ]

    state: {
        if (!alexaAuth.authorizationSucceed) {
            if (!root.authorizationRequested) {
                return "initial_state"
            } else if (alexaAuth.isAuthorizing) {
                if (alexaAuth.isFillingCaptcha) {
                    return "captcha"
                } else {
                    return "automatic_auth"
                }
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
