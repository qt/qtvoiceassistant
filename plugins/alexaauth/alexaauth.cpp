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

#ifdef ALEXA_QT_WEBENGINE
#include <QWebEngineCookieStore>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#endif
#include <QDebug>
#include <QFile>

AlexaAuth::AlexaAuth(QObject *parent) : QObject(parent)
{
#ifdef ALEXA_QT_WEBENGINE
    m_authPage.profile()->clearHttpCache();
    m_authPage.profile()->cookieStore()->deleteAllCookies();
    m_authPage.profile()->setHttpAcceptLanguage("en-US,en;q=0.9");
    m_httpUserAgent = m_authPage.profile()->httpUserAgent();
    m_error = ErrorState::None;
#else
    qDebug() << "QWebEngine not available, cannot authorize automatically.";
    m_error = AlexaAuth::WebEngineNotAvailable;
#endif
}

void AlexaAuth::setIsAuthorizing(bool isAuthorizing)
{
    if (m_isAuthorizing == isAuthorizing)
        return;

    m_isAuthorizing = isAuthorizing;
    emit isAuthorizingChanged(m_isAuthorizing);
}

void AlexaAuth::setAuthCode(QString authCode)
{
    qDebug() << Q_FUNC_INFO << " " << authCode;
    if (m_authCode == authCode)
        return;

    m_authCode = authCode;
    emit authCodeChanged(m_authCode);
}

void AlexaAuth::setAuthUrl(QUrl authUrl)
{
    qDebug() << Q_FUNC_INFO << " " << authUrl;
    if (m_authUrl == authUrl)
        return;

    m_authUrl = authUrl;
    emit authUrlChanged(m_authUrl);
}

void AlexaAuth::setError(AlexaAuth::ErrorState error)
{
    if (m_error == error)
        return;

    if (error != AlexaAuth::None) {
        setIsAuthorizing(false);
    }

    m_error = error;
#ifdef ALEXA_QT_WEBENGINE
    QObject::disconnect( &m_authPage, &QWebEnginePage::loadFinished, this, &AlexaAuth::authPageLoaded);
#endif
    emit errorChanged(m_error);
}

void AlexaAuth::setHttpUserAgent(QString httpUserAgent)
{
    qDebug() << Q_FUNC_INFO << httpUserAgent;
    if (m_httpUserAgent == httpUserAgent)
        return;

    m_httpUserAgent = httpUserAgent;
#ifdef ALEXA_QT_WEBENGINE
    m_authPage.profile()->setHttpUserAgent(m_httpUserAgent);
#endif
    emit httpUserAgentChanged(m_httpUserAgent);
}

void AlexaAuth::setAuthorizationSucceed(bool authorizationSucceed)
{
    if (m_authorizationSucceed == authorizationSucceed)
        return;

    m_authorizationSucceed = authorizationSucceed;
    emit authorizationSucceedChanged(m_authorizationSucceed);
}

void AlexaAuth::setEmail(QString email)
{
    if (m_email == email)
        return;
    m_email = email;
    emit emailChanged(m_email);
}

void AlexaAuth::setPassword(QString password)
{
    if (m_password == password)
        return;
    m_password = password;
    emit passwordChanged(m_password);
}

void AlexaAuth::authorize()
{
    qDebug() << Q_FUNC_INFO << " " << m_authUrl;
#ifdef ALEXA_QT_WEBENGINE
    if (parseJson()) {
        QObject::connect( &m_authPage, &QWebEnginePage::loadFinished, this, &AlexaAuth::authPageLoaded);
        setIsAuthorizing(true);
        m_authPage.load(m_authUrl);
    }
#endif
}

#ifdef ALEXA_QT_WEBENGINE
bool AlexaAuth::parseJson()
{
    qDebug() << Q_FUNC_INFO;
    if (qEnvironmentVariableIsSet("ALEXA_SDK_CONFIG_FILE")) {
        QFile file(qEnvironmentVariable("ALEXA_SDK_CONFIG_FILE"));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

            QJsonParseError jsonError;
            QTextStream in(&file);
            QString lines, line;
            int indx = 0;
            // Remove comment lines because Qt doesn't parse those
            while (!in.atEnd()) {
                line = in.readLine();
                indx = line.indexOf("//");
                if ( indx >= 0 ) {
                  lines += line.mid(0, indx);
                } else {
                  lines += line + "\n";
                }
            }
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(lines.toUtf8(), &jsonError);

            if (jsonError.error != QJsonParseError::NoError){
                qDebug() << "Cannot parse AlexaClientSDKConfig.json: " << jsonError.errorString();
                setError(AlexaAuth::ConfigFileFailure);
                return false;
            }
            return true;
        } else {
            qWarning() << "Couldn't open the config file AlexaClientSDKConfig.json";
            setError(AlexaAuth::ConfigFileFailure);
            return false;
        }
    } else {
        qWarning() << "Couldn't read the environment variable ALEXA_SDK_CONFIG_FILE";
        setError(AlexaAuth::ConfigFileFailure);
        return false;
    }
}

