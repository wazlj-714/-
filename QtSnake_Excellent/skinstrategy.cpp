// skinstrategy.cpp - Three interchangeable implementations, no rule changes.
#include "skinstrategy.h"
namespace {
class FreshSkin final : public SkinStrategy {
public:
    QColor surface() const override { return QColor("#E8F5F2"); }
    QColor tile() const override { return QColor("#F8FDFB"); }
    QColor grid() const override { return QColor("#CDE8E1"); }
    QColor snake(int player) const override { return player == 1 ? QColor("#4DAE9C") : QColor("#E4A35F"); }
    QColor head(int player) const override { return player == 1 ? QColor("#258E7C") : QColor("#CC8240"); }
    QColor food() const override { return QColor("#F17278"); }
};
class RetroSkin final : public SkinStrategy {
public:
    QColor surface() const override { return QColor("#ECE7D6"); }
    QColor tile() const override { return QColor("#F6F0DF"); }
    QColor grid() const override { return QColor("#D5CAB2"); }
    QColor snake(int player) const override { return player == 1 ? QColor("#66824A") : QColor("#C29A55"); }
    QColor head(int player) const override { return player == 1 ? QColor("#3B6142") : QColor("#986A35"); }
    QColor food() const override { return QColor("#D96855"); }
};
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
std::unique_ptr<SkinStrategy> makeSkin(int index) {
    if (index == 1) return std::unique_ptr<SkinStrategy>(new RetroSkin());
    if (index == 2) return std::unique_ptr<SkinStrategy>(new AuroraSkin());
    return std::unique_ptr<SkinStrategy>(new FreshSkin());
}
