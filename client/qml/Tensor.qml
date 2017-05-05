import QtQuick 2.0
import QtQuick.Controls 1.0
import Matrix 1.0

Rectangle {
    id: window
    visible: true
    width: 960
    height: 600
    color: "#eee"

    property bool initialised: false

    Connection { id: connection }
    Settings   { id: settings }

    function resync() {
        if(!initialised) {
            login.visible = false
            mainView.visible = true
            roomListItem.init()
            initialised = true
        }
        connection.sync(30000)
    }

    function login(user, pass, connect) {
        if(!connect) connect = connection.connectToServer

        // TODO: apparently reconnect is done with password but only a token is available so it won't reconnect
        connection.connected.connect(function() {
            settings.setValue("user",  connection.userId())
            settings.setValue("token", connection.token())

            connection.connectionError.connect(connection.reconnect)
            connection.syncDone.connect(resync)
            connection.reconnected.connect(resync)

            connection.sync()
        })

        var userParts = user.split(':')
        if(userParts.length === 1 || userParts[1] === "matrix.org") {
            connect(user, pass)
        } else {
            connection.resolved.connect(function() {
                connect(user, pass)
            })
            connection.resolveError.connect(function() {
                console.log("Couldn't resolve server!")
            })
            connection.resolveServer(userParts[1])
        }
    }

    Item {
        id: mainView
        anchors.fill: parent
        visible: false

        SplitView {
            anchors.fill: parent

            RoomList {
                id: roomListItem
                width: parent.width / 5
                height: parent.height

                Component.onCompleted: {
                    setConnection(connection)
                    enterRoom.connect(roomView.setRoom)
                    joinRoom.connect(connection.joinRoom)
                }
            }

            RoomView {
                id: roomView
                width: parent.width * 4/5
                height: parent.height
                Component.onCompleted: {
                    setConnection(connection)
                    roomView.changeRoom.connect(roomListItem.changeRoom)
                }
            }
        }
    }

    Login {
        id: login
        window: window
        anchors.fill: parent
        Component.onCompleted: {
            var user =  settings.value("user")
            var token = settings.value("token")
            if(user && token) {
                login.login(true)
                window.login(user, token, connection.connectWithToken)
            }
        }
    }
}
