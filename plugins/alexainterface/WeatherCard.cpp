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

#include "WeatherCard.h"
#include <QDebug>

WeatherCard::WeatherCard(QObject *parent) : BaseCard(parent)
{
    setType(BaseCard::Weather);
}

void WeatherCard::setTemperature(qreal temperature)
{
    if (qFuzzyCompare(m_temperature, temperature))
        return;
    m_temperature = temperature;
    Q_EMIT temperatureChanged(m_temperature);
}

void WeatherCard::setLowTemperature(qreal lowTemperature)
{
    if (qFuzzyCompare(m_lowTemperature, lowTemperature))
        return;
    m_lowTemperature = lowTemperature;
    Q_EMIT lowTemperatureChanged(m_lowTemperature);
}

void WeatherCard::setHighTemperature(qreal highTemperature)
{
    if (qFuzzyCompare(m_highTemperature, highTemperature))
        return;
    m_highTemperature = highTemperature;
    Q_EMIT highTemperatureChanged(m_highTemperature);
}

void WeatherCard::setSubtitle(QString date)
{
    if (m_subtitle == date)
        return;
    m_subtitle = date;
    Q_EMIT subtitleChanged(m_subtitle);
}

void WeatherCard::setTitle(QString title)
{
    if (m_title == title)
        return;
    m_title = title;
    Q_EMIT titleChanged(m_title);
}

void WeatherCard::setDescription(QString description)
{
    if (m_description == description)
        return;

    m_description = description;
    Q_EMIT descriptionChanged(m_description);
}

void WeatherCard::setWeatherIcon(QString icon) {
    if (m_weatherIcon == icon)
        return;
    m_weatherIcon = icon;
    Q_EMIT weatherIconChanged(m_weatherIcon);
}

void WeatherCard::setJsonDocument(QJsonDocument jsonDocument)
{
    QJsonObject obj = jsonDocument.object();

    QString currentTemperature = obj["currentWeather"].toString();
    currentTemperature.remove("°");
    setTemperature(qreal(currentTemperature.toDouble()));

    QJsonObject lowTemperature = obj["lowTemperature"].toObject();
    QString lowTemperatureValue = lowTemperature["value"].toString();
    lowTemperatureValue.remove("°");
    setLowTemperature(qreal(lowTemperatureValue.toDouble()));

    QJsonObject highTemperature = obj["highTemperature"].toObject();
    QString highTemperatureValue = highTemperature["value"].toString();
    highTemperatureValue.remove("°");
    setHighTemperature(qreal(highTemperatureValue.toDouble()));

    QJsonObject title = obj["title"].toObject();
    QString date = title["subTitle"].toString();
    QString location = title["mainTitle"].toString();
    setSubtitle(date);
    setTitle(location);

    QString forecastDescription = obj["description"].toString();
    setDescription(forecastDescription);

    QJsonObject currentWeatherIcon = obj["currentWeatherIcon"].toObject();
    QJsonArray sources = currentWeatherIcon["sources"].toArray();
    QJsonObject sourcesMedium = sources.at(1).toObject();
    QString url = sourcesMedium["url"].toString();
    setWeatherIcon(url);

}
