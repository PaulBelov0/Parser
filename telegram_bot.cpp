#include "telegram_bot.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

TelegramBot::TelegramBot(const QString &token, QObject *parent)
    : QObject(parent), m_token(token) {
    m_manager = new QNetworkAccessManager(this);
    m_manager->setCache(new QNetworkDiskCache(this));
    m_manager->cache()->setCacheDirectory("/tmp/telegram_bot_cache");
}

QNetworkRequest TelegramBot::createRequest(const QString &method) const {
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(m_token, method));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setTransferTimeout(10000); // 10 секунд таймаут
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

    QNetworkReply *reply = m_manager->get(createRequest("getChatAdministrators")
                                              .withRawHeader("chat_id", QByteArray::number(chatId)));
