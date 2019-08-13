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
import QtQuick.Layouts 1.12

import shared.Sizes 1.0

BasicCard {
    id: root

    Column {
        id: titleContent
        anchors.top: parent.top
        anchors.topMargin: Sizes.dp(10)
        anchors.left: parent.left
        anchors.leftMargin: Sizes.dp(50)
        anchors.right: parent.right
        anchors.rightMargin: Sizes.dp(50)
        spacing: Sizes.dp(7)
        Label {
            width: parent.width
            font.pixelSize: Sizes.fontSizeL
            elide: Text.ElideRight
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: cardData.title
        }
        Label {
            width: parent.width
            font.pixelSize: Sizes.fontSizeM
            elide: Text.ElideRight
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: cardData.subtitle
            opacity: 0.7
        }
    }

    RowLayout {
        id: temperatureRow
        anchors.top: titleContent.bottom
        anchors.topMargin: Sizes.dp(10)
        anchors.left: parent.left
        anchors.leftMargin: Sizes.dp(50)
        anchors.right: parent.right
        anchors.rightMargin: Sizes.dp(50)
        spacing: Sizes.dp(50)
        Image {
            Layout.preferredHeight: root.height/3
            Layout.preferredWidth: root.height/4
            fillMode: Image.PreserveAspectFit
            source: cardData.weatherIcon
            BusyIndicator {
                anchors.centerIn: parent
                visible: parent.state === Image.Loading
            }
        }
        Label {
            Layout.fillWidth: true
            font.pixelSize: root.height/4
            text: cardData.temperature + "\u00b0"
            opacity: 0.7
        }

        Column {
            Layout.preferredHeight: root.height/3
            Layout.preferredWidth: height
            Label {
                height: parent.height / 2
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.height * 0.3
                text: "\u2191 " + cardData.highTemperature + "\u00b0"
                opacity: 0.5
            }
            Label {
                height: parent.height / 2
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.height * 0.3
                text: "\u2193 " + cardData.lowTemperature + "\u00b0"
                opacity: 0.5
            }
        }
    }

    Label {
        anchors.top: temperatureRow.bottom
        anchors.topMargin: Sizes.dp(60)
        anchors.left: parent.left
        anchors.leftMargin: Sizes.dp(50)
        anchors.right: parent.right
        anchors.rightMargin: Sizes.dp(50)
        elide: Text.ElideRight
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: cardData.description
        font.pixelSize: Sizes.fontSizeM
        opacity: 0.7
    }
}
