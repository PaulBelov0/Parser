#include "news_item.h"
#include <QCryptographicHash>
#include <QRegularExpression>

bool NewsItem::isValid() const {
    return !url.isEmpty() && !title.isEmpty() && publishDate.isValid();
}

bool NewsItem::operator==(const NewsItem &other) const {
    return hash == other.hash;
}

NewsItem NewsItem::fromJson(const QJsonObject &json) {
    NewsItem item;

    // Базовые поля
    item.id = json["id"].toString();
    item.title = json["title"].toString().trimmed();
    item.description = json["description"].toString().trimmed();
    item.url = json["url"].toString().trimmed();
    item.source = json["source"].toString();

    // Парсинг даты (поддержка нескольких форматов)
    QString dateStr = json["publishedAt"].toString();
    if (!dateStr.isEmpty()) {
        item.publishDate = QDateTime::fromString(dateStr, Qt::ISODate);
        if (!item.publishDate.isValid()) {
            item.publishDate = QDateTime::fromString(dateStr, "ddd, dd MMM yyyy HH:mm:ss GMT");
        }
    }

    // Генерация уникального хеша
    QString uniqueStr = item.url.isEmpty() ? item.id : item.url;
    item.hash = QCryptographicHash::hash(
                    (uniqueStr + item.title).toUtf8(),
                    QCryptographicHash::Md5).toHex();

    // Очистка от HTML-тегов
    static QRegularExpression htmlTags("<[^>]*>");
    item.description.remove(htmlTags);

    return item;
}

QJsonObject NewsItem::toJson() const {
    return {
        {"id", id},
        {"title", title},
        {"description", description},
        {"url", url},
        {"source", source},
        {"publishedAt", publishDate.toString(Qt::ISODate)},
        {"hash", hash}
    };
}

QDebug operator<<(QDebug debug, const NewsItem &item) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "NewsItem("
                    << "title=" << item.title.left(20) << "..."
                    << ", url=" << item.url
                    << ", date=" << item.publishDate.toString("dd.MM.yy hh:mm")
                    << ")";
    return debug;
}

QDataStream &operator<<(QDataStream &out, const NewsItem &item) {
    out << item.id
        << item.title
        << item.description
        << item.url
        << item.source
        << item.publishDate
        << item.hash;
    return out;
}

QDataStream &operator>>(QDataStream &in, NewsItem &item) {
    in >> item.id
        >> item.title
        >> item.description
        >> item.url
        >> item.source
        >> item.publishDate
        >> item.hash;
    return in;
}
