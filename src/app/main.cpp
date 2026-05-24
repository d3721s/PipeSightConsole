#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("PipeSightConsole"));
    QGuiApplication::setOrganizationName(QStringLiteral("PipeSight"));

    // Material works well on Android, Fusion on desktop. Override via env var if needed.
    QQuickStyle::setStyle(QStringLiteral("Material"));

    QQmlApplicationEngine engine;
    engine.loadFromModule(QStringLiteral("PipeSightConsole"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
