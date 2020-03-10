/****************************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#include "ConsolePrinter.h"
#include <QDebug>

ConsolePrinter::ConsolePrinter(avsCommon::utils::logger::Level level)
    : avsCommon::utils::logger::Logger(level)
{

}

void ConsolePrinter::emit(avsCommon::utils::logger::Level level,
                          std::chrono::system_clock::time_point time,
                          const char *threadMoniker, const char *text)
{
    QString message = QString::fromStdString(m_logFormatter.format(level, time,
                                                                   threadMoniker, text));

    switch (level) {
        case(avsCommon::utils::logger::Level::DEBUG9):
        case(avsCommon::utils::logger::Level::DEBUG8):
        case(avsCommon::utils::logger::Level::DEBUG7):
        case(avsCommon::utils::logger::Level::DEBUG6):
        case(avsCommon::utils::logger::Level::DEBUG5):
        case(avsCommon::utils::logger::Level::DEBUG4):
        case(avsCommon::utils::logger::Level::DEBUG3):
        case(avsCommon::utils::logger::Level::DEBUG2):
        case(avsCommon::utils::logger::Level::DEBUG1):
        case(avsCommon::utils::logger::Level::DEBUG0):
            qDebug().noquote() << message;
        break;
        case(avsCommon::utils::logger::Level::INFO):
            qInfo().noquote() << message;
        break;
        case(avsCommon::utils::logger::Level::WARN):
            qWarning().noquote() << message;
        break;
        case(avsCommon::utils::logger::Level::ERROR):
        case(avsCommon::utils::logger::Level::CRITICAL):
            qCritical().noquote() << message;
        break;
        case(avsCommon::utils::logger::Level::UNKNOWN):
        case(avsCommon::utils::logger::Level::NONE):
            // no output
        break;
    }
}
