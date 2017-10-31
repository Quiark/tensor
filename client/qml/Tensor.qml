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
    property var lastSync
    property bool roomListComplete: false
    signal componentsComplete();

    Timer {
        id: synctimer
        repeat: false
        onTriggered: {
            connection.sync(30000)
        }
    }

    Connection {
        id: connection
        property string stateSaveFile: (StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/state.json")
    }
    Settings   {
        id: settings

        property string user: ""
        property string token: ""

        property alias winWidth: window.width
        property alias winHeight: window.height

        property int minResyncMs: 4000
        property string deviceId: ""
    }

    function resync() {
        if (!initialised) {
            roomListItem.init()

            login.hide()
            mainView.visible = true
            initialised = true
        }
        syncIx += 1

        // timing
        var now = new Date()
        var delay = (now - lastSync) / 1000
        //console.log("..> synced in ", delay, " s <..")
        roomView.displayStatus("synced (in "+ delay +"s)")
        synctimer.interval = settings.minResyncMs - (delay * 1000)
        if (!(synctimer.interval > 0)) { // this expression also takes care of NaN
            synctimer.interval = 0
        }
        //console.log("resync in .. ", synctimer.interval, " ms")

        synctimer.start()
        lastSync = now

        // every now and then but not on the first sync
        if ((syncIx % 30) == 2) connection.saveState(connection.stateSaveFile)
    }

    function reconnect() {
        connection.connectWithToken(connection.userId(), connection.token(), connection.deviceId())
    }

    function login(user, pass, connectFn) {
        if (!connectFn) connectFn = connection.connectToServer

        connection.connected.connect(function() {
            settings.user = connection.userId()
            settings.token = connection.token()
            var deviceId = connection.deviceId()
            if (deviceId !== undefined) settings.deviceId = deviceId
            roomView.displayStatus("connected")

            connection.syncError.connect(reconnect)
            connection.syncError.connect(function() { roomView.displayStatus("sync error")})
            connection.resolveError.connect(reconnect)
            connection.resolveError.connect(function() { roomView.displayStatus("resolve error")})
            connection.syncDone.connect(resync)
            connection.reconnected.connect(resync)

            var startSyncFn = function() {
                connection.loadState(connection.stateSaveFile)
                connection.sync()
            }
            if (roomListComplete) startSyncFn()
            else componentsComplete.connect(startSyncFn)
        })

        connection.loginError.connect(function() {
            login.restore("Login invalid")
        })

        var userParts = user.split(':')
        if(userParts.length === 1 || userParts[1] === "matrix.org") {
            connectFn(user, pass, settings.deviceId)
        } else {
            connection.resolved.connect(function() {
                connectFn(user, pass, settings.deviceId)
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
                    // TODO if token not available, this will execute before its attached signal
                    setConnection(connection)
                    enterRoom.connect(roomView.setRoom)
                    joinRoom.connect(connection.joinRoom)
                    leaveRoom.connect(connection.leaveRoom)
                    roomListComplete = true
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
            if (user && token) {
                login.login(true)
                window.login(user, token, connection.connectWithToken)
            }
        }
    }
}
