/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 IVI UI.
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

#include "alexaauth.h"

#include <QJsonParseError>
#include <QDebug>
#include <QFile>
#include <QtWebEngine/QtWebEngine>
#include <QQuickWebEngineProfile>

AlexaAuth::AlexaAuth(QObject *parent) : QObject(parent)
{
    QtWebEngine::initialize();
    QQuickWebEngineProfile::defaultProfile()->cookieStore()->deleteAllCookies();
}

QString AlexaAuth::getJSString(AlexaAuth::JSAuthString id, const QString &value) const
{
    QString result;
    switch (id) {
    case SignIn:
        result = QString("document.getElementsByClassName('") + TAG_ALERT_HEADING_ID + "')[0].textContent";
        break;
    case CaptchaSrc:
        result = QString("document.getElementById('") + TAG_CAPTCHA_IMAGE_ID +"').src";
        break;
    case GetCaptchaInput:
        result = QString("document.getElementById('") + TAG_CAPTCHA_GUESS_ID + "')";
        break;
    case SetCaptcha:
        result = QString("document.getElementById('") + TAG_CAPTCHA_GUESS_ID + "').value='" + value + "'";
        break;
    case GetEmailInput:
        result = QString("document.getElementById('") + TAG_EMAIL_ID +"')";
        break;
    case SetEmail:
        result = QString("document.getElementById('") + TAG_EMAIL_ID + "').value='" + value+ "'";
        break;
    case GetPasswordInput:
        result = QString("document.getElementById('") + TAG_PASSWORD_ID +"')";
        break;
    case SetPassword:
        result = QString("document.getElementById('") + TAG_PASSWORD_ID + "').value='" + value + "'";
        break;
    case GetClickSignIn:
        result = QString("document.getElementById('") + TAG_SIGN_IN_SUBMIT_ID + "')";
        break;
    case RegisterDeviceTitle:
        result = QString("document.getElementById('") + TAG_SUCCESS_TITLE_ID + "').textContent";
        break;
    case GetInputCode:
        result = QString("document.getElementById('") + TAG_REGISTRATION_FIELD_ID + "')";
        break;
    case SetInputCode:
        result = QString("document.getElementById('") + TAG_REGISTRATION_FIELD_ID + "').value='" + value + "'";
        break;
    case GetContinue:
        result = QString("document.getElementById('") + TAG_CONTINUE_BUTTON_ID + "')";
        break;
    case ClickElement:
        result = value + ".click()";
        break;
    }

    return result;
}

AlexaAuth::AuthStage AlexaAuth::getAuthStage(const QString &title)
{
    if (title == HTML_TITLE_FIRST) {
        return AuthSignIn;
    } else if (title == HTML_TITLE_SECOND) {
        return AuthRegisterDevice;
    }
    qWarning() << "Unknown HTML title " << title;
    return AuthError;
}

AlexaAuth::SignInResult AlexaAuth::signinToAmazonResult(const QVariant &cb)
{
    if (cb.isNull() || cb.toString() == "" || cb.toString() == HTML_ENABLE_COOKIES) {
        return SignInInputEmail;
    } else if (cb.toString() == HTML_IMPORTANT_MESSAGE) {
        qWarning() << "Image capture detected!";
        return SignInCaptcha;
    } else if (cb.toString() == HTML_TITLE_ERROR_CAPTCHA) {
        qWarning() << "Image capture detected!, wrong captcha";
        return SignInCaptcha;
    }

    qDebug() << "Something went wrong in " << Q_FUNC_INFO << " " << cb.toString();
    return SignInError;
}

AlexaAuth::RegisterDeviceResult AlexaAuth::registerDeviceResult(const QVariant &cb)
{
    if (cb.toString() == HTML_REGISTER_DEVICE) {
        return AlexaAuth::RegisterDevice;
    } else if (cb.toString() == HTML_SUCCESS) {
        qDebug() << "Automatic authorization completed successfully.";
        return AlexaAuth::RegisterDeviceSuccess;
    } else {
        qDebug() << "Something went wrong in " << Q_FUNC_INFO << " " << cb.toString();
        return AlexaAuth::RegisterDeviceError;
    }
}
