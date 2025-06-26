#ifndef PAGE_DOWNLOADER_H
#define PAGE_DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>

class PageDownloader : public QObject {
    Q_OBJECT
public:
    explicit PageDownloader(QObject *parent = nullptr);
    void download(const QUrl &url);
    void setUserAgent(const QString &userAgent);

signals:
    void downloaded(const QByteArray &content, const QUrl &url);
    void error(const QString &errorString, const QUrl &url);

private:
    QNetworkAccessManager *m_manager;
    QString m_userAgent;
};

#endif // PAGE_DOWNLOADER_H
