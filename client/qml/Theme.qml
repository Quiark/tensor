pragma Singleton

import QtQuick 2.0
import Qt.labs.settings 1.0
import Matrix 1.0

Settings {

    category: "Theme"

    property string textFont: "Consolas"
    property string nickFont: "Consolas"
    property int textSize: 11
    property string roomListBg: "#6a1b9a"
    property string chatBg: "#fdf6e3"
    property string chatFg: "black"
    property string roomListSelectedBg: "#9c27b0"
    property string unreadRoomFg: "white"
    property string highlightRoomFg: "white"
    property string normalRoomFg: "#dddddd"
    property string textInputFg: "black"
    property string nonMessageFg: "lightgrey"
}
