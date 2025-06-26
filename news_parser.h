#ifndef NEWS_PARSER_H
#define NEWS_PARSER_H

#include <QObject>
#include <QTimer>
#include <QThreadPool>

#include "news_item.h"
#include "telegram_bot.h"
#include "page_downloader.h"

class NewsParser : public QObject {
    Q_OBJECT
public:
    explicit NewsParser(const QString &configPath, QObject *parent = nullptr);
    ~NewsParser();

    void setThrottle(int ms) { m_throttleMs = ms; }

public slots:
    void start();
    void stop();

signals:
    void messageProcessed();
    void errorOccurred(const QString &error);

private slots:
    void processContent(const QByteArray &content, const QUrl &url);
    void handleDownloadError(const QString &error, const QUrl &url);

private:
    void loadConfig();
    void initDatabase();
    void throttleIfNeeded();

    TelegramBot *m_bot;
    PageDownloader *m_downloader;
    QTimer *m_updateTimer;
    QThreadPool m_threadPool;
    int m_throttleMs = 0;
    QString m_dbPath;
};

#endif // NEWS_PARSER_H
