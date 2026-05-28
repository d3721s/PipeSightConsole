#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>

#include "native_notifier.h"

namespace {

QFile *g_logFile = nullptr;
QMutex g_logMutex;

void messageHandler(QtMsgType type, const QMessageLogContext &ctx,
                    const QString &msg) {
  QMutexLocker lock(&g_logMutex);
  if (!g_logFile)
    return;

  const char *level = "INFO";
  switch (type) {
  case QtDebugMsg:
    level = "DEBUG";
    break;
  case QtInfoMsg:
    level = "INFO";
    break;
  case QtWarningMsg:
    level = "WARN";
    break;
  case QtCriticalMsg:
    level = "ERROR";
    break;
  case QtFatalMsg:
    level = "FATAL";
    break;
  }

  QTextStream out(g_logFile);
  out << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << " ["
      << level << "] ";
  if (ctx.category && *ctx.category)
    out << ctx.category << ": ";
  out << msg << "\n";
  g_logFile->flush();

  fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}

void setupLogging() {
  const QString path =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
      QStringLiteral("/pipesight.log");
  QDir().mkpath(QFileInfo(path).absolutePath());

  g_logFile = new QFile(path);
  g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

  qInstallMessageHandler(messageHandler);
  qInfo() << "PipeSight Console started";
}

} // namespace

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName(QStringLiteral("PipeSightConsole"));
  QGuiApplication::setOrganizationName(QStringLiteral("PipeSight"));

  setupLogging();

  QQuickStyle::setStyle(QStringLiteral("Material"));

  QQmlApplicationEngine engine;
  engine.loadFromModule(QStringLiteral("PipeSightConsole"),
                        QStringLiteral("Main"));
  if (engine.rootObjects().isEmpty())
    return -1;

  QTimer::singleShot(0, [] {
    pipesight::viewmodels::NativeNotifier notifier;
    notifier.notify(QStringLiteral("PipeSightConsole"),
                    QStringLiteral("PipeSightConsole 已启动"));
  });

  return app.exec();
}
