#include "telegram_bot.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

TelegramBot::TelegramBot(const QString &token, QObject *parent)
    : QObject(parent), m_token(token)
{
    m_manager = new QNetworkAccessManager(this);

    // Инициализация кэша
    QNetworkDiskCache *cache = new QNetworkDiskCache(this);
    QString cachePath = QDir::temp().filePath("telegram_bot_cache");
    cache->setCacheDirectory(cachePath);
    m_manager->setCache(cache);
}

QNetworkRequest TelegramBot::createRequest(const QString &method) const {
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(m_token, method));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
    request.setTransferTimeout(10000);
    return request;
}

void TelegramBot::sendMessage(qint64 chatId, const QString &text, bool disableWebPagePreview) {
    QUrlQuery query;
    query.addQueryItem("chat_id", QString::number(chatId));
    query.addQueryItem("text", text);
    query.addQueryItem("parse_mode", "HTML");
    query.addQueryItem("disable_web_page_preview", disableWebPagePreview ? "true" : "false");

    QNetworkReply *reply = m_manager->post(createRequest("sendMessage"),
                                           query.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, [this, reply, chatId]() {
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(reply);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        qint64 messageId = doc.object()["result"].toObject()["message_id"].toInt();
        emit messageSent(chatId, messageId);
        reply->deleteLater();
    });
}

void TelegramBot::checkAdminRights(qint64 chatId) {
    if (m_adminCache.contains(chatId)) {
        emit adminCheckComplete(chatId, m_adminCache[chatId]);
        return;
    }

    QNetworkRequest request = createRequest("getChatAdministrators");
    QUrlQuery query;
    query.addQueryItem("chat_id", QString::number(chatId));

    QNetworkReply *reply = m_manager->post(request, query.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, [this, reply, chatId]() {
        bool isAdmin = false;
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray admins = doc.object()["result"].toArray();
            QString botId = m_token.split(':').first();

            for (const QJsonValue &admin : admins) {
                if (admin.toObject()["user"].toObject()["id"].toString() == botId) {
                    isAdmin = true;
                    break;
                }
            }
            m_adminCache[chatId] = isAdmin;
        }
        emit adminCheckComplete(chatId, isAdmin);
        reply->deleteLater();
    });
}

void TelegramBot::handleNetworkError(QNetworkReply *reply) {
    QString errorStr = reply->errorString();
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        errorStr = "Request timeout";
    }
    emit errorOccurred(errorStr);
    reply->deleteLater();
}
