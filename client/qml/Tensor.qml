import QtQuick 2.0
import QtQuick.Controls 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0
import Matrix 1.0
import Tensor 1.0

Rectangle {
    id: window
    visible: true
    width: 960
    height: 600
    color: "#eee"

    property bool initialised: false
    property int syncIx: 0
    signal componentsComplete();

    Connection {
        id: connection
        stateSaveFile: (StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/state.json")
    }
    Settings   {
        id: settings

        property string user: ""
        property string token: ""

        property alias winWidth: window.width
        property alias winHeight: window.height
    }

    function resync() {
        if (!initialised) {
            roomListItem.init()

            login.visible = false
            mainView.visible = true
            initialised = true
        }
        syncIx += 1
        connection.sync(30000)

        // every now and then but not on the first sync
        if ((syncIx % 30) == 2) connection.saveState()
    }

    function reconnect() {
        connection.connectWithToken(connection.userId(), connection.token())
    }

    function login(user, pass, connect) {
        if(!connect) connect = connection.connectToServer

        connection.connected.connect(function() {
            settings.user = connection.userId()
            settings.token = connection.token()
            roomView.displayStatus("connected")

            connection.syncError.connect(reconnect)
            connection.syncError.connect(function() { roomView.displayStatus("sync error")})
            connection.resolveError.connect(reconnect)
            connection.resolveError.connect(function() { roomView.displayStatus("resolve error")})
            connection.syncDone.connect(resync)
            connection.syncDone.connect(function() { roomView.displayStatus("synced") })
            connection.reconnected.connect(resync)

            componentsComplete.connect(function() {
                connection.loadState()
                connection.sync()
            })
        })

        connection.loginError.connect(function() {
            login.restore("Login invalid")
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

            handleDelegate: Rectangle {
                border.width: 0
            }

            RoomList {
                id: roomListItem
                width: parent.width / 5
                height: parent.height

                Component.onCompleted: {
                    setConnection(connection)
                    enterRoom.connect(roomView.setRoom)
                    joinRoom.connect(connection.joinRoom)
                    leaveRoom.connect(connection.leaveRoom)
                    componentsComplete();
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
            var user = settings.user
            var token = settings.token
            if(user && token) {
                login.login(true)
                window.login(user, token, connection.connectWithToken)
            }
        }
    }
}
