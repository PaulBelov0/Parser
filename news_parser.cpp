#include "news_parser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QCryptographicHash>

NewsParser::NewsParser(const QString &configPath, QObject *parent)
    : QObject(parent), m_configPath(configPath) {

    m_bot = nullptr;
    m_downloader = new PageDownloader(this);
    m_updateTimer = new QTimer(this);

    connect(m_downloader, &PageDownloader::downloaded,
            this, &NewsParser::processNewsContent);
    connect(m_downloader, &PageDownloader::error,
            this, &NewsParser::handleDownloadError);
    connect(m_updateTimer, &QTimer::timeout,
            this, &NewsParser::checkForUpdates);

    m_threadPool.setMaxThreadCount(QThread::idealThreadCount() > 2 ?
                                       QThread::idealThreadCount() / 2 : 1);

    loadConfig();
    initDatabase();
}

void NewsParser::processNewsContent(const QByteArray &content, const QUrl &url) {
    QtConcurrent::run(&m_threadPool, [this, content, url]() {
        QList<NewsItem> newsItems = parseNews(content, m_parserType);

        QMutexLocker locker(&m_dbMutex);
        for (NewsItem &item : newsItems) {
            item.source = url.toString();
            item.hash = QCryptographicHash::hash(
                            (item.url + item.title).toUtf8(),
                            QCryptographicHash::Md5).toHex();

            if (!isNewsSent(item)) {
                QString message = formatNewsMessage(item);
                emit messageReady(message, item);
                saveSentNews(item);

                if (m_throttleMs > 0) {
                    QThread::msleep(m_throttleMs);
                }
            }
        }
    });
}

bool NewsParser::isNewsSent(const NewsItem &item) {
    QSqlQuery query;
    query.prepare("SELECT 1 FROM sent_news WHERE hash = ?");
    query.addBindValue(item.hash);
    return query.exec() && query.next();
}

void NewsParser::saveSentNews(const NewsItem &item) {
    QSqlQuery query;
    query.prepare("INSERT INTO sent_news (id, title, url, publish_date, source, hash) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(item.id);
    query.addBindValue(item.title);
    query.addBindValue(item.url);
    query.addBindValue(item.publishDate);
    query.addBindValue(item.source);
    query.addBindValue(item.hash);
    query.exec();
}
