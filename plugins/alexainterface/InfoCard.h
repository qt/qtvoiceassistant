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

#ifndef INFOCARD_H
#define INFOCARD_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "BaseCard.h"

class InfoCard : public BaseCard
{
    Q_OBJECT

    Q_PROPERTY(QString subtitle READ subtitle NOTIFY subtitleChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString skillIcon READ skillIcon NOTIFY skillIconChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QString image READ image NOTIFY imageChanged)

public:

    explicit InfoCard(QObject *parent = nullptr);
    virtual ~InfoCard() override = default;

    QString subtitle() const { return m_subtitle; }
    QString title() const { return m_title; }
    QString skillIcon() const { return m_skillIcon; }
    QString text() const { return m_text; }
    QString image() const { return m_image; }

    virtual void setJsonDocument(QJsonDocument jsonDocument) override;

    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);
    void setText(const QString &text);
    void setImage(const QString &image);
    void setSkillIcon(const QString &skillIcon);
Q_SIGNALS:
    void subtitleChanged(QString subtitle);
    void titleChanged(QString title);
    void skillIconChanged(QString skillIcon);
    void textChanged(QString text);
    void imageChanged(QString image);

private:
    QString m_subtitle;
    QString m_title;
    QString m_skillIcon;
    QString m_text;
    QString m_image;
};

#endif //INFOCARD_H
