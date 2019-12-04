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

#ifndef OPENAPPINTENTCARD_H
#define OPENAPPINTENTCARD_H


#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "BaseCard.h"


class OpenAppIntentCard : public BaseCard
{
    Q_OBJECT

    Q_PROPERTY(QString appName READ appName NOTIFY appNameChanged)

public:
    explicit OpenAppIntentCard(QObject *parent = nullptr);
    void setJsonDocument(QJsonDocument jsonDocument) override;

    QString appName() const;
Q_SIGNALS:
    void appNameChanged();

private:
    QString m_appName;
};


#endif // OPENAPPINTENTCARD_H
