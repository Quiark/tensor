#include <QGuiApplication>
#include <QQuickView>
#include <QtQml>

#include "connection.h"
#include "room.h"
#include "user.h"
#include "jobs/syncjob.h"
#include "csapi/joining.h"
#include "csapi/inviting.h"
#include "csapi/leaving.h"
#include "models/messageeventmodel.h"
#include "models/roomlistmodel.h"
#include "imageprovider.h"
#include "settings.h"
using namespace QMatrixClient;

// https://forum.qt.io/topic/57809
Q_DECLARE_METATYPE(SyncJob*)
Q_DECLARE_METATYPE(Room*)

int main(int argc, char* argv[]) {
    QGuiApplication app(argc,argv);
    app.setOrganizationName("David A Roberts");
    app.setOrganizationDomain("davidar.io");
    app.setApplicationName("Tensor");
    app.setApplicationVersion("Q0.4");
    QSettings::setDefaultFormat(QSettings::IniFormat);

	// debugging
	QLoggingCategory::setFilterRules(QStringLiteral("libqmatrixclient.main.debug=true"));

    QQuickView view;
    if(qgetenv("QT_QUICK_CORE_PROFILE").toInt()) {
        QSurfaceFormat f = view.format();
        f.setProfile(QSurfaceFormat::CoreProfile);
        f.setVersion(4, 4);
        view.setFormat(f);
    }
    view.connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    new QQmlFileSelector(view.engine(), &view);
    ImageProvider* m_imageProvider(new ImageProvider(nullptr)); // No connection yet
    view.rootContext()->setContextProperty(QLatin1String("mtxImageProvider"), m_imageProvider);
    view.engine()->addImageProvider(QLatin1String("mtx"), m_imageProvider);


    qmlRegisterType<SyncJob>(); qRegisterMetaType<SyncJob*> ("SyncJob*");
    qmlRegisterType<JoinRoomJob>(); 	qRegisterMetaType<JoinRoomJob*> ("JoinRoomJob*");
    qmlRegisterType<LeaveRoomJob>(); 	qRegisterMetaType<LeaveRoomJob*> ("LeaveRoomJob*");
    qmlRegisterType<ForgetRoomJob>(); 	qRegisterMetaType<ForgetRoomJob*> ("ForgetRoomJob*");
    qmlRegisterType<Room>();    qRegisterMetaType<Room*>    ("Room*");
    qmlRegisterType<User>();    qRegisterMetaType<User*>    ("User*");
    qRegisterMetaType<MessageEventType>("MessageEventType");

    qmlRegisterType<Connection>        ("Matrix", 1, 0, "Connection");
    qmlRegisterType<MessageEventModel> ("Matrix", 1, 0, "MessageEventModel");
    qmlRegisterType<RoomListModel>     ("Matrix", 1, 0, "RoomListModel");
    qmlRegisterSingletonType(QUrl("qrc:/qml/Theme.qml"), "Tensor", 1, 0, "Theme");
    qmlRegisterUncreatableType<RoomMessageEvent>("Matrix", 1, 0, "RoomMessageEvent", "ENUM");

    view.setSource(QUrl("qrc:/qml/Tensor.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
	view.setTitle("Tensor " + app.applicationVersion());
    if(QGuiApplication::platformName() == QLatin1String("qnx") ||
       QGuiApplication::platformName() == QLatin1String("eglfs")) {
        view.showFullScreen();
    } else {
        view.show();
    }
    return app.exec();
}

