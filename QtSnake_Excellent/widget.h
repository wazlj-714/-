// widget.h - Qt display + event adapter for the independent game core.
#ifndef WIDGET_H
#define WIDGET_H
#include <QWidget>
#include <QTimer>
#include <QString>
#include <memory>
#include "core/game.h"
#include "skinstrategy.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
class QComboBox;
class QPushButton;
class QPainter;
class QKeyEvent;
class QPaintEvent;

class Widget final : public QWidget {
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private:
    static constexpr int CELL = 24;
    static constexpr int BOARD_X = 32;
    static constexpr int BOARD_Y = 112;
    static constexpr int SIDE_X = 542;
    static constexpr int WINDOW_W = 968;
    static constexpr int WINDOW_H = 684;

    Ui::Widget *ui_;
    Game game_;
    QTimer timer_;
    std::unique_ptr<SkinStrategy> skin_;
    QComboBox *difficultyBox_ = nullptr;
    QComboBox *skinBox_ = nullptr;
    QComboBox *slotBox_ = nullptr;
    QPushButton *singleButton_ = nullptr;
    QPushButton *dualButton_ = nullptr;
    QPushButton *pauseButton_ = nullptr;
    QPushButton *resumeButton_ = nullptr;
    QPushButton *menuButton_ = nullptr;
    QPushButton *saveButton_ = nullptr;
    QPushButton *loadButton_ = nullptr;
    QPushButton *exitButton_ = nullptr;
    QString dataDir_;

    void buildControls();
    void refreshControls();
    void startGame(GameMode mode);
    void pauseGame();
    void resumeGame();
    void backToMenu();
    void saveSlot();
    void loadSlot();
    void onTick();
    QString slotPath() const;
    void drawScene(QPainter &p, Scene scene);
    void drawBoard(QPainter &p, bool showPlay);
    void drawSnake(QPainter &p, const Snake &snake, int player);
    void drawRightPanel(QPainter &p, Scene scene);
    void drawHeader(QPainter &p, Scene scene);
    void drawOverlay(QPainter &p, const QString &title, const QString &detail);
};
#endif
