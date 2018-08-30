/**************************************************************************
 *                                                                        *
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>                        *
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

//#include "../quaternionroom.h"
#include <QtCore/QAbstractListModel>

#include <room.h>

typedef QMatrixClient::Room QuaternionRoom;

class MessageEventModel: public QAbstractListModel
{
        Q_OBJECT
    public:
        explicit MessageEventModel(QObject* parent = nullptr);

        Q_INVOKABLE void changeRoom(QuaternionRoom* room);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

    private slots:
        int refreshEvent(const QString& eventId);
        void refreshRow(int row);

    private:
        QuaternionRoom* m_currentRoom;
        QString lastReadEventId;
        int rowBelowInserted = -1;
        bool movingEvent = 0;

        int timelineBaseIndex() const;
        QDateTime makeMessageTimestamp(const QuaternionRoom::rev_iter_t& baseIt) const;
        QString renderDate(QDateTime timestamp) const;
        bool isUserActivityNotable(const QuaternionRoom::rev_iter_t& baseIt) const;

        void refreshLastUserEvents(int baseRow);
        void refreshEventRoles(int row, const QVector<int>& roles = {});
        int refreshEventRoles(const QString& eventId,
                              const QVector<int>& roles = {});
};
