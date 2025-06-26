#include "resource_monitor.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

ResourceMonitor::ResourceMonitor(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ResourceMonitor::checkResources);
}

void ResourceMonitor::startMonitoring(int intervalSec) {
    m_timer->start(intervalSec * 1000);
}

void ResourceMonitor::checkResources() {
    int cpu = getCpuUsage();
    int mem = getMemoryUsage();
    float temp = getCpuTemperature();

    emit statusUpdate(cpu, mem, temp);

    if (cpu > 80 || mem > 90 || temp > 75.0f) {
        emit overloadAlert();
    }
}

int ResourceMonitor::getCpuUsage() {
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) return 0;

    QTextStream in(&file);
    QString line = in.readLine();
    QStringList values = line.split(' ', Qt::SkipEmptyParts);

    if (values.size() < 5) return 0;

    static qint64 lastIdle = 0, lastTotal = 0;

    qint64 idle = values[4].toLongLong();
    qint64 total = 0;
    for (int i = 1; i < values.size(); ++i) {
        total += values[i].toLongLong();
    }

    qint64 diffIdle = idle - lastIdle;
    qint64 diffTotal = total - lastTotal;

    lastIdle = idle;
    lastTotal = total;

    return diffTotal > 0 ? (100 - (100 * diffIdle) / diffTotal) : 0;
}

int ResourceMonitor::getMemoryUsage() {
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) return 0;

    qint64 total = 0, free = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("MemTotal:")) {
            total = line.split(' ', Qt::SkipEmptyParts)[1].toLongLong();
        } else if (line.startsWith("MemAvailable:")) {
            free = line.split(' ', Qt::SkipEmptyParts)[1].toLongLong();
        }
    }

    return total > 0 ? (100 - (100 * free) / total) : 0;
}

float ResourceMonitor::getCpuTemperature() {
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll().toFloat() / 1000.0f;
    }
    return 0.0f;
}