void AlexaAuth::authPageLoaded(bool ok)
{
    qDebug() << Q_FUNC_INFO << " " << ok << " " << m_authPage.title();
    if (ok) {
        if (m_authPage.title() == HTML_TITLE_FIRST) {
            QTimer::singleShot(2000, this, &AlexaAuth::signinToAmazon);

        } else if (m_authPage.title() == HTML_TITLE_SECOND) {
            QTimer::singleShot(1000, this, &AlexaAuth::registerDevice);

        } else {
            qWarning() << "Unknown HTML title " << m_authPage.title();
            setError(AlexaAuth::AutomaticAuthFailed);
        }
    } else {
        qWarning() << "Something went wrong to load the auth page";
        setError(AlexaAuth::AutomaticAuthFailed);
    }
}

void AlexaAuth::signinToAmazon()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementsByClassName('") + TAG_ALERT_HEADING_ID + "')[0].textContent", [this](const QVariant &cb) {
        if (cb.isNull() || cb.toString() == "" || cb.toString() == HTML_ENABLE_COOKIES) {
            inputEMail();
        } else if (cb.toString() == HTML_IMPORTANT_MESSAGE) {
            qWarning() << "Image capture detected! Cannot proceed automatically. Please, authorize manually on " << m_authUrl;
            setError(AlexaAuth::ImageRecognizionRequired);
        } else {
            qDebug() << "Something went wrong in " << Q_FUNC_INFO << " " << cb.toString();
            setError(AlexaAuth::AutomaticAuthFailed);
        }
    });
}

void AlexaAuth::inputEMail()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_EMAIL_ID +"')", [this](const QVariant &cb) {
        if (cb.isNull()) {
            qWarning() << "Email field doesn't exist on the page.";
            setError(AlexaAuth::HtmlItemNotFound);
        } else {
            m_authPage.runJavaScript(QString("document.getElementById('") + TAG_EMAIL_ID + "').value='" + m_email + "'");
            QTimer::singleShot(2000, this, &AlexaAuth::inputPassword);
        }
    });
}

void AlexaAuth::inputPassword()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_PASSWORD_ID + "')", [this](const QVariant &cb){
        if (cb.isNull()) {
            qWarning() << "Password field doesn't exist on the page";
            setError(AlexaAuth::HtmlItemNotFound);
        } else {
            m_authPage.runJavaScript(QString("document.getElementById('") + TAG_PASSWORD_ID + "').value='" + m_password + "'");
            QTimer::singleShot(2000, this, &AlexaAuth::clickSignIn);
        }
    });
}

void AlexaAuth::clickSignIn()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_SIGN_IN_SUBMIT_ID + "')", [this](const QVariant &cb){
        if (cb.isNull()) {
            qWarning() << "Sign in button doesn't exist on the page";
            setError(AlexaAuth::HtmlItemNotFound);
        } else {
            m_authPage.runJavaScript(QString("document.getElementById('") + TAG_SIGN_IN_SUBMIT_ID + "').click()");
        }
    });
}

void AlexaAuth::registerDevice()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_SUCCESS_TITLE_ID + "').textContent", [this](const QVariant &cb) {
        if (cb.toString() == HTML_REGISTER_DEVICE) {
            inputCode();
        } else if (cb.toString() == HTML_SUCCESS) {
            qDebug() << "Automatic authorization completed successfully.";
            setIsAuthorizing(false);
            setAuthorizationSucceed(true);
        } else {
            qDebug() << "Something went wrong in " << Q_FUNC_INFO << " " << cb.toString();
            setError(AlexaAuth::AutomaticAuthFailed);
        }
    });
}

void AlexaAuth::inputCode()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_REGISTRATION_FIELD_ID + "')" , [this] (const QVariant &cb) {
        if (cb.isNull()) {
            qWarning() << "No field for authorization code!";
            setError(AlexaAuth::HtmlItemNotFound);
        } else if (m_authCode.length() > 0) {
            m_authPage.runJavaScript(QString("document.getElementById('") + TAG_REGISTRATION_FIELD_ID + "').value='" + m_authCode + "'");
            QTimer::singleShot(2000, this, &AlexaAuth::clickContinue);
        } else {
            qDebug() << "Authorization code was empty";
            setError(AlexaAuth::AutomaticAuthFailed);
        }
    });
}

void AlexaAuth::clickContinue()
{
    qDebug() << Q_FUNC_INFO;
    m_authPage.runJavaScript(QString("document.getElementById('") + TAG_CONTINUE_BUTTON_ID + "')", [this] (const QVariant &cb) {
        if (cb.isNull()) {
            qWarning() << "Not found 'continue' button";
            setError(AlexaAuth::HtmlItemNotFound);
        } else {
            m_authPage.runJavaScript(QString("document.getElementById('") + TAG_CONTINUE_BUTTON_ID + "').click()");
        }
    });
    // todo: check what happens if the code was wrong
}
#endif
