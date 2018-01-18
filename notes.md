## Crash when leaving room ##

Got invited to a room by IRC NickServ. Open it and right - click to leave. App crashes in
`MessageEventModel::changeRoom` because apparently the room is now deleted.

It crashes when I leave twice (because the room somehow didn't close). It did get deleted on next start.

When a room is left by a different client, this client crashes in RoomListModel::at() called somewhere from QML.
