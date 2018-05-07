/**************************************************************************
 *                                                                        *
 * Copyright (C) 2016 Felix Rohrbach <kde@fxrh.de>                        *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 3         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <QtQuick/QQuickImageProvider>
#include <QtCore/QReadWriteLock>
#include "connection.h"

class ImageProvider: public QObject, public QQuickImageProvider
{
    Q_OBJECT
    public:
        ImageProvider(QMatrixClient::Connection* connection);

        QImage requestImage(const QString& id, QSize* pSize,
                              const QSize& requestedSize) override;

    public slots:
        void setConnection(QMatrixClient::Connection* connection);

    private:
        QMatrixClient::Connection* m_connection;
        QReadWriteLock m_lock;
};
