#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include "bot_instance.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("MultiBotLauncher");
    app.setApplicationVersion("2.0");

    // Настройка Qt для embedded систем
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);

    // Парсер командной строки
    QCommandLineParser parser;
    parser.setApplicationDescription("Multi Telegram Bot Launcher for Raspberry Pi");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configDirOption(
        "d", "config-dir",
        "Directory with bot configs",
        "configs"
        );
    parser.addOption(configDirOption);

    parser.process(app);

    // Загрузка конфигов
    QDir configDir(parser.value(configDirOption));
    if (!configDir.exists()) {
        qCritical() << "Config directory not found:" << configDir.path();
        return 1;
    }

    QStringList configs = configDir.entryList({"*.json"}, QDir::Files);
    if (configs.isEmpty()) {
        qCritical() << "No config files found in" << configDir.path();
        return 1;
    }

    // Создание экземпляров ботов
    QList<BotInstance*> bots;
    for (const QString &config : configs) {
        QString configPath = configDir.filePath(config);
        bots.append(new BotInstance(configPath, &app));
        qInfo() << "Initialized bot with config:" << config;
    }

    // Запуск всех ботов
    for (BotInstance *bot : bots) {
        QTimer::singleShot(1000, bot, &BotInstance::start); // Задержка между запусками
    }

    return app.exec();
}
