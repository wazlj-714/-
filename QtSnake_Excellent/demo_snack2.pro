# qmake project - compatible with Qt 5.15+ and Qt 6.x Widgets.
QT += core gui widgets
CONFIG += c++17 warn_on
TEMPLATE = app
TARGET = demo_snack2

SOURCES += \
    main.cpp \
    widget.cpp \
    skinstrategy.cpp \
    core/snake.cpp \
    core/food.cpp \
    core/leaderboard.cpp \
    core/gamestate.cpp \
    core/game.cpp

HEADERS += \
    widget.h \
    skinstrategy.h \
    core/types.h \
    core/snake.h \
    core/food.h \
    core/leaderboard.h \
    core/gamestate.h \
    core/game.h

FORMS += widget.ui
