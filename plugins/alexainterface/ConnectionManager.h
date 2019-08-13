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

#ifndef ALEXAINTERFACE_CONNECTIONMANAGER_H
#define ALEXAINTERFACE_CONNECTIONMANAGER_H

#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <QObject>

/**
 * This class manages the connection status of the client
 */

using namespace alexaClientSDK;

class ConnectionManager
        : public QObject
        , public avsCommon::sdkInterfaces::ConnectionStatusObserverInterface {
    Q_OBJECT

    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(ChangedReason changedReason READ changedReason NOTIFY changedReasonChanged)

public:
    enum class ConnectionStatus {
        /// ACL is not connected to AVS.
        Disconnected,
        /// ACL is attempting to establish a connection to AVS.
        Pending,
        /// ACL is connected to AVS.
        Connected
    };
    Q_ENUM(ConnectionStatus)

    enum class ChangedReason {
        /// the non-reason, to be used when no reason is specified (i.e. the 'unset' value).
        None,
        /// the status changed to due to a successful operation.
        Success,
        /// the status changed due to an error from which there is no recovery.
        UnrecoverableError,
        /// the connection status changed due to the client interacting with the connection public api.
        ACLClientRequest,
        /// the connection attempt failed due to the connection object being disabled.
        ACLDisabled,
        /// the connection attempt failed due to dns resolution timeout.
        DNSTimeout,
        /// the connection attempt failed due to timeout.
        ConnectionTimeout,
        /// the connection attempt failed due to excessive load on the server.
        ConnectionThrottled,
        /// the access credentials provided to acl were invalid.
        InvalidAuth,
        /// there was a timeout sending a ping request.
        PingTimeout,
        /// there was a timeout writing to avs.
        WriteTimeout,
        /// there was a timeout reading from avs.
        ReadTimeout,
        /// there was an underlying protocol error.
        FailureProtocolError,
        /// there was an internal error within acl.
        InternalError,
        /// there was an internal error on the server.
        ServerInternalError,
        /// the server asked the client to reconnect.
        ServerSideDisconnect,
        /// the server endpoint has changed.
        ServerEndpointChanged
    };
    Q_ENUM(ChangedReason)

    explicit ConnectionManager(QObject* parent = nullptr);
    virtual ~ConnectionManager() override = default;

    ConnectionManager::ConnectionStatus connectionStatus() const { return m_connectionStatus; }
    ConnectionManager::ChangedReason changedReason() const { return m_changedReason; }

    void onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                   const ConnectionStatusObserverInterface::ChangedReason reason) override;

Q_SIGNALS:
    void connectionStatusChanged();
    void changedReasonChanged();

private:
    ConnectionManager::ConnectionStatus m_connectionStatus = ConnectionStatus::Disconnected;
    ConnectionManager::ChangedReason m_changedReason = ChangedReason::None;

    ConnectionManager::ChangedReason fromAVSChangedReason( avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason ) const;

    avsCommon::utils::threading::Executor m_executor;
};

#endif //ALEXAINTERFACE_CONNECTIONMANAGER_H
