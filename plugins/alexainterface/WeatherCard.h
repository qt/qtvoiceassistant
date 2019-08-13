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

#ifndef WEATHERCARD_H
#define WEATHERCARD_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "BaseCard.h"

class WeatherCard : public BaseCard
{
    Q_OBJECT

    Q_PROPERTY(qreal temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(qreal lowTemperature READ lowTemperature NOTIFY lowTemperatureChanged)
    Q_PROPERTY(qreal highTemperature READ highTemperature NOTIFY highTemperatureChanged)
    Q_PROPERTY(QString subtitle READ subtitle NOTIFY subtitleChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString weatherIcon READ weatherIcon NOTIFY weatherIconChanged)

public:
    explicit WeatherCard(QObject *parent = nullptr);

    qreal temperature() const { return m_temperature; }
    qreal lowTemperature() const { return m_lowTemperature; }
    qreal highTemperature() const { return m_highTemperature; }
    QString subtitle() const { return m_subtitle; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QString weatherIcon() const { return m_weatherIcon; }

    void setJsonDocument(QJsonDocument jsonDocument) override;

Q_SIGNALS:
    void temperatureChanged(qreal temperature);
    void lowTemperatureChanged(qreal lowestTemperature);
    void highTemperatureChanged(qreal highestTemperature);
    void titleChanged(QString title);
    void subtitleChanged(QString subtitle);
    void descriptionChanged(QString description);
    void weatherIconChanged(QString weatherIcon);

public Q_SLOTS:

private:
    void setTemperature(qreal temperature);
    void setLowTemperature(qreal lowTemperature);
    void setHighTemperature(qreal highTemperature);
    void setSubtitle(QString subtitle);
    void setTitle(QString title);
    void setDescription(QString description);
    void setWeatherIcon(QString icon);

    qreal m_temperature;
    qreal m_lowTemperature;
    qreal m_highTemperature;
    QString m_subtitle;
    QString m_title;
    QString m_description;
    QString m_weatherIcon;
};

#endif // WEATHERCARD_H
