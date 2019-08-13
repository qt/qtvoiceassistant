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

#include "ConnectionManager.h"


ConnectionManager::ConnectionManager(QObject* parent) : QObject (parent)
{
}

void ConnectionManager::onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status, const ConnectionStatusObserverInterface::ChangedReason reason) {
    m_executor.submit([this, status, reason]() {

        ChangedReason r = this->fromAVSChangedReason(reason);
        if (r != m_changedReason) {
            m_changedReason = r;
            Q_EMIT changedReasonChanged();
        }

        ConnectionStatus connectionStatus = m_connectionStatus;
        if (status == ConnectionStatusObserverInterface::Status::PENDING) {
            connectionStatus = ConnectionStatus::Pending;
        } else if (status == ConnectionStatusObserverInterface::Status::CONNECTED) {
            connectionStatus = ConnectionStatus::Connected;
        } else {
            connectionStatus = ConnectionStatus::Disconnected;
        }

        if (m_connectionStatus == connectionStatus)
            return;

        m_connectionStatus = connectionStatus;
        Q_EMIT connectionStatusChanged();
    });
}

ConnectionManager::ChangedReason ConnectionManager::fromAVSChangedReason( avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason ) const
{
    switch (reason) {
        case ConnectionStatusObserverInterface::ChangedReason::SUCCESS:
            return ChangedReason::Success;
        case ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR:
            return ChangedReason::UnrecoverableError;
        case ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST:
            return ChangedReason::ACLClientRequest;
        case ConnectionStatusObserverInterface::ChangedReason::ACL_DISABLED:
            return ChangedReason::ACLDisabled;
        case ConnectionStatusObserverInterface::ChangedReason::DNS_TIMEDOUT:
            return ChangedReason::DNSTimeout;
        case ConnectionStatusObserverInterface::ChangedReason::CONNECTION_TIMEDOUT:
            return ChangedReason::ConnectionTimeout;
        case ConnectionStatusObserverInterface::ChangedReason::CONNECTION_THROTTLED:
            return ChangedReason::ConnectionThrottled;
        case ConnectionStatusObserverInterface::ChangedReason::INVALID_AUTH:
            return ChangedReason::InvalidAuth;
        case ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT:
            return ChangedReason::PingTimeout;
        case ConnectionStatusObserverInterface::ChangedReason::WRITE_TIMEDOUT:
            return ChangedReason::WriteTimeout;
        case ConnectionStatusObserverInterface::ChangedReason::READ_TIMEDOUT:
            return ChangedReason::ReadTimeout;
        case ConnectionStatusObserverInterface::ChangedReason::FAILURE_PROTOCOL_ERROR:
            return ChangedReason::FailureProtocolError;
        case ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR:
            return ChangedReason::InternalError;
        case ConnectionStatusObserverInterface::ChangedReason::SERVER_INTERNAL_ERROR:
            return ChangedReason::ServerInternalError;
        case ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT:
            return ChangedReason::ServerSideDisconnect;
        case ConnectionStatusObserverInterface::ChangedReason::SERVER_ENDPOINT_CHANGED:
            return ChangedReason::ServerEndpointChanged;
        default:
            return ChangedReason::None;
    }
}
