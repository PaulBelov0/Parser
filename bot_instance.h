#ifndef BOT_INSTANCE_H
#define BOT_INSTANCE_H

#include <QObject>

#include "news_parser.h"
#include "resource_monitor.h"

class BotInstance : public QObject {
    Q_OBJECT
public:
    explicit BotInstance(const QString &configPath, QObject *parent = nullptr);
    void start();

signals:
    void resourceAlert(int cpuUsage, int memUsage);

private:
    NewsParser *m_parser;
    ResourceMonitor *m_monitor;
    QString m_configPath;
};

#endif // BOT_INSTANCE_H
