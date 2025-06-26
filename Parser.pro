#-------------------------------------------------
# Базовые настройки проекта
#-------------------------------------------------
TEMPLATE = app
TARGET = multi_bot
QT += core network sql concurrent
CONFIG += c++17 warn_on release link_pkgconfig
CONFIG -= qt debug_and_release

#-------------------------------------------------
# Списки файлов
#-------------------------------------------------
SOURCES += \
    src/main.cpp \
    src/bot_instance.cpp \
    src/resource_monitor.cpp \
    src/news_item.cpp \
    src/page_downloader.cpp \
    src/telegram_bot.cpp \
    src/news_parser.cpp

HEADERS += \
    include/bot_instance.h \
    include/resource_monitor.h \
    include/news_item.h \
    include/page_downloader.h \
    include/telegram_bot.h \
    include/news_parser.h

#-------------------------------------------------
# Настройки для Raspberry Pi 5
#-------------------------------------------------
raspberry_pi {
    # Аппаратно-специфичные оптимизации
    QMAKE_CXXFLAGS += -march=armv8-a+crc+crypto -mtune=cortex-a76 -mfpu=neon-fp-armv8
    QMAKE_CFLAGS += -march=armv8-a+crc+crypto -mtune=cortex-a76 -mfpu=neon-fp-armv8

    # Оптимизации для Qt
    DEFINES += QT_NO_DEBUG_OUTPUT \
               QT_NO_FOREACH \
               QT_STRICT_ITERATORS \
               RASPBERRY_PI_OPTIMIZED

    # Настройки для уменьшения потребления памяти
    QMAKE_LFLAGS += -Wl,--as-needed,--gc-sections
    QMAKE_CXXFLAGS_RELEASE += -ffunction-sections -fdata-sections -Os
}

#-------------------------------------------------
# Общие оптимизации
#-------------------------------------------------
linux {
    # Кэширование DNS
    DEFINES += QT_NETWORK_DISK_CACHE=1024

    # Настройки для SQLite
    PKGCONFIG += sqlite3
    DEFINES += SQLITE_DEFAULT_MEMSTATUS=0 \
               SQLITE_DEFAULT_WAL_SYNCHRONOUS=1 \
               SQLITE_MAX_MMAP_SIZE=268435456
}

#-------------------------------------------------
# Установка и деплой
#-------------------------------------------------
target.path = /usr/local/bin

config.path = /etc/multi_bot
config.files = configs/*.json

service.path = /etc/systemd/system
service.files = systemd/multi_bot.service

INSTALLS += target config service

#-------------------------------------------------
# Пост-сборочные шаги
#-------------------------------------------------
# Генерация хешей для новостей при сборке
system_commands.commands = $$system(openssl version)
system_commands.target = $$OUT_PWD/.system_checks
PRE_TARGETDEPS += $$system_commands.target

#-------------------------------------------------
# Дополнительные настройки
#-------------------------------------------------
# Отключаем ненужные функции Qt
DEFINES += QT_NO_CAST_FROM_ASCII \
           QT_NO_CAST_TO_ASCII \
           QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

# Оптимизация работы со строками
QMAKE_CXXFLAGS += -fshort-wchar

# Линковка с математической библиотекой
unix:!macx: LIBS += -lm

#-------------------------------------------------
# Настройки для релизной сборки
#-------------------------------------------------
release {
    # Оптимизация по размеру
    QMAKE_CXXFLAGS += -Os
    # LTO (если поддерживается)
    equals(QMAKE_LFLAGS_LTO, ""): QMAKE_LFLAGS += -flto=auto
}
