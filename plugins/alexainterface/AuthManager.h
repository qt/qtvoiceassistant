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

#ifndef ALEXAINTERFACE_AUTHMANAGER_H
#define ALEXAINTERFACE_AUTHMANAGER_H

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <CBLAuthDelegate/CBLAuthRequesterInterface.h>
#include <RegistrationManager/RegistrationObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <QObject>

/**
 * This class manages the auth states
 */

using namespace alexaClientSDK;

class AuthManager
        : public QObject
        , public avsCommon::sdkInterfaces::AuthObserverInterface
        , public authorization::cblAuthDelegate::CBLAuthRequesterInterface
        , public registrationManager::RegistrationObserverInterface {
    Q_OBJECT

    Q_PROPERTY(AuthState authState READ authState NOTIFY authStateChanged)
    Q_PROPERTY(AuthError authError READ authError NOTIFY authErrorChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)

public:
    enum AuthState {
        Uninitialized,
        Refreshed,
        Expired,
        Unrecovarable_error
    };
    Q_ENUM(AuthState)

    enum AuthError {
        /// Success.
        Success,
        /// An unknown body containing no error field has been encountered.
        UnknownError,
        /// The client authorization failed.
        AuthorizationFailed,
        /// The client is not authorized to use authorization codes.
        UnauthorizedClient,
        /// The server encountered a runtime error.
        ServerError,
        /// The request is missing a required parameter, has an invalid value, or is otherwise improperly formed.
        InvalidRequest,
        /// One of the values in the request was invalid.
        InvalidValue,
        /// The authorization code is invalid, expired, revoked, or was issued to a different client.
        AuthorizationExpired,
        /// The client specified the wrong token type.
        UnsupportedGrantType,
        /// Invalid code pair provided in Code-based linking token request.
        InvalidCodePair,
        /// Waiting for user to authorize the specified code pair.
        AuthorizationPending,
        /// Client should slow down in the rate of requests polling for an access token.
        SlowDown,
        /// Internal error in client code.
        InternalError,
        /// Client ID not valid for use with code based linking.
        InvalidCBLClientID
    }; Q_ENUM(AuthError)

    explicit AuthManager(QObject* parent = nullptr);

    AuthState authState() const { return m_authState; }
    AuthError authError() const { return m_authError; }
    bool isLoggedIn() const { return m_isLoggedIn; }

    /// @name RegistrationObserverInterface Functions
    /// @{
    void onLogout() override;
    /// @}

    /// @name CBLAuthRequesterInterface Functions
    /// @{
    void onRequestAuthorization(const std::string& url, const std::string& code) override;
    void onCheckingForAuthorization() override;
    /// }

    /// @name AuthObserverInterface Methods
    /// @{
    void onAuthStateChange(
        avsCommon::sdkInterfaces::AuthObserverInterface::State newState,
        avsCommon::sdkInterfaces::AuthObserverInterface::Error newError) override;
    /// }

Q_SIGNALS:
    void authStateChanged();
    void authErrorChanged();
    void authCodeReady(QString authURL, QString authCode);
    /// The signal gets emitted whenever the application checks the remote server for authorization
    void checkServerForAuthorization(int attempt);
    void isLoggedInChanged();

private:

    AuthState m_authState = AuthState::Uninitialized;
    AuthError m_authError = AuthError::Success;
    bool m_isLoggedIn = false;

    /// Counter used to make repeated messages about checking for authorization distinguishable from each other.
    int m_authCheckCounter;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    avsCommon::utils::threading::Executor m_executor;

    AuthError fromAVSAuthError( avsCommon::sdkInterfaces::AuthObserverInterface::Error error ) const;
};


#endif  // ALEXAINTERFACE_AUTHMANAGER_H

