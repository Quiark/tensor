import QtQuick 2.0
import QtQuick.Controls 2.1
import Matrix 1.0
import Tensor 1.0
import "jschat.js" as JsChat

Rectangle {
    id: root
    color: Theme.chatBg

    property Connection currentConnection: null
    property var currentRoom: null
    property string status: ""

    function setRoom(room) {
        currentRoom = room
        messageModel.changeRoom(room)
        room.markAllMessagesAsRead()
        chatView.positionViewAtBeginning()
    }

    function setConnection(conn) {
        currentConnection = conn
    }

    function sendLine(text) {
        if (!currentRoom || !currentConnection)
            return
        if (text.trim().length === 0)
            return

        var type = RoomMessageEvent.Text
        var PREFIX_ME = '/me '
        if (text.startsWith(PREFIX_ME)) {
            text = text.substr(PREFIX_ME.length)
            type = RoomMessageEvent.Emote
        }
        currentRoom.postMessage(text, type)
        chatView.positionViewAtBeginning()
    }

    function scrollPage(amount) {
        scrollBar.position = Math.max(
                    0,
                    Math.min(1 - scrollBar.size,
                             scrollBar.position + amount * scrollBar.stepSize))
    }

    ListView {
        id: chatView
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        verticalLayoutDirection: ListView.BottomToTop
        model: MessageEventModel {
            id: messageModel
        }

        delegate: Row {
            id: message
            width: parent.width
            spacing: 8

            property bool contentIsText: contentType.startsWith("text/")

            Label {
                id: timelabel
                text: time.toLocaleTimeString("hh:mm:ss")
                color: "grey"
                width: 80
                horizontalAlignment: Text.AlignRight
            }
            Item {
                id: authorIcon
                width: height
                height: timelabel.height
                Rectangle {
                    anchors.fill: parent
                    color: authorlabel.color
                    visible: !authorIconImage.status
                }
                Image {
                    id: authorIconImage
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: author.avatarMediaId ? "image://mtx/" + author.avatarMediaId : ""
                }
            }

            Label {
                id: authorlabel
                width: 140
                elide: Text.ElideRight
                text: {
                    if (eventType == "emote")
                        return "* " + author.displayName
                    else if (eventType == "state")
                        return "***" //author.displayName
                    else
                        return author.displayName
                }
                font.family: Theme.nickFont
                font.italic: eventType == "emote" ? true : false
                color: (eventType != "state" && eventType != "other") ? JsChat.NickColoring.get(
                                                                        author.displayName) : Theme.nonMessageFg
                horizontalAlignment: Text.AlignRight
            }
            Label {
                visible: !imageItem.visible
                id: contentlabel
                text: {
                    if (eventType == "state")
                        return author.displayName + " " + display
                    else
                        return display
                }
                wrapMode: Text.Wrap
                width: parent.width - (x - parent.x) - spacing
                color: (eventType == "message" || eventType == "emote") ? Theme.chatFg : Theme.nonMessageFg
                linkColor: "black"
                textFormat: Text.RichText
                font.family: Theme.textFont
                font.pointSize: Theme.textSize
                font.italic: eventType == "emote" ? true : false
                onLinkActivated: Qt.openUrlExternally(link)
            }
            Item {
                id: imageItem
                width: 320
                height: 240
                visible: eventType === "image"
                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: visible ? "image://mtx/" + content.thumbnailMediaId : ""
                }
            }
        }

        section {
            property: "date"
            labelPositioning: ViewSection.CurrentLabelAtStart
            delegate: Rectangle {
                width: parent.width
                height: childrenRect.height
                color: Theme.chatBg
                Label {
                    width: parent.width
                    text: status + " " + section.toLocaleString(Qt.locale())
                    color: "grey"
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        onAtYBeginningChanged: {
            if (currentRoom && atYBeginning)
                currentRoom.getPreviousContent(50)
        }

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            stepSize: chatView.visibleArea.heightRatio / 3
        }
    }
}
