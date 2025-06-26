#ifndef TELEGRAM_BOT_H
#define TELEGRAM_BOT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

class TelegramBot : public QObject {

    Q_OBJECT

public:
    explicit TelegramBot(const QString &token, QObject *parent = nullptr);

    void sendMessage(qint64 chatId, const QString &text, bool disableWebPagePreview = true);
    void editMessage(qint64 chatId, qint64 messageId, const QString &newText);
    void checkAdminRights(qint64 chatId);

signals:
    void messageSent(qint64 chatId, qint64 messageId);
    void adminCheckComplete(qint64 chatId, bool isAdmin);
    void errorOccurred(const QString &error);

private:
    QNetworkRequest createRequest(const QString &method) const;
    void handleNetworkError(QNetworkReply *reply);

private:
    QString m_token;
    QNetworkAccessManager *m_manager;
    QNetworkDiskCache *m_cache;
    QMap<qint64, bool> m_adminCache;

};

#endif // TELEGRAM_BOT_H
