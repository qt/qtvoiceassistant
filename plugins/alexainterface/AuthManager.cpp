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

#include "AuthManager.h"

#include <QDebug>

using namespace alexaClientSDK::avsCommon::sdkInterfaces;

AuthManager::AuthManager(QObject* parent) :
        QObject(parent),
        m_authCheckCounter{0} {
}

void AuthManager::onRequestAuthorization(const std::string& url, const std::string& code) {
    m_executor.submit([this, url, code]() {
        m_authCheckCounter = 0;
        qDebug("To authorize, browse to %s and enter the code: %s", url.c_str(), code.c_str());
        if (code != "") {
            Q_EMIT authCodeReady(QString(url.c_str()), QString(code.c_str()));
        }
    });
}

void AuthManager::onCheckingForAuthorization() {
    m_executor.submit([this]() {
        qDebug("Check the remote server for authorization (%d)", ++m_authCheckCounter);
        Q_EMIT checkServerForAuthorization(m_authCheckCounter);
    });
}

void AuthManager::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
    m_executor.submit([this, newState, newError]() {
        m_authError = AuthError::Success;
        if (newError != AuthObserverInterface::Error::SUCCESS )
        {
            m_authError = this->fromAVSAuthError(newError);
            Q_EMIT authErrorChanged();
            qDebug("Auth error changed (%d)", m_authError);
        }

        AuthState authState = AuthState::Uninitialized;

        if (newState == alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED) {
            authState = AuthState::Refreshed;
        }
        else if (newState == alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::EXPIRED) {
            authState = AuthState::Expired;
        }
        else if (newState == alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNRECOVERABLE_ERROR) {
            authState = AuthState::Unrecovarable_error;
        }

        if (m_authState == authState)
            return;
        m_authState = authState;
        Q_EMIT authStateChanged();
        qDebug("Auth state changed (%d)", authState);

        if ((m_authState == AuthState::Refreshed) && (m_authError == AuthError::Success)) {
            m_isLoggedIn = true;
            Q_EMIT isLoggedInChanged();
        }
    });
}

AuthManager::AuthError AuthManager::fromAVSAuthError( avsCommon::sdkInterfaces::AuthObserverInterface::Error error ) const
{
    switch (error) {
        case AuthObserverInterface::Error::UNKNOWN_ERROR:
            return AuthError::UnknownError;
        case AuthObserverInterface::Error::AUTHORIZATION_FAILED:
            return AuthError::AuthorizationFailed;
        case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT:
            return AuthError::UnauthorizedClient;
        case AuthObserverInterface::Error::SERVER_ERROR:
            return AuthError::ServerError;
        case AuthObserverInterface::Error::INVALID_REQUEST:
            return AuthError::InvalidRequest;
        case AuthObserverInterface::Error::INVALID_VALUE:
            return AuthError::InvalidValue;
        case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED:
            return AuthError::AuthorizationExpired;
        case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE:
            return AuthError::UnsupportedGrantType;
        case AuthObserverInterface::Error::INVALID_CODE_PAIR:
            return AuthError::InvalidCodePair;
        case AuthObserverInterface::Error::AUTHORIZATION_PENDING:
            return AuthError::AuthorizationPending;
        case AuthObserverInterface::Error::SLOW_DOWN:
            return AuthError::SlowDown;
        case AuthObserverInterface::Error::INTERNAL_ERROR:
            return AuthError::InternalError;
        case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID:
            return AuthError::InvalidCBLClientID;
        default:
            return AuthError::Success;
    }
}

void AuthManager::onLogout() {
    if (m_isLoggedIn) {
        m_isLoggedIn = false;
        Q_EMIT isLoggedInChanged();
    }
}
