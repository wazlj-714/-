// skinstrategy.cpp：提供可替换的配色方案，切换皮肤不改变游戏数据。
#include "skinstrategy.h"
// 具体皮肤只在本文件可见，外部通过工厂与抽象接口使用。
namespace {
// 清新配色。
class FreshSkin final : public SkinStrategy {
public:
    QColor surface() const override { return QColor("#E8F5F2"); }
    QColor tile() const override { return QColor("#F8FDFB"); }
    QColor grid() const override { return QColor("#CDE8E1"); }
    QColor snake(int player) const override { return player == 1 ? QColor("#4DAE9C") : QColor("#E4A35F"); }
    QColor head(int player) const override { return player == 1 ? QColor("#258E7C") : QColor("#CC8240"); }
    QColor food() const override { return QColor("#F17278"); }
};
// 复古配色。
class RetroSkin final : public SkinStrategy {
public:
    QColor surface() const override { return QColor("#ECE7D6"); }
    QColor tile() const override { return QColor("#F6F0DF"); }
    QColor grid() const override { return QColor("#D5CAB2"); }
    QColor snake(int player) const override { return player == 1 ? QColor("#66824A") : QColor("#C29A55"); }
    QColor head(int player) const override { return player == 1 ? QColor("#3B6142") : QColor("#986A35"); }
    QColor food() const override { return QColor("#D96855"); }
};
// 深色配色。
class AuroraSkin final : public SkinStrategy {
public:
    QColor surface() const override { return QColor("#17263A"); }
    QColor tile() const override { return QColor("#1C3147"); }
    QColor grid() const override { return QColor("#304960"); }
    QColor snake(int player) const override { return player == 1 ? QColor("#67C8D9") : QColor("#FFB66D"); }
    QColor head(int player) const override { return player == 1 ? QColor("#4BE1D7") : QColor("#F59D5E"); }
    QColor food() const override { return QColor("#FF7E93"); }
};
}
// 按界面选项创建皮肤，其他索引使用默认方案。
std::unique_ptr<SkinStrategy> makeSkin(int index) {
    if (index == 1) return std::unique_ptr<SkinStrategy>(new RetroSkin());
    if (index == 2) return std::unique_ptr<SkinStrategy>(new AuroraSkin());
    return std::unique_ptr<SkinStrategy>(new FreshSkin());
}
