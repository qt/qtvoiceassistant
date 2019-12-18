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
import QtQuick.Controls 2.5

import shared.Sizes 1.0
import shared.Style 1.0

import "stores" 1.0

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

    Label {
        id: alexaLoadErrorText
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: Sizes.fontSizeM
        font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter
        width: root.exposedRect.width
        opacity: Style.opacityHigh
        text: qsTr("Please make sure that the Alexa SDK is installed correctly")
        visible: false
    }

    Loader {
        source: "MainView.qml"
        onStatusChanged: {
            if (status === Loader.Error) {
                alexaLoadErrorText.visible = true
            }
        }
        onLoaded: {
            alexaLoadErrorText.visible = false
            item.x = Qt.binding(function() { return root.exposedRect.x; })
            item.y = Qt.binding(function() { return root.exposedRect.y; })
            item.width = Qt.binding(function() { return root.exposedRect.width; })
            item.height = Qt.binding(function() { return root.exposedRect.height; })
            item.neptuneState = Qt.binding(function() { return root.neptuneState; })
            item.visible = Qt.binding(function() { return root.exposedRect.height > 0; })
            item.store = alexaStore
        }
    }

    AlexaStore {
        id: alexaStore
        onRequestRaiseAppReceived: {
            root.riseWindow();
        }
    }
}
