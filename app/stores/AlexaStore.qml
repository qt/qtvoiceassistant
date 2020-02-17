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

import QtQuick 2.8
import QtApplicationManager.Application 2.0
import alexainterface 1.0

QtObject {
    id: root

    signal triggerVoiceAssistant()
    signal requestRaiseAppReceived()

    readonly property IntentHandler intentHandler: IntentHandler {
        intentIds: ["trigger-voiceassistant", "activate-app"]
        onRequestReceived: {
            switch (request.intentId) {
            case "trigger-voiceassistant":
                root.triggerVoiceAssistant();
                request.sendReply({ "done": true });
                break;
            case "activate-app":
                root.requestRaiseAppReceived();
                request.sendReply({ "done": true });
                break;
            default:
                break;
            }
        }
    }

    Component.onCompleted: {
        // For multi-process mode of QtAppMan all the paths for apps are relative to their main.qml.
        // But for single process mode all paths are relative to main.qml of the Neptune 3 UI
        // and we need to discover where we are. This is workaround to make Alexa app support
        // singe-process mode and initialize db and configs paths to absolute paths upon start up
        AlexaInterface.initAlexaQMLClient(Qt.resolvedUrl(".."))
    }
}
