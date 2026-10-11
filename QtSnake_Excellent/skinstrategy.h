// skinstrategy.h - Strategy interface for GUI-only theme changes.
#ifndef SNAKE_SKINSTRATEGY_H
#define SNAKE_SKINSTRATEGY_H
#include <QColor>
#include <memory>
class SkinStrategy {
public:
    virtual ~SkinStrategy() = default;
    virtual QColor surface() const = 0;
    virtual QColor tile() const = 0;
    virtual QColor grid() const = 0;
    virtual QColor snake(int player) const = 0;
    virtual QColor head(int player) const = 0;
    virtual QColor food() const = 0;
};
std::unique_ptr<SkinStrategy> makeSkin(int index);
#endif
