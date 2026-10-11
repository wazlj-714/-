// skinstrategy.h：界面皮肤策略，只提供绘制颜色，不参与游戏规则。
#ifndef SNAKE_SKINSTRATEGY_H
#define SNAKE_SKINSTRATEGY_H
#include <QColor>
#include <memory>
// 各皮肤通过相同接口提供配色，颜色类型仅出现在界面层。
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
// 工厂返回拥有皮肤对象的智能指针，由窗口接管其生命周期。
std::unique_ptr<SkinStrategy> makeSkin(int index);
#endif
