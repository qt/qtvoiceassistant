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

#ifndef ALEXAAUTH_H
#define ALEXAAUTH_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QQmlEngine>


// ## Step 1, login to amazon.developer.com
#define HTML_TITLE_FIRST "Amazon Sign-In"
#define HTML_IMPORTANT_MESSAGE "Important Message!"
#define HTML_ENABLE_COOKIES "Please Enable Cookies to Continue"
#define TAG_ALERT_HEADING_ID "a-alert-heading"
#define TAG_EMAIL_ID "ap_email"
#define TAG_PASSWORD_ID "ap_password"
#define TAG_SIGN_IN_SUBMIT_ID "signInSubmit"
#define TAG_CAPTCHA_IMAGE_ID "auth-captcha-image"
#define TAG_CAPTCHA_GUESS_ID "auth-captcha-guess"

// ## Step 2, give authorization code
#define HTML_TITLE_SECOND "Amazon Two-Step Verification"
#define HTML_TITLE_ERROR_CAPTCHA "There was a problem"
#define TAG_REGISTRATION_FIELD_ID "cbl-registration-field"
#define TAG_CONTINUE_BUTTON_ID "cbl-continue-button"

// ## Step 3, success
#define HTML_REGISTER_DEVICE "Register Your Device"
#define HTML_SUCCESS "Success!"
#define TAG_SUCCESS_TITLE_ID "cbl-page-title"

class AlexaAuth : public QObject
{
    Q_OBJECT

public:
    enum ErrorState {
        NoError,
        WebEngineNotAvailable,
        ConfigFileFailure,
        HtmlItemNotFound,
        ImageRecognizionRequired,
        AutomaticAuthFailed
    };
    Q_ENUM(ErrorState)

    enum AuthStage {
        AuthSignIn,
        AuthRegisterDevice,
        AuthError
    };
    Q_ENUM(AuthStage)

    enum SignInResult {
        SignInInputEmail,
        SignInCaptcha,
        SignInError
    };
    Q_ENUM(SignInResult)

    enum RegisterDeviceResult {
        RegisterDevice,
        RegisterDeviceSuccess,
        RegisterDeviceError
    };
    Q_ENUM(RegisterDeviceResult)

    enum JSAuthString {
        SignIn,
        CaptchaSrc,
        GetCaptchaInput,
        SetCaptcha,
        GetEmailInput,
        SetEmail,
        GetPasswordInput,
        SetPassword,
        GetClickSignIn,
        RegisterDeviceTitle,
        GetInputCode,
        SetInputCode,
        GetContinue,
        ClickElement
    };
    Q_ENUM(JSAuthString)

    explicit AlexaAuth(QObject *parent = nullptr);

    Q_INVOKABLE AlexaAuth::AuthStage getAuthStage(const QString &title);
    Q_INVOKABLE QString getJSString(AlexaAuth::JSAuthString id, const QString &value = "") const;
    Q_INVOKABLE AlexaAuth::SignInResult signinToAmazonResult(const QVariant &cb);
    Q_INVOKABLE AlexaAuth::RegisterDeviceResult registerDeviceResult(const QVariant &cb);
};

#endif // ALEXAAUTH_H
