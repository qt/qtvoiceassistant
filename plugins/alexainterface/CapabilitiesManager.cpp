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

#include "CapabilitiesManager.h"

CapabilitiesManager::CapabilitiesManager(QObject* parent) : QObject(parent)
{
}

void CapabilitiesManager::onCapabilitiesStateChange(
    CapabilitiesObserverInterface::State newState,
    CapabilitiesObserverInterface::Error newError) {
    m_executor.submit([this, newState, newError]() {

        CapabilitiesError error = CapabilitiesError::Uninitialized;

        if (newError == CapabilitiesObserverInterface::Error::SUCCESS) {
            error = CapabilitiesError::Success;
        } else if (newError == CapabilitiesObserverInterface::Error::UNKNOWN_ERROR) {
            error = CapabilitiesError::UnknownError;
        } else if (newError == CapabilitiesObserverInterface::Error::FORBIDDEN) {
            error = CapabilitiesError::Forbidden;
        } else if (newError == CapabilitiesObserverInterface::Error::SERVER_INTERNAL_ERROR) {
            error = CapabilitiesError::ServerInternalError;
        } else if (newError == CapabilitiesObserverInterface::Error::BAD_REQUEST) {
            error = CapabilitiesError::BadRequest;
        }

        if (m_capabilitiesError != error) {
            m_capabilitiesError = error;
            Q_EMIT capabilitiesErrorChanged();
        }

        CapabilitiesState state = CapabilitiesState::Uninitialized;

        if (newState == CapabilitiesObserverInterface::State::FATAL_ERROR) {
            state = CapabilitiesState::FatalError;
        } else if (newState == CapabilitiesObserverInterface::State::RETRIABLE_ERROR) {
            state = CapabilitiesState::RetriableError;
        } else if (newState == CapabilitiesObserverInterface::State::SUCCESS) {
            state = CapabilitiesState::Success;
        }

        if (m_capabilitiesState != state) {
            m_capabilitiesState = state;
            Q_EMIT capabilitiesStateChanged();
        }
    });
}
