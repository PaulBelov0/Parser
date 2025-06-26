#ifndef NEWS_PARSER_H
#define NEWS_PARSER_H

#include <QObject>
#include <QTimer>
#include <QThreadPool>
#include <QMutex>
#include <QSqlDatabase>
#include "news_item.h"
#include "telegram_bot.h"
#include "page_downloader.h"

class NewsParser : public QObject
{
    Q_OBJECT
public:
    explicit NewsParser(const QString &configPath, QObject *parent = nullptr);
    ~NewsParser();

    void setThrottle(int ms);
    void start();
    void stop();

signals:
    void messageReady(const QString &message, const NewsItem &item);
    void errorOccurred(const QString &error);

private slots:
    void processNewsContent(const QByteArray &content, const QUrl &url);
    void handleDownloadError(const QString &error, const QUrl &url);

private:
    void loadConfig();
    void initDatabase();
    bool isNewsSent(const NewsItem &item);
    void saveSentNews(const NewsItem &item);
    QList<NewsItem> parseNews(const QByteArray &content, const QString &parserType);
    QString formatNewsMessage(const NewsItem &item) const;

    TelegramBot *m_bot;
    PageDownloader *m_downloader;
    QTimer *m_updateTimer;
    QThreadPool m_threadPool;
    QMutex m_dbMutex;
    QSqlDatabase m_database;
    QString m_configPath;
    QString m_dbPath;
    int m_throttleMs = 0;
    QString m_parserType = "simple";
    QMap<QUrl, int> m_updateIntervals;
};

#endif // NEWS_PARSER_H
