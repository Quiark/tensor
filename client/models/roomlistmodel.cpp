/******************************************************************************
 * Copyright (C) 2016 Felix Rohrbach <kde@fxrh.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "roomlistmodel.h"

#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtCore/QDebug>

#include "connection.h"
#include "room.h"

const int RoomEventStateRole = Qt::UserRole + 1;

RoomListModel::RoomListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_connection = 0;
}

RoomListModel::~RoomListModel()
{
}

void RoomListModel::setConnection(QMatrixClient::Connection* connection)
{
    beginResetModel();
    m_connection = connection;
    m_rooms.clear();

    connect( m_connection, &QMatrixClient::Connection::invitedRoom,
             this, &RoomListModel::updateRoom);
    connect( m_connection, &QMatrixClient::Connection::joinedRoom,
             this, &RoomListModel::updateRoom);
    connect( m_connection, &QMatrixClient::Connection::leftRoom,
             this, &RoomListModel::updateRoom);
    connect( m_connection, &QMatrixClient::Connection::aboutToDeleteRoom,
             this, &RoomListModel::deleteRoom);


    // connect( connection, &QMatrixClient::Connection::newRoom, this, &RoomListModel::addRoom );
    // connect( connection, &QMatrixClient::Connection::leftRoom, this, &RoomListModel::removeRoom );
    for( QMatrixClient::Room* room: connection->roomMap().values() ) {
        connect( room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
        m_rooms.append(room);
    }
    endResetModel();
}

QMatrixClient::Room* RoomListModel::roomAt(int row)
{
    return m_rooms.at(row);
}

void RoomListModel::connectRoomSignals(QMatrixClient::Room* room)
{
    connect( room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
    connect( room, &QMatrixClient::Room::unreadMessagesChanged, this, &RoomListModel::unreadMessagesChanged );
    connect( room, &QMatrixClient::Room::highlightCountChanged, this, &RoomListModel::highlightCountChanged );
}

void RoomListModel::updateRoom(QMatrixClient::Room* room,
                               QMatrixClient::Room* prev)
{
    // There are two cases when this method is called:
    // 1. (prev == nullptr) adding a new room to the room list
    // 2. (prev != nullptr) accepting/rejecting an invitation or inviting to
    //    the previously left room (in both cases prev has the previous state).
    if (prev == room)
    {
        qCritical() << "RoomListModel::updateRoom: room tried to replace itself";
        refresh(room);
        return;
    }
    if (prev && room->id() != prev->id())
    {
        qCritical() << "RoomListModel::updateRoom: attempt to update room"
                    << room->id() << "to" << prev->id();
        // That doesn't look right but technically we still can do it.
    }
    // Ok, we're through with pre-checks, now for the real thing.
    auto* newRoom = room;
    const auto it = std::find_if(m_rooms.begin(), m_rooms.end(),
          [=](const QMatrixClient::Room* r) { return r == prev || r == newRoom; });
    if (it != m_rooms.end())
    {
        const int row = it - m_rooms.begin();
        // There's no guarantee that prev != newRoom
        if (*it == prev && *it != newRoom)
        {
            prev->disconnect(this);
            m_rooms.replace(row, newRoom);
            connectRoomSignals(newRoom);
        }
        emit dataChanged(index(row), index(row));
    }
    else
    {
        beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
        doAddRoom(newRoom);
        endInsertRows();
    }
}

void RoomListModel::deleteRoom(QMatrixClient::Room* room)
{
    auto i = m_rooms.indexOf(room);
    if (i == -1)
        return; // Already deleted, nothing to do

    beginRemoveRows(QModelIndex(), i, i);
    m_rooms.removeAt(i);
    endRemoveRows();
}

void RoomListModel::doAddRoom(QMatrixClient::Room* r)
{
    if (r != nullptr)
    {
        m_rooms.append(r);
        connectRoomSignals(r);
    } else
    {
        qCritical() << "Attempt to add nullptr to the room list";
        Q_ASSERT(false);
    }
}

void RoomListModel::refresh(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    if (row == -1)
        qCritical() << "Room" << room->id() << "not found in the room list";
    else
        emit dataChanged(index(row), index(row));
}


int RoomListModel::rowCount(const QModelIndex& parent) const
{
    if( parent.isValid() )
        return 0;
    return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex& index, int role) const
{
    if( !index.isValid() )
        return QVariant();

    if( index.row() >= m_rooms.count() )
    {
        qDebug() << "UserListModel: something wrong here...";
        return QVariant();
    }
    QMatrixClient::Room* room = m_rooms.at(index.row());
    if( role == Qt::DisplayRole )
    {
		return room->displayName();
    }
	if ( role == RoomEventStateRole )
    {
		if (room->highlightCount() > 0) {
			return "highlight";
		} else if (room->hasUnreadMessages()) {
			return "unread";
		} else {
			return "normal";
		}
    }
    return QVariant();
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
	return QHash<int, QByteArray>({
					  std::make_pair(Qt::DisplayRole, QByteArray("display")),
					  std::make_pair(RoomEventStateRole, QByteArray("roomEventState"))
		  });
}

void RoomListModel::namesChanged(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

void RoomListModel::unreadMessagesChanged(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

void RoomListModel::highlightCountChanged(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}
