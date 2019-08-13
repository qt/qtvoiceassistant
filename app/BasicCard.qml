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

import QtQuick 2.10
import QtQuick.Controls 2.3

import shared.Sizes 1.0

Control {
    id: root

    property var cardData

    //default property alias cardContent: root.contentItem
    signal closeClicked()
    property bool closeEnabled: true

    Item {
        id: closeButton
        z: 10
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(15)
        anchors.right: parent.right
        anchors.rightMargin: Sizes.dp(15)
        width: Sizes.dp(64)
        height: Sizes.dp(64)
        visible: root.closeEnabled
        Rectangle {
            anchors.fill: parent
            radius: width/2
            color: "lightgrey"
            opacity: maCloseButton.containsMouse ? 0.8 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }
            scale: maCloseButton.containsPress ? 0.95 : 1
            Behavior on scale { NumberAnimation { duration: 100 } }
        }
        Label {
            anchors.centerIn: parent
            font.pixelSize: Sizes.fontSizeXXL
            opacity: maCloseButton.containsMouse ? 0.8 : 0.3
            Behavior on opacity { NumberAnimation { duration: 200 } }
            scale: maCloseButton.containsPress ? 0.95 : 1
            Behavior on scale { NumberAnimation { duration: 100 } }
            text: "\u2715"
        }
        MouseArea {
            id: maCloseButton
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.closeClicked()
        }
    }
}
