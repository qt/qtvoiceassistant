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

#ifndef ALEXAINTERFACE_CAPABILITIESMANAGER_H
#define ALEXAINTERFACE_CAPABILITIESMANAGER_H

#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <QObject>

/**
 * This class is used to observe changes to the state of the CapabilitiesDelegate.
 */

using namespace alexaClientSDK;

class CapabilitiesManager
        : public QObject
        , public avsCommon::sdkInterfaces::CapabilitiesObserverInterface
{
    Q_OBJECT

    Q_PROPERTY(CapabilitiesState capabilitiesState READ capabilitiesState NOTIFY capabilitiesStateChanged)
    Q_PROPERTY(CapabilitiesError capabilitiesError READ capabilitiesError NOTIFY capabilitiesErrorChanged)

public:
    enum class CapabilitiesState {
        Uninitialized,
        Success,
        FatalError,
        RetriableError
    };
    Q_ENUM(CapabilitiesState)

    enum class CapabilitiesError {
        Uninitialized,
        Success,
        UnknownError,
        Forbidden,
        ServerInternalError,
        BadRequest
    };
    Q_ENUM(CapabilitiesError)

    explicit CapabilitiesManager(QObject* parent = nullptr);

    CapabilitiesState capabilitiesState() const { return m_capabilitiesState; }
    CapabilitiesError capabilitiesError() const { return m_capabilitiesError; }

    /// @name CapabilitiesObserverInterface Methods
    /// @{
    void onCapabilitiesStateChange(
        avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State newState,
        avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error newError) override;
    /// }

Q_SIGNALS:
    void capabilitiesStateChanged();
    void capabilitiesErrorChanged();

private:
    CapabilitiesState m_capabilitiesState = CapabilitiesState::Uninitialized;
    CapabilitiesError m_capabilitiesError = CapabilitiesError::Uninitialized;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    avsCommon::utils::threading::Executor m_executor;

};

#endif // ALEXAINTERFACE_CAPABILITIESMANAGER_H
