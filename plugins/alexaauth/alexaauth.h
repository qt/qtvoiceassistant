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
#ifdef ALEXA_QT_WEBENGINE
#include <QWebEnginePage>
#include <QWebEngineProfile>

// ## Step 1, login to amazon.developer.com
#define HTML_TITLE_FIRST "Amazon Sign-In"
#define HTML_IMPORTANT_MESSAGE "Important Message!"
#define HTML_ENABLE_COOKIES "Please Enable Cookies to Continue"
#define TAG_ALERT_HEADING_ID "a-alert-heading"
#define TAG_EMAIL_ID "ap_email"
#define TAG_PASSWORD_ID "ap_password"
#define TAG_SIGN_IN_SUBMIT_ID "signInSubmit"

// ## Step 2, give authorization code
#define HTML_TITLE_SECOND "Amazon Two-Step Verification"
#define TAG_REGISTRATION_FIELD_ID "cbl-registration-field"
#define TAG_CONTINUE_BUTTON_ID "cbl-continue-button"

// ## Step 3, success
#define HTML_REGISTER_DEVICE "Register Your Device"
#define HTML_SUCCESS "Success!"
#define TAG_SUCCESS_TITLE_ID "cbl-page-title"
#endif

class AlexaAuth : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isAuthorizing READ isAuthorizing NOTIFY isAuthorizingChanged)
    Q_PROPERTY(QString authCode READ authCode WRITE setAuthCode NOTIFY authCodeChanged)
    Q_PROPERTY(QUrl authUrl READ authUrl WRITE setAuthUrl NOTIFY authUrlChanged)
    Q_PROPERTY(ErrorState error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString httpUserAgent READ httpUserAgent WRITE setHttpUserAgent NOTIFY httpUserAgentChanged)
    Q_PROPERTY(bool authorizationSucceed READ authorizationSucceed NOTIFY authorizationSucceedChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

public:

    enum ErrorState {
        None,
        WebEngineNotAvailable,
        ConfigFileFailure,
        HtmlItemNotFound,
        ImageRecognizionRequired,
        AutomaticAuthFailed
    };
    Q_ENUM(ErrorState)

    explicit AlexaAuth(QObject *parent = nullptr);

    Q_INVOKABLE void authorize();

    bool isAuthorizing() const { return m_isAuthorizing; }
    QString authCode() const { return m_authCode; }
    QUrl authUrl() const { return m_authUrl; }
    ErrorState error() const { return m_error; }
    QString httpUserAgent() const { return m_httpUserAgent; }
    bool authorizationSucceed() const { return m_authorizationSucceed; }
    QString email() const { return m_email; }
    QString password() const { return m_password; }

    void setIsAuthorizing(bool isAuthorizing);
    void setAuthCode(QString authCode);
    void setAuthUrl(QUrl authUrl);
    void setError(AlexaAuth::ErrorState error);
    void setHttpUserAgent(QString httpUserAgent);
    void setAuthorizationSucceed(bool authorizationSucceed);
    void setEmail(QString email);
    void setPassword(QString password);


signals:
    void isAuthorizingChanged(bool isAuthorizing);
    void authCodeChanged(QString authCode);
    void authUrlChanged(QUrl authUrl);
    void errorChanged(AlexaAuth::ErrorState error);
    void httpUserAgentChanged(QString httpUserAgent);
    void authorizationSucceedChanged(bool authorizationSucceed);
    void emailChanged(QString email);
    void passwordChanged(QString password);

public slots:


private:
#ifdef ALEXA_QT_WEBENGINE
    bool parseJson();
    void authPageLoaded(bool ok);
    void signinToAmazon();
    void inputEMail();
    void inputPassword();
    void clickSignIn();
    void registerDevice();
    void inputCode();
    void clickContinue();

    QWebEnginePage m_authPage;
#endif
    QString m_authCode;
    bool m_isAuthorizing = false;
    QUrl m_authUrl;
    ErrorState m_error = ErrorState::None;
    QString m_httpUserAgent;
    bool m_authorizationSucceed = false;
    QString m_email;
    QString m_password;
};

#endif // ALEXAAUTH_H
