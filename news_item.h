#ifndef NEWS_ITEM_H
#define NEWS_ITEM_H

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QJsonObject>
#include <QDebug>
#include <QDataStream>

struct NewsItem {
    // Основные поля
    QString id;
    QString title;
    QString description;
    QString url;
    QString source;
    QDateTime publishDate;
    QString hash;

    // Методы
    bool isValid() const;
    bool operator==(const NewsItem &other) const;

    // Фабричные методы
    static NewsItem fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
};

// Для использования в QVariant/QSettings
Q_DECLARE_METATYPE(NewsItem)

// Операторы сериализации
QDataStream &operator<<(QDataStream &out, const NewsItem &item);
QDataStream &operator>>(QDataStream &in, NewsItem &item);

// Оператор для отладки
QDebug operator<<(QDebug debug, const NewsItem &item);

#endif // NEWS_ITEM_H
