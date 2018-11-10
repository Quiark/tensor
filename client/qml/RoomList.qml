import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import Matrix 1.0
import Tensor 1.0
import "jschat.js" as JsChat

Rectangle {
    color: Theme.roomListBg

    signal enterRoom(var room)
    signal joinRoom(string name)
    signal leaveRoom(var room)
    signal forgetRoom(var room)

    property bool initialised: false

    RoomListModel {
        id: rooms

        onDataChanged: {
            // may have received a message but if focused, mark as read
            var room = currentRoom()
            if (room !== null)
                room.markAllMessagesAsRead()
        }
    }

    function setConnection(conn) {
        rooms.setConnection(conn)
    }

    function init() {
        var defaultRoom = "#tensor:matrix.org"
        initialised = true
        if (rooms.rowCount() === 0) {
            joinRoom(defaultRoom)
        } else {
            roomListView.currentIndex = 0
            enterRoom(rooms.roomAt(roomListView.currentIndex))
        }
    }

    function refresh() {
        if (roomListView.visible)
            roomListView.forceLayout()
    }

    function changeRoom(dir) {
        roomListView.currentIndex = JsChat.posmod(
                    roomListView.currentIndex + dir, roomListView.count)
        enterRoom(rooms.roomAt(roomListView.currentIndex))
    }

    function currentRoom() {
        if (roomListView.currentIndex < 0)
            return null
        var room = rooms.roomAt(roomListView.currentIndex)
        return room
    }

    Column {
        anchors.fill: parent

        ListView {
            id: roomListView
            model: rooms
            width: parent.width
            height: parent.height - textEntry.height

            delegate: Rectangle {
                width: parent.width
                height: Math.max(20, roomLabel.implicitHeight + 4)
                color: "transparent"

                Label {
                    id: roomLabel
                    text: display
                    color: roomEventState == "highlight" ? Theme.highlightRoomFg : (roomEventState == "unread" ? Theme.unreadRoomFg : Theme.normalRoomFg)
                    elide: Text.ElideRight
                    font.family: Theme.nickFont
                    font.bold: roomListView.currentIndex == index
                    anchors.margins: 2
                    anchors.leftMargin: 6
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onPressed: {
                        roomListView.currentIndex = index
                        enterRoom(rooms.roomAt(index))
                    }
                    onClicked: {
                        if (mouse.button === Qt.RightButton)
                            contextMenu.popup()
                    }
                }
            }

            highlight: Rectangle {
                height: 20
                radius: 2
                color: Theme.roomListSelectedBg
            }
            highlightMoveDuration: 0

            onCountChanged: if (initialised) {
                                roomListView.currentIndex = count - 1
                                enterRoom(rooms.roomAt(count - 1))
                            }

            Menu {
                id: contextMenu
                MenuItem {
                    text: qsTr("Leave")
                    onTriggered: {
                        var roomToLeave = currentRoom()
                        changeRoom(+1)
                        forgetRoom(roomToLeave.id)
                    }
                }
            }
        }

        TextField {
            id: textEntry
            width: parent.width
            placeholderText: qsTr("Join room...")
            onAccepted: {
                joinRoom(text)
                text = ""
            }

            style: TextFieldStyle {
                font: Theme.textFont
                textColor: Theme.textInputFg
                background: Rectangle {
                    border.width: 0
                    color: Theme.roomListBg
                }
            }
        }
    }
}
