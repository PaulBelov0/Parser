#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <QObject>
#include <QTimer>

class ResourceMonitor : public QObject {
    Q_OBJECT
public:
    explicit ResourceMonitor(QObject *parent = nullptr);
    void startMonitoring(int intervalSec = 5);

signals:
    void statusUpdate(int cpuUsage, int memUsage, float temperature);
    void overloadAlert();

private slots:
    void checkResources();

private:
    QTimer *m_timer;
    int getCpuUsage();
    int getMemoryUsage();
    float getCpuTemperature();
};

#endif // RESOURCE_MONITOR_H
