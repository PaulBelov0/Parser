#include "page_downloader.h"
#include <QNetworkRequest>
#include <QNetworkReply>

PageDownloader::PageDownloader(QObject *parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
    m_userAgent = "Mozilla/5.0 (X11; Linux armv8l) NewsParserBot/1.0";
}

void PageDownloader::download(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::PreferCache);
    request.setMaximumRedirectsAllowed(3);

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, url]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit downloaded(reply->readAll(), url);
        } else {
            emit error(reply->errorString(), url);
        }
        reply->deleteLater();
    });
}

void PageDownloader::setUserAgent(const QString &userAgent) {
    m_userAgent = userAgent;
}
