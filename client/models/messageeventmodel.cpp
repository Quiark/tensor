/******************************************************************************
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>
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

#include "messageeventmodel.h"

#include <algorithm>
#include <QtCore/QRegularExpression>
#include <QtCore/QDebug>

#include "connection.h"
#include "room.h"
#include "user.h"
#include "events/event.h"
#include "events/roommessageevent.h"
#include "events/roommemberevent.h"
#include "events/simplestateevents.h"
#include "events/redactionevent.h"

MessageEventModel::MessageEventModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_currentRoom = 0;
    m_connection = 0;
}

MessageEventModel::~MessageEventModel()
{
}

void MessageEventModel::changeRoom(QMatrixClient::Room* room)
{
    beginResetModel();
    if( m_currentRoom )
    {
        m_currentRoom->disconnect(this);
    }
    m_currentRoom = room;
    if( room )
    {
        using namespace QMatrixClient;
        connect( room, &Room::aboutToAddNewMessages,
                [=](const RoomEventsRange& events)
                {
                    beginInsertRows(QModelIndex(), 0, events.size() - 1);
                });
        connect( room, &Room::aboutToAddHistoricalMessages,
                [=](const RoomEventsRange& events)
                {
                    beginInsertRows(QModelIndex(),
                                    rowCount(), rowCount() + events.size() - 1);
                });
        connect( room, &Room::addedMessages,
                 this, &MessageEventModel::endInsertRows );
    }
    endResetModel();
}

void MessageEventModel::setConnection(QMatrixClient::Connection* connection)
{
    m_connection = connection;
}

// QModelIndex LogMessageModel::index(int row, int column, const QModelIndex& parent) const
// {
//     if( parent.isValid() )
//         return QModelIndex();
//     if( row < 0 || row >= m_currentMessages.count() )
//         return QModelIndex();
//     return createIndex(row, column, m_currentMessages.at(row));
// }
//
// LogMessageModel::parent(const QModelIndex& index) const
// {
//     return QModelIndex();
// }

int MessageEventModel::rowCount(const QModelIndex& parent) const
{
    if( !m_currentRoom || parent.isValid() )
        return 0;
	return m_currentRoom->messageEvents().size();
}

QVariant MessageEventModel::data(const QModelIndex& index, int role) const
{
    using namespace QMatrixClient;
    if( !m_currentRoom ||
			index.row() < 0 || index.row() >= m_currentRoom->messageEvents().size() )
        return QVariant();

    RoomEvent *event = (m_currentRoom->messageEvents().end() - index.row() - 1)->event();

    if( role == Qt::DisplayRole )
    {
        if (event->isRedacted())
        {
            auto reason = event->redactedBecause()->reason();
            if (reason.isEmpty())
                return tr("Redacted");
            else
                return tr("Redacted: %1")
                    .arg(event->redactedBecause()->reason());
        }

        if( event->type() == EventType::RoomMessage )
        {
            using namespace MessageEventContent;

            auto* e = static_cast<const RoomMessageEvent*>(event);
            if (e->hasTextContent() && e->mimeType().name() != "text/plain")
                return static_cast<const TextContent*>(e->content())->body;
            if (e->hasFileContent())
            {
                auto fileCaption = e->content()->fileInfo()->originalName;
                if (fileCaption.isEmpty())
                    fileCaption = m_currentRoom->prettyPrint(e->plainBody());
                if (fileCaption.isEmpty())
                    return tr("a file");
            }
            //return m_currentRoom->prettyPrint(e->plainBody());

            User* user = m_connection->user(e->senderId());
			return QString("%1 (%2): %3").arg(user->displayname()).arg(user->id()).arg(e->plainBody());
        }
        if( event->type() == EventType::RoomMember )
        {
            RoomMemberEvent* e = static_cast<RoomMemberEvent*>(event);
            switch( e->membership() )
            {
                case MembershipType::Join:
                    return QString("%1 (%2) joined the room").arg(e->displayName(), e->userId());
                case MembershipType::Leave:
                    return QString("%1 (%2) left the room").arg(e->displayName(), e->userId());
                case MembershipType::Ban:
                    return QString("%1 (%2) was banned from the room").arg(e->displayName(), e->userId());
                case MembershipType::Invite:
                    return QString("%1 (%2) was invited to the room").arg(e->displayName(), e->userId());
                case MembershipType::Knock:
                    return QString("%1 (%2) knocked").arg(e->displayName(), e->userId());
            }
        }
        if( event->type() == EventType::RoomAliases )
        {
            RoomAliasesEvent* e = static_cast<RoomAliasesEvent*>(event);
            return QString("Current aliases: %1").arg(e->aliases().join(", "));
        }
        if( event->type() == EventType::RoomEncryption )
        {
            return tr("activated End-to-End Encryption");
        }
        return "Unknown Event";
    }

    if( role == Qt::ToolTipRole )
    {
        return event->originalJson();
    }

    if( role == EventTypeRole )
    {
        if( event->type() == EventType::RoomMessage ) {
            RoomMessageEvent* re = static_cast<RoomMessageEvent*>(event);
            if (re->msgtype() == RoomMessageEvent::MsgType::Emote) {
                return "message.emote";
            } else if (re->msgtype() == RoomMessageEvent::MsgType::Notice) {
                return "message.notice";
            } else {
                return "message";
            }
        }
        return "other";
    }

    if( role == TimeRole )
    {
		return event->timestamp();
    }

    if( role == DateRole )
    {
        return event->timestamp().toLocalTime().date();
    }

    if( role == AuthorRole )
    {
        if( event->type() == EventType::RoomMessage )
        {
            RoomMessageEvent* e = static_cast<RoomMessageEvent*>(event);
            User *user = m_connection->user(e->senderId());
            return user->displayname();
        }
        return QVariant();
    }

    if( role == ContentRole )
    {
        if( event->type() == EventType::RoomMessage )
        {
            using namespace MessageEventContent;

            auto* e = static_cast<const RoomMessageEvent*>(event);
            switch (e->msgtype())
            {
                case MessageEventType::Image:
                case MessageEventType::File:
                case MessageEventType::Audio:
                case MessageEventType::Video:
                    return QVariant::fromValue(e->content()->originalJson);
                default:
                {
                    QString body = e->plainBody();
                    body.replace("<", "&lt;").replace(">", "&gt;");

                    QRegularExpression reLinks("(https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%_\\+.~#?!&//=]*))");
                    body.replace(reLinks, "<a href=\"\\1\">\\1</a>");

                    return body;
                }
            }
        }
        if( event->type() == EventType::RoomMember )
        {
            RoomMemberEvent* e = static_cast<RoomMemberEvent*>(event);
            switch( e->membership() )
            {
                case MembershipType::Join:
                    return QString("%1 (%2) joined the room").arg(e->displayName(), e->userId());
                case MembershipType::Leave:
                    return QString("%1 (%2) left the room").arg(e->displayName(), e->userId());
                case MembershipType::Ban:
                    return QString("%1 (%2) was banned from the room").arg(e->displayName(), e->userId());
                case MembershipType::Invite:
                    return QString("%1 (%2) was invited to the room").arg(e->displayName(), e->userId());
                case MembershipType::Knock:
                    return QString("%1 (%2) knocked").arg(e->displayName(), e->userId());
            }
        }
        if( event->type() == EventType::RoomAliases )
        {
            RoomAliasesEvent* e = static_cast<RoomAliasesEvent*>(event);
            return QString("Current aliases: %1").arg(e->aliases().join(", "));
        }
        return "Unknown Event";
    }
//     if( event->type() == EventType::Unknown )
//     {
//         UnknownEvent* e = static_cast<UnknownEvent*>(event);
//         return "Unknown Event: " + e->typeString() + "(" + e->content();
//     }
    return QVariant();
}

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[EventTypeRole] = "eventType";
    roles[TimeRole] = "time";
    roles[DateRole] = "date";
    roles[AuthorRole] = "author";
    roles[ContentRole] = "content";
    return roles;
}
