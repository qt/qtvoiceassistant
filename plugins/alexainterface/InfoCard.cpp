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

#include "InfoCard.h"

#include <QDebug>

InfoCard::InfoCard(QObject *parent) : BaseCard(parent)
{
    setType(BaseCard::Info);
}

void InfoCard::setSubtitle(const QString &subtitle)
{
    if (m_subtitle == subtitle)
        return;
    m_subtitle = subtitle;
    Q_EMIT subtitleChanged(m_subtitle);
}

void InfoCard::setText(const QString &text)
{
    if (m_text == text)
        return;
    m_text = text;
    Q_EMIT textChanged(m_text);
}

void InfoCard::setImage(const QString &image)
{
    if (m_image == image)
        return;
    m_image = image;
    Q_EMIT imageChanged(m_image);
}

void InfoCard::setSkillIcon(const QString &skillIcon)
{
    if (m_skillIcon == skillIcon)
        return;
    m_skillIcon = skillIcon;
    Q_EMIT skillIconChanged(m_skillIcon);
}

void InfoCard::setTitle(const QString &title)
{
    if (m_title == title)
        return;
    m_title = title;
    Q_EMIT titleChanged(m_title);
}

void InfoCard::setJsonDocument(QJsonDocument jsonDocument)
{
    QJsonObject obj = jsonDocument.object();
    //qWarning() << jsonDocument.toJson(QJsonDocument::Indented);

    QString text = obj["textField"].toString();
    setText(text);

    QJsonObject header = obj["title"].toObject();
    QString title = header["mainTitle"].toString();
    setTitle(title);

    QString subtitle = header["subTitle"].toString();
    setSubtitle(subtitle);

    QJsonObject skillIcon = obj["skillIcon"].toObject();
    QJsonArray skillIconSources = skillIcon["sources"].toArray();
    QJsonObject sourceSmall = skillIconSources.at(0).toObject();
    QString urlSkillIcon = sourceSmall["url"].toString();
    setSkillIcon(urlSkillIcon);

    QString urlLargeImage, urlMediumImage, urlSmallImage;
    QJsonObject image = obj["image"].toObject();
    QJsonArray sources = image["sources"].toArray();
    for (auto jsVal : sources) {
        QString size = jsVal.toObject()["size"].toString().toLower();
        if (size == "large") {
            urlLargeImage = jsVal.toObject()["url"].toString();
        } else if (size == "medium") {
            urlMediumImage = jsVal.toObject()["url"].toString();
        } else if (size == "small") {
            urlSmallImage = jsVal.toObject()["url"].toString();
        }
    }
    if (!urlLargeImage.isEmpty()) {
        setImage(urlLargeImage);
    } else if (!urlMediumImage.isEmpty()) {
        setImage(urlMediumImage);
    } else if (!urlSmallImage.isEmpty()) {
        setImage(urlSmallImage);
    }
}
