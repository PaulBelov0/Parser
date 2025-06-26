#-------------------------------------------------
# Базовые настройки проекта
#-------------------------------------------------
TEMPLATE = app
TARGET = Parser
QT += core network sql concurrent
CONFIG += c++17 #warn_on release
# CONFIG -= qt debug_and_release

#-------------------------------------------------
# Списки файлов
#-------------------------------------------------
SOURCES += \
    main.cpp \
    bot_instance.cpp \
    resource_monitor.cpp \
    news_item.cpp \
    page_downloader.cpp \
    telegram_bot.cpp \
    news_parser.cpp

HEADERS += \
    bot_instance.h \
    resource_monitor.h \
    news_item.h \
    page_downloader.h \
    telegram_bot.h \
    news_parser.h

# #-------------------------------------------------
# # Настройки для Linux/Raspberry Pi
# #-------------------------------------------------
# linux {
#     # Общие флаги оптимизации
#     QMAKE_CXXFLAGS += -ffunction-sections -fdata-sections
#     QMAKE_LFLAGS += -Wl,--gc-sections

#     # Настройки для SQLite
#     DEFINES += SQLITE_DEFAULT_MEMSTATUS=0 \
#                SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
# }

# #-------------------------------------------------
# # Оптимизации для релиза
# #-------------------------------------------------
# release {
#     QMAKE_CXXFLAGS += -O2
#     DEFINES += QT_NO_DEBUG_OUTPUT
# }

#-------------------------------------------------
# Установка
#-------------------------------------------------
target.path = $$PWD/bin
INSTALLS += target

#-------------------------------------------------
# Дополнительные настройки
#-------------------------------------------------
# Отключаем ненужные функции Qt
# DEFINES += QT_NO_CAST_FROM_ASCII \
#            QT_NO_CAST_TO_ASCII

# Линковка с математической библиотекой
unix:!macx: LIBS += -lm
