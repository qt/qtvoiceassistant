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
import QtQuick.Layouts 1.12
import QtApplicationManager 2.0

import shared.Style 1.0
import shared.controls 1.0
import shared.Sizes 1.0
import shared.utils 1.0
import alexainterface 1.0

Control {
    id: root

    property string neptuneState: "Maximized"

    signal cardShown()

    state: {
        if (neptuneState == "Maximized") {
            if (priv.activeCard) {
                return "cardShown"
            } else {
                return "cardHidden"
            }
        } else {
            return neptuneState
        }
    }

    states: [
        State {
            name: "cardShown"
            PropertyChanges {
                target: interactionButton
                height: Sizes.dp(120)
                anchors.horizontalCenterOffset: -root.width/2 + interactionButton.width / 2 + Sizes.dp(50)
                anchors.topMargin: Sizes.dp(100) - interactionButton.height / 2
            }
            PropertyChanges {
                target: stopSpeakingButton
                anchors.horizontalCenterOffset: root.width/2 - stopSpeakingButton.width / 2 - Sizes.dp(50)
                anchors.topMargin: Sizes.dp(100) - stopSpeakingButton.height / 2
            }
            PropertyChanges {
                target: cardPane
                anchors.topMargin: interactionButton.height * 1.5
                height: root.height  - cardPane.anchors.topMargin
                opacity: 1
            }
            PropertyChanges {
                target: interactionPane
                height: parent.height / 2
            }
        },
        State {
            name: "cardHidden"
        },
        State {
            name: "Widget1Row"
            PropertyChanges {
                target: interactionButton
                height: Sizes.dp(150)
                anchors.horizontalCenterOffset: -root.width/2 + interactionButton.width / 2 + Sizes.dp(60)
                anchors.topMargin: Sizes.dp(120) - interactionButton.height / 2
            }
            PropertyChanges {
                target: stopSpeakingButton
                anchors.horizontalCenterOffset: root.width/2 - stopSpeakingButton.width / 2 - Sizes.dp(60)
                anchors.topMargin: Sizes.dp(120) - stopSpeakingButton.height / 2
            }
            PropertyChanges {
                target: cardPane
                opacity: 0
            }
        },
        State {
            name: "Widget2Rows"
            PropertyChanges {
                target: interactionButton
                height: Sizes.dp(200)
                anchors.horizontalCenterOffset: 0
                anchors.topMargin: Sizes.dp(260) - interactionButton.height / 2
            }
            PropertyChanges {
                target: stopSpeakingButton
                anchors.horizontalCenterOffset: 0
                anchors.topMargin: Sizes.dp(450) - stopSpeakingButton.height / 2
            }
            PropertyChanges {
                target: headerText
                opacity: 1
            }
        },
        State {
            name: "Widget3Rows"
            PropertyChanges {
                target: interactionButton
                height: Sizes.dp(250)
                anchors.horizontalCenterOffset: 0
                anchors.topMargin: Sizes.dp(350) - interactionButton.height / 2
            }
            PropertyChanges {
                target: stopSpeakingButton
                anchors.horizontalCenterOffset: 0
                anchors.topMargin: Sizes.dp(600) - stopSpeakingButton.height / 2
            }
            PropertyChanges {
                target: headerText
                anchors.topMargin: Sizes.dp(70)
                opacity: 1
            }
        }
    ]
    transitions: Transition {
        NumberAnimation {
            properties: "opacity,height,anchors.horizontalCenterOffset,anchors.topMargin"
            duration: 200
        }
    }

    QtObject {
        id: priv
        property var activeCardComponent: null
        property var activeCard: null
        property var cardData: null

        function createCardObjects(file) {
            if (priv.activeCard) {
                closeCard()
            }

            priv.activeCardComponent = Qt.createComponent(file);
            if (priv.activeCardComponent.status === Component.Ready)
                finishCreation();
            else
                priv.activeCardComponent.statusChanged.connect(finishCreation);
        }

        function finishCreation() {
            if (priv.activeCardComponent.status === Component.Ready) {
                priv.activeCard = priv.activeCardComponent.createObject(cardPane, {"anchors.fill": cardPane, "cardData": priv.cardData});
                if (priv.activeCard == null) {
                    console.warn("Error creating object");
                }
                priv.activeCard.closeClicked.connect( closeCard )
            } else if (priv.activeCardComponent.status === Component.Error) {
                console.warn("Error loading component:", priv.activeCardComponent.errorString());
            }
        }

        function closeCard() {
            priv.activeCard.destroy()
            priv.activeCard = null
            priv.activeCardComponent = null
        }
    }

    Connections {
        target: AlexaInterface
        onCardReady: {
            if (card.type === BaseCard.Weather) {
                root.cardShown();
                priv.cardData = card
                priv.createCardObjects("WeatherCard.qml")
            } else if (card.type === BaseCard.Info) {
                root.cardShown();
                priv.cardData = card
                priv.createCardObjects("InfoCard.qml")
            } else if (card.type === BaseCard.VehicleIntent) {
                var request = IntentClient.sendIntentRequest("vehicle-control",
                                                             "com.luxoft.vehicle",
                                                             {action: card.action, side: card.side, part: card.part });
                request.onReplyReceived.connect(function() {
                    if (request.succeeded) {
                        var result = request.result
                        console.log(Logging.apps, "Intent result: " + result.done)
                    } else {
                        console.log(Logging.apps, "Intent request failed: " + request.errorMessage)
                    }
                });
            } else if (card.type === BaseCard.OpenAppIntent) {
                request = IntentClient.sendIntentRequest("activate-app", {"guess-app": card.appName});
                request.onReplyReceived.connect(function() {
                    if (request.succeeded) {
                        var result = request.result
                        console.log(Logging.apps, "Intent result: " + result.done)
                    } else {
                        console.log(Logging.apps, "Intent request failed: " + request.errorMessage)
                    }
                });
            }
        }
    }


    contentItem: Item {
        Row {
            id: headerText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Sizes.dp(40)
            spacing: Sizes.dp(35)
            opacity: 0
            visible: opacity > 0
            Image {
                id: alexaLogo
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                height: Sizes.dp(64)
                source: "assets/logo.png"
            }
            Label {
                id: alexaLogoText
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Sizes.fontSizeXXL
                font.letterSpacing: Sizes.dp(5)
                opacity: 0.75
                font.weight: Font.Medium
                text: "amazon alexa"
            }
        }

        Item {
            id: interactionPane
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height
            Rectangle {
                width: interactionButton.width * ( 1.0 + 0.3 * AlexaInterface.audioLevel )
                height: width
                anchors.centerIn: interactionButton
                radius: width / 2
                color: Style.accentColor
            }
            DropShadow {
                anchors.fill: interactionButton
                horizontalOffset: Sizes.dp(3)
                verticalOffset: Sizes.dp(3)
                radius: Sizes.dp(8.0)
                samples: 17
                color: "#80000000"
                source: interactionButton.background
            }
            Button {
                id: interactionButton
                visible: opacity > 0
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: Sizes.dp(300)
                width: height
                height: parent.height/2 > Sizes.dp(270) ? Sizes.dp(270) : parent.height/2
                background: Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: AlexaInterface.connectionStatus === Alexa.Connected ? "#00caff" : "lightgrey"
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
                    if (AlexaInterface.dialogState === Alexa.Idle) {
                        AlexaInterface.tapToTalk()
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
                    visible: (AlexaInterface.dialogState === Alexa.Listening) || (AlexaInterface.dialogState === Alexa.Thinking)
                    RotationAnimation on rotation {
                        loops: Animation.Infinite
                        from: 0
                        to: 360
                        duration: 2000
                        running: overlay.visible
                    }
                }
            }
            Button {
                id: stopSpeakingButton
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: interactionButton.anchors.topMargin + interactionButton.height + Sizes.dp(100)
                width: parent.width > Sizes.dp(270) ? Sizes.dp(270) : parent.width
                height: Sizes.dp(70)
                opacity: (AlexaInterface.dialogState === Alexa.Listening) || (AlexaInterface.dialogState === Alexa.Speaking) ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 100 } }
                visible: opacity > 0
                text: qsTr("STOP")
                font.pixelSize: Sizes.fontSizeM
                onClicked: AlexaInterface.stopTalking()
                background: ButtonBackground {
                    anchors.fill: parent
                    opacity: 1
                    color: "#ff4922"
                }
            }
        }
        Item {
            id: cardPane
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 0
            opacity: 0
            visible: opacity > 0
        }
    }
}
