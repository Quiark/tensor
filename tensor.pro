TEMPLATE = app

QT += qml quick
CONFIG += c++11 qml_debug

include(lib/libqmatrixclient.pri)

HEADERS += \
    client/models/messageeventmodel.h \
    client/models/roomlistmodel.h

SOURCES += client/main.cpp \
    client/models/messageeventmodel.cpp \
    client/models/roomlistmodel.cpp

RESOURCES += client/resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

ICON = tensor.icns
RC_ICONS = tensor.ico

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    notes.md

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

# qtdeploy config
win32 {
        DEPLOY_COMMAND = windeployqt
        TARGET_EXT = .exe
}
macx {
        DEPLOY_COMMAND = macdeployqt
        TARGET_EXT = .app
}

# was:
# debug, debug|release
CONFIG( release ) {
    # debug
    #DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_EXT}))
#} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK = $${DEPLOY_COMMAND} --qmldir $$PWD/client/qml $${DEPLOY_TARGET}
