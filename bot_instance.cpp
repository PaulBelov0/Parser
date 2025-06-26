#include "bot_instance.h"
#include <QCoreApplication>

BotInstance::BotInstance(const QString &configPath, QObject *parent)
    : QObject(parent), m_configPath(configPath) {

    m_parser = new NewsParser(configPath, this);
    m_monitor = new ResourceMonitor(this);

    connect(m_monitor, &ResourceMonitor::overloadAlert, [this]() {
        m_parser->setThrottle(1000); // Увеличиваем задержку при перегрузке
    });

    connect(m_monitor, &ResourceMonitor::statusUpdate, [](int cpu, int mem, float temp) {
        qDebug().nospace()
            << "Resource usage: CPU=" << cpu << "%, "
            << "MEM=" << mem << "%, "
            << "TEMP=" << temp << "°C";
    });
}

void BotInstance::start() {
    m_monitor->startMonitoring();
    m_parser->start();
}
