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

import shared.Sizes 1.0
import shared.Style 1.0
import shared.controls 1.0
import shared.utils 1.0

import alexainterface 1.0


Item {
    id: root

    signal resetAccountClicked()

    Flickable {
        anchors.fill: parent
        anchors.topMargin: Sizes.dp(80)
        anchors.bottomMargin: Sizes.dp(20)
        contentHeight: settingsLayout.height
        clip: true

        ScrollIndicator.vertical: ScrollIndicator { }

        ColumnLayout {
            id: settingsLayout

            spacing: Sizes.dp(40)
            width: parent.width

            ListItem {
                subText: "123456";
                text: qsTr("Device serial number")
                Layout.fillWidth: true
            }

            ListItem {
                subText: "amzn1.application-oa2-client.a92e3edd0c8542a6bafcd89e5f125851";
                text: qsTr("Alexa Client ID")
                Layout.fillWidth: true
            }

            ListItemSwitch {
                text: qsTr("Alexa wake word")
                switchOn: true
                enabled: false
                Layout.fillWidth: true
            }

            Button {
                visible: AlexaInterface.loggedIn
                implicitWidth: Sizes.dp(315)
                implicitHeight: Sizes.dp(64)
                font.pixelSize: Sizes.fontSizeS
                text: qsTr("Reset account")
                Layout.alignment: Qt.AlignHCenter
                enabled: false
            }

            ListItem {
                text: qsTr("Recording devices")
                Layout.fillWidth: true
            }

            ListView {
                id: listView
                model: AlexaInterface.deviceList
                height: childrenRect.height
                Layout.fillWidth: true
                interactive: false
                delegate: RadioButton {
                    checked: modelData === "default"
                    width: parent.width
                    height: Sizes.dp(50)
                    font.pixelSize: Sizes.fontSizeXS
                    indicator.implicitHeight: Sizes.dp(30)
                    indicator.implicitWidth: Sizes.dp(30)
                    text: modelData
                    spacing: Sizes.dp(10)
                    checkable: false
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
