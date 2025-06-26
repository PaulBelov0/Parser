#include "news_parser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QtConcurrent>

NewsParser::NewsParser(const QString &configPath, QObject *parent)
    : QObject(parent), m_configPath(configPath)
{
    m_bot = nullptr;
    m_downloader = new PageDownloader(this);
    m_updateTimer = new QTimer(this);

    connect(m_downloader, &PageDownloader::downloaded,
            this, &NewsParser::processNewsContent);
    connect(m_downloader, &PageDownloader::error,
            this, &NewsParser::handleDownloadError);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        for (auto it = m_updateIntervals.constBegin(); it != m_updateIntervals.constEnd(); ++it) {
            m_downloader->download(it.key());
        }
    });

    m_threadPool.setMaxThreadCount(QThread::idealThreadCount() > 2 ?
                                       QThread::idealThreadCount() / 2 : 1);

    loadConfig();
    initDatabase();
}

NewsParser::~NewsParser()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

void NewsParser::setThrottle(int ms)
{
    m_throttleMs = ms;
}

void NewsParser::start()
{
    if (!m_updateIntervals.isEmpty()) {
        m_updateTimer->start(5 * 60 * 1000); // 5 минут
        QTimer::singleShot(0, this, [this]() {
            for (auto it = m_updateIntervals.constBegin(); it != m_updateIntervals.constEnd(); ++it) {
                m_downloader->download(it.key());
            }
        });
    }
}

void NewsParser::stop()
{
    m_updateTimer->stop();
}

void NewsParser::loadConfig()
{
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open config file: %1").arg(m_configPath));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    QJsonObject config = doc.object();

    m_bot = new TelegramBot(config["telegram_token"].toString(), this);
    m_dbPath = config["database"].toString("news_bot.db");
    m_parserType = config["parser_type"].toString("simple");

    QJsonArray sources = config["news_sources"].toArray();
    for (const QJsonValue &source : sources) {
        QUrl url(source.toObject()["url"].toString());
        int interval = source.toObject()["update_interval"].toInt(3600);
        m_updateIntervals.insert(url, interval);
    }
}

void NewsParser::initDatabase()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_dbPath);

    if (!m_database.open()) {
        emit errorOccurred(tr("Database error: %1").arg(m_database.lastError().text()));
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS sent_news ("
               "id TEXT, "
               "title TEXT, "
               "url TEXT PRIMARY KEY, "
               "publish_date DATETIME, "
               "source TEXT, "
               "hash TEXT UNIQUE)");
}

void NewsParser::processNewsContent(const QByteArray &content, const QUrl &url)
{
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

bool NewsParser::isNewsSent(const NewsItem &item)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM sent_news WHERE hash = ?");
    query.addBindValue(item.hash);
    return query.exec() && query.next();
}

void NewsParser::saveSentNews(const NewsItem &item)
{
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO sent_news "
                  "(id, title, url, publish_date, source, hash) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(item.id);
    query.addBindValue(item.title);
    query.addBindValue(item.url);
    query.addBindValue(item.publishDate);
    query.addBindValue(item.source);
    query.addBindValue(item.hash);

    if (!query.exec()) {
        emit errorOccurred(tr("Database error: %1").arg(query.lastError().text()));
    }
}

QList<NewsItem> NewsParser::parseNews(const QByteArray &content, const QString &parserType)
{
    QList<NewsItem> items;

    if (parserType == "simple") {
        QRegularExpression re(
            "<item>.*?<title>(.*?)</title>.*?"
            "<description>(.*?)</description>.*?"
            "<link>(.*?)</link>.*?"
            "<pubDate>(.*?)</pubDate>.*?</item>",
            QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatchIterator i = re.globalMatch(content);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            NewsItem item;
            item.title = match.captured(1).trimmed();
            item.description = match.captured(2).trimmed();
            item.url = match.captured(3).trimmed();
            item.publishDate = QDateTime::fromString(match.captured(4), Qt::ISODate);
            items.append(item);
        }
    }
    // Добавьте другие парсеры по необходимости

    return items;
}

QString NewsParser::formatNewsMessage(const NewsItem &item) const
{
    return QString("<b>%1</b>\n\n%2\n\n<a href=\"%3\">Читать далее</a>\n\n%4")
        .arg(item.title.left(100),
             item.description.left(200) + (item.description.length() > 200 ? "..." : ""),
             item.url,
             item.publishDate.toString("dd.MM.yyyy hh:mm"));
}

void NewsParser::handleDownloadError(const QString &error, const QUrl &url)
{
    emit errorOccurred(tr("Failed to download %1: %2").arg(url.toString(), error));
}
