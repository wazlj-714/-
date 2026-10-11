// widget.cpp：根据只读业务数据绘制画面，游戏规则由 core/game.cpp 处理。
#include "widget.h"
#include "ui_widget.h"
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QStandardPaths>
#include <QVariant>
#include <algorithm>

namespace {
constexpr int BUTTON_W = 176;
constexpr int BUTTON_H = 36;
QColor INK("#213748");
QColor MUTED("#687B86");
QColor ACCENT("#2AA88F");
// 将业务枚举转为界面文字，业务核心仍使用标准 C++ 类型。
QString labelDifficulty(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Easy: return QStringLiteral("简单");
    case Difficulty::Hard: return QStringLiteral("困难");
    default: return QStringLiteral("普通");
    }
}
QString labelEnding(EndReason reason) {
    switch (reason) {
    case EndReason::Wall: return QStringLiteral("撞到了边界");
    case EndReason::Body: return QStringLiteral("碰撞发生");
    case EndReason::HeadOn: return QStringLiteral("双方蛇头相撞");
    case EndReason::BoardFull: return QStringLiteral("填满棋盘，挑战完成！");
    default: return QStringLiteral("本局结束");
    }
}
QString labelMode(GameMode mode) {
    return mode == GameMode::Single ? QStringLiteral("单人") : QStringLiteral("双人");
}
}
// 初始化窗口、数据目录、控件、变化通知和定时调度。
Widget::Widget(QWidget *parent)
    : QWidget(parent), ui_(new Ui::Widget), game_(20, 20), skin_(makeSkin(0)) {
    ui_->setupUi(this);
    setWindowTitle(QStringLiteral("贪吃蛇 · 面向对象综合实训"));
    setFixedSize(WINDOW_W, WINDOW_H);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_StyledBackground, true);
    dataDir_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir_);
    std::string error;
    const QString rankingPath = QDir(dataDir_).filePath(QStringLiteral("top10.txt"));
    if (!game_.loadLeaderboard(rankingPath.toLocal8Bit().toStdString(), &error))
        QMessageBox::warning(this, QStringLiteral("排行榜读取失败"), QString::fromStdString(error));

    buildControls();
    // update 只请求重绘，实际绘图由 Qt 后续调用 paintEvent。
    game_.setChangeObserver([this] { update(); });
    timer_.setTimerType(Qt::PreciseTimer);
    connect(&timer_, &QTimer::timeout, this, &Widget::onTick);
    refreshControls();
}
// 退出时尝试保存排行榜；目前未向界面反馈保存失败。
Widget::~Widget() {
    game_.saveLeaderboard();
    delete ui_;
}
// 动态创建控件并连接信号槽，按钮操作统一调用业务接口。
void Widget::buildControls() {
    const QString comboStyle = QStringLiteral(
        "QComboBox{background:#F7FBFA;color:#24394A;border:1px solid #CADCD9;"
        "border-radius:9px;padding:6px 12px;font-size:13px;}"
        "QComboBox::drop-down{border:0;width:18px;}"
        "QComboBox QAbstractItemView{background:white;color:#24394A;}");
    auto newCombo = [&](int x, int y, int width) -> QComboBox * {
        auto *box = new QComboBox(this);
        box->setGeometry(x, y, width, 35);
        box->setStyleSheet(comboStyle);
        box->setFocusPolicy(Qt::NoFocus);
        return box;
    };
    difficultyBox_ = newCombo(566, 185, 176);
    difficultyBox_->addItems({QStringLiteral("简单 / 慢速"),
                              QStringLiteral("普通 / 标准"), QStringLiteral("困难 / 快速")});
    difficultyBox_->setCurrentIndex(1);
    skinBox_ = newCombo(754, 185, 178);
    skinBox_->addItems({QStringLiteral("薄荷清新"), QStringLiteral("复古纸张"),
                        QStringLiteral("极光之夜")});
    connect(skinBox_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int index) { skin_ = makeSkin(index); update(); });
    slotBox_ = newCombo(566, 337, 176);
    slotBox_->addItems({QStringLiteral("存档位 1"), QStringLiteral("存档位 2"),
                        QStringLiteral("存档位 3")});

    auto newButton = [&](const QString &title, int x, int y, bool primary) -> QPushButton * {
        auto *button = new QPushButton(title, this);
        button->setGeometry(x, y, BUTTON_W, BUTTON_H);
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus); // 避免控件占用键盘焦点，让方向键交给主窗口处理。
        button->setStyleSheet(primary ? QStringLiteral(
            "QPushButton{color:white;background:#269E8A;border:0;border-radius:9px;"
            "font-size:13px;font-weight:bold;}"
            "QPushButton:hover{background:#218B79;}QPushButton:pressed{background:#187866;}")
            : QStringLiteral(
            "QPushButton{color:#2A5361;background:#E9F2F0;border:1px solid #C9DEDA;"
            "border-radius:9px;font-size:13px;font-weight:bold;}"
            "QPushButton:hover{background:#D5E9E5;}QPushButton:pressed{background:#C5E0D9;}"));
        return button;
    };
    singleButton_ = newButton(QStringLiteral("开始单人游戏  [1]"), 566, 244, true);
    dualButton_   = newButton(QStringLiteral("开始双人游戏  [2]"), 754, 244, true);
    pauseButton_  = newButton(QStringLiteral("暂停游戏  [P]"), 566, 244, false);
    resumeButton_ = newButton(QStringLiteral("继续游戏  [C]"), 566, 244, true);
    menuButton_   = newButton(QStringLiteral("返回主菜单  [Esc]"), 754, 244, false);
    saveButton_   = newButton(QStringLiteral("保存当前进度"), 566, 290, false);
    loadButton_   = newButton(QStringLiteral("读取选中存档"), 754, 290, false);
    exitButton_   = newButton(QStringLiteral("退出程序"), 754, 337, false);

    connect(singleButton_, &QPushButton::clicked, this, [this] { startGame(GameMode::Single); });
    connect(dualButton_, &QPushButton::clicked, this, [this] { startGame(GameMode::Dual); });
    connect(pauseButton_, &QPushButton::clicked, this, &Widget::pauseGame);
    connect(resumeButton_, &QPushButton::clicked, this, &Widget::resumeGame);
    connect(menuButton_, &QPushButton::clicked, this, &Widget::backToMenu);
    connect(saveButton_, &QPushButton::clicked, this, &Widget::saveSlot);
    connect(loadButton_, &QPushButton::clicked, this, &Widget::loadSlot);
    connect(exitButton_, &QPushButton::clicked, this, &QWidget::close);
}
// 按场景与暂停标记调整按钮显示，非运行场景停止定时器。
void Widget::refreshControls() {
    const bool playing = game_.scene() == Scene::Playing;
    const bool stopped = playing && game_.paused();
    singleButton_->setVisible(!playing);
    dualButton_->setVisible(!playing);
    pauseButton_->setVisible(playing && !stopped);
    resumeButton_->setVisible(stopped);
    menuButton_->setVisible(playing);
    saveButton_->setVisible(playing);
    loadButton_->setVisible(true);
    difficultyBox_->setEnabled(!playing);
    if (!playing) timer_.stop();
    update();
}
void Widget::startGame(GameMode mode) {
    game_.start(mode, static_cast<Difficulty>(difficultyBox_->currentIndex()));
    timer_.start(game_.stepIntervalMs());
    refreshControls();
    setFocus();
}
// 同时设置业务暂停标记并停止定时器，保持操作与显示一致。
void Widget::pauseGame() {
    game_.pause();
    timer_.stop();
    refreshControls();
    setFocus();
}
void Widget::resumeGame() {
    game_.resume();
    if (game_.scene() == Scene::Playing) timer_.start(game_.stepIntervalMs());
    refreshControls();
    setFocus();
}
void Widget::backToMenu() {
    timer_.stop();
    game_.backToMenu();
    refreshControls();
    setFocus();
}
// 选中存档位对应一个独立文件，统一放入应用数据目录。
QString Widget::slotPath() const {
    return QDir(dataDir_).filePath(QStringLiteral("slot_%1.sav").arg(slotBox_->currentIndex() + 1));
}
// 保存前暂停游戏；保存结束后由玩家主动继续。
void Widget::saveSlot() {
    std::string error;
    timer_.stop();
    game_.pause();
    if (!game_.saveSnapshot(slotPath().toLocal8Bit().toStdString(), &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), QString::fromStdString(error));
    } else {
        QMessageBox::information(this, QStringLiteral("已保存"),
                                 QStringLiteral("游戏已保存到存档位 %1，按 C 继续。")
                                     .arg(slotBox_->currentIndex() + 1));
    }
    refreshControls();
    setFocus();
}
// 读档前停止定时器，成功后保持暂停。
// 当前失败分支没有恢复定时器，运行中读档失败可能造成画面停止。
void Widget::loadSlot() {
    timer_.stop();
    std::string error;
    if (!game_.loadSnapshot(slotPath().toLocal8Bit().toStdString(), &error)) {
        QMessageBox::warning(this, QStringLiteral("读取失败"), QString::fromStdString(error));
    } else {
        difficultyBox_->setCurrentIndex(static_cast<int>(game_.difficulty()));
        QMessageBox::information(this, QStringLiteral("读取成功"),
                                 QStringLiteral("已读取存档，按 C 或点击「继续游戏」恢复。"));
    }
    refreshControls();
    setFocus();
}
// 推进回合后更新移动间隔和控件，碰撞与加分由 Game 处理。
void Widget::onTick() {
    game_.tick();
    if (game_.scene() == Scene::Playing) {
        const int interval = game_.stepIntervalMs();
        if (timer_.interval() != interval) timer_.setInterval(interval);
    }
    refreshControls();
}
// 将按键转为操作请求，转向合法性仍由业务层判定。
void Widget::keyPressEvent(QKeyEvent *event) {
    const int key = event->key();
    if (key == Qt::Key_Escape) { backToMenu(); return; }
    if (game_.scene() != Scene::Playing) {
        if (key == Qt::Key_1 || key == Qt::Key_Return || key == Qt::Key_Enter)
            startGame(GameMode::Single);
        else if (key == Qt::Key_2) startGame(GameMode::Dual);
        else if (key == Qt::Key_R && game_.scene() == Scene::GameOver)
            startGame(game_.mode());
        else QWidget::keyPressEvent(event);
        return;
    }
    if (key == Qt::Key_P || key == Qt::Key_Space) { pauseGame(); return; }
    if (key == Qt::Key_C) { resumeGame(); return; }
    if (key == Qt::Key_S && (event->modifiers() & Qt::ControlModifier)) { saveSlot(); return; }
    if (key == Qt::Key_L && (event->modifiers() & Qt::ControlModifier)) { loadSlot(); return; }

    // 单人方向键控制玩家一；双人方向键控制玩家二，WASD 控制玩家一。
    int player = game_.mode() == GameMode::Single ? 1 : 2;
    Direction direction;
    bool matched = true;
    switch (key) {
    case Qt::Key_Up: direction = Direction::Up; break;
    case Qt::Key_Down: direction = Direction::Down; break;
    case Qt::Key_Left: direction = Direction::Left; break;
    case Qt::Key_Right: direction = Direction::Right; break;
    default: matched = false; break;
    }
    if (matched) { game_.turn(player, direction); return; }
    if (game_.mode() == GameMode::Dual) {
        player = 1;
        switch (key) {
        case Qt::Key_W: direction = Direction::Up; break;
        case Qt::Key_S: direction = Direction::Down; break;
        case Qt::Key_A: direction = Direction::Left; break;
        case Qt::Key_D: direction = Direction::Right; break;
        default: QWidget::keyPressEvent(event); return;
        }
        game_.turn(player, direction);
    } else QWidget::keyPressEvent(event);
}
// 画笔只在绘制事件中创建，通过只读查询显示当前业务状态。
void Widget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    // 业务层仅返回场景枚举，具体绘制由界面回调完成。
    game_.renderScene([&](Scene scene) { drawScene(p, scene); });
}
// 组合公共画面，并根据场景叠加菜单、结束或暂停提示。
void Widget::drawScene(QPainter &p, Scene scene) {
    p.fillRect(rect(), QColor("#F2F6F7"));
    drawHeader(p, scene);
    drawBoard(p, scene != Scene::Menu);
    drawRightPanel(p, scene);
    if (scene == Scene::Menu) {
        drawOverlay(p, QStringLiteral("SNAKE  LAB"),
                    QStringLiteral("选择单人或双人模式，开启你的像素冒险\n方向键移动 · P 暂停 · C 继续"));
    } else if (scene == Scene::GameOver) {
        QString status;
        if (game_.mode() == GameMode::Single) {
            status = labelEnding(game_.endReason()) +
                     QStringLiteral("   ·   本局得分 %1").arg(game_.score(1));
        } else {
            const QString winner = game_.winner() == Winner::Player1 ? QStringLiteral("玩家 1 胜利") :
                                   game_.winner() == Winner::Player2 ? QStringLiteral("玩家 2 胜利") :
                                   QStringLiteral("平局");
            status = winner + QStringLiteral("  ·  P1 %1 / P2 %2")
                                   .arg(game_.score(1)).arg(game_.score(2));
        }
        drawOverlay(p, QStringLiteral("GAME OVER"), status +
                    QStringLiteral("\n右侧按钮可再开一局，R 键重开当前模式"));
    } else if (game_.paused()) {
        drawOverlay(p, QStringLiteral("PAUSED"),
                    QStringLiteral("时间暂时停在这里\n按 C 或点击「继续游戏」恢复"));
    }
}
void Widget::drawHeader(QPainter &p, Scene scene) {
    Q_UNUSED(scene);
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0, QColor("#163F47"));
    gradient.setColorAt(1, QColor("#1E6164"));
    p.fillRect(QRect(0,0,width(),84), gradient);
    p.setPen(QColor("#FEFCF3"));
    QFont title(QStringLiteral("Microsoft YaHei"), 19, QFont::Bold);
    p.setFont(title);
    p.drawText(QRect(30, 13, 440, 34), Qt::AlignVCenter, QStringLiteral("贪吃蛇 · 双人竞技"));
    p.setPen(QColor("#BDE5DA"));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));
    p.drawText(QRect(32, 51, 600, 22), Qt::AlignVCenter,
               QStringLiteral("SNAKE LAB  /  QT WIDGETS  /  面向对象综合实训"));
    p.setPen(QColor("#FFFFFF"));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::DemiBold));
    p.drawText(QRect(680,24,250,32), Qt::AlignRight | Qt::AlignVCenter,
               game_.scene() == Scene::Menu ? QStringLiteral("准备开始") :
               game_.scene() == Scene::GameOver ? QStringLiteral("本局已结束") :
               game_.paused() ? QStringLiteral("暂停中") : QStringLiteral("正在游戏"));
}
// 将格点映射为像素，依次绘制底色、网格、食物和蛇。
void Widget::drawBoard(QPainter &p, bool showPlay) {
    const int w = game_.width() * CELL;
    const int h = game_.height() * CELL;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#D8E3E4"));
    p.drawRoundedRect(QRect(BOARD_X-4,BOARD_Y-4,w+8,h+8),15,15);
    p.setBrush(skin_->surface());
    p.drawRoundedRect(QRect(BOARD_X,BOARD_Y,w,h),11,11);
    p.save();
    p.setClipRect(QRect(BOARD_X,BOARD_Y,w,h));
    p.setPen(Qt::NoPen);
    for (int y = 0; y < game_.height(); ++y)
        for (int x = 0; x < game_.width(); ++x)
            if (((x + y) % 2) == 0) {
                p.setBrush(skin_->tile());
                p.drawRect(QRect(BOARD_X+x*CELL,BOARD_Y+y*CELL,CELL,CELL));
            }
    p.setPen(QPen(skin_->grid(), 0.5));
    for (int x = 0; x <= game_.width(); ++x) {
        const int px = BOARD_X+x*CELL;
        p.drawLine(px, BOARD_Y, px, BOARD_Y+h);
    }
    for (int y = 0; y <= game_.height(); ++y) {
        const int py = BOARD_Y+y*CELL;
        p.drawLine(BOARD_X,py,BOARD_X+w,py);
    }
    if (showPlay) {
        if (game_.food().present()) {
            Cell c = game_.food().position();
            const int fx = BOARD_X+c.x*CELL, fy = BOARD_Y+c.y*CELL;
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0,0,0,22));
            p.drawEllipse(QRect(fx+4,fy+6,CELL-7,CELL-7));
            p.setBrush(skin_->food());
            p.drawEllipse(QRect(fx+4,fy+3,CELL-8,CELL-8));
            p.setBrush(QColor(255,255,255,140));
            p.drawEllipse(QRect(fx+8,fy+6,5,5));
        }
        drawSnake(p, game_.snake(1), 1);
        if (game_.mode() == GameMode::Dual) drawSnake(p, game_.snake(2), 2);
    }
    p.restore();
    p.setPen(MUTED);
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));
    p.drawText(QRect(BOARD_X,608,w,36),Qt::AlignVCenter,
               showPlay ? QStringLiteral("P1 %1     P2 %2     ·     第 %3 帧")
                              .arg(game_.score(1)).arg(game_.score(2)).arg(game_.frames())
                        : QStringLiteral("20 × 20 棋盘  ·  经典规则与双人同时结算"));
}
// 从尾部向头部绘制，让蛇头最后显示。
void Widget::drawSnake(QPainter &p, const Snake &snake, int player) {
    const auto &segments = snake.body();
    for (std::size_t i = segments.size(); i-- > 0;) {
        const Cell c = segments[i];
        const QRect r(BOARD_X+c.x*CELL+2, BOARD_Y+c.y*CELL+2,CELL-4,CELL-4);
        p.setPen(Qt::NoPen);
        p.setBrush(i == 0 ? skin_->head(player) : skin_->snake(player));
        p.drawRoundedRect(r, i == 0 ? 8 : 5, i == 0 ? 8 : 5);
        if (i > 0) {
            p.setBrush(QColor(255,255,255,45));
            p.drawRoundedRect(QRect(r.left()+3,r.top()+2,r.width()-6,4),2,2);
        } else {
            // 眼睛位置显示已提交的移动方向，不提前显示候选方向。
            const Direction d = snake.direction();
            QPoint e1, e2;
            if (d == Direction::Left || d == Direction::Right) {
                const int x = d == Direction::Left ? r.left()+5 : r.right()-5;
                e1=QPoint(x,r.top()+6); e2=QPoint(x,r.bottom()-6);
            } else {
                const int y = d == Direction::Up ? r.top()+5 : r.bottom()-5;
                e1=QPoint(r.left()+6,y); e2=QPoint(r.right()-6,y);
            }
            p.setBrush(QColor("#FDFCF9"));
            p.drawEllipse(e1,2,2);
            p.drawEllipse(e2,2,2);
        }
    }
}
// 显示控制区与排行榜，分数排序已经由 Leaderboard 完成。
void Widget::drawRightPanel(QPainter &p, Scene scene) {
    const QRect box(SIDE_X, 101, 406, 550);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFFFFF"));
    p.drawRoundedRect(box,14,14);
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 12, QFont::Bold));
    p.setPen(INK);
    p.drawText(QRect(566,115,360,28), Qt::AlignVCenter,
               QStringLiteral("控制中心   /   CONTROL"));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    p.setPen(MUTED);
    p.drawText(QRect(566,148,176,28), Qt::AlignVCenter,QStringLiteral("游戏难度"));
    p.drawText(QRect(754,148,176,28), Qt::AlignVCenter,QStringLiteral("视觉皮肤"));
    p.setPen(QPen(QColor("#E1E9E8"),1));
    p.drawLine(566,390,926,390);
    p.setPen(INK);
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 11, QFont::Bold));
    p.drawText(QRect(566,393,325,24), Qt::AlignVCenter,
               QStringLiteral("TOP 10  /  本地排行榜"));
    p.setPen(MUTED);
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    p.drawText(QRect(566,365,359,19), Qt::AlignVCenter,
               scene == Scene::Playing ?
                    QStringLiteral("%1 · %2 · %3ms / 格 · %4")
                        .arg(labelMode(game_.mode())).arg(labelDifficulty(game_.difficulty()))
                        .arg(game_.stepIntervalMs()).arg(game_.paused() ? QStringLiteral("暂停") : QStringLiteral("运行")) :
                    QStringLiteral("保存 3 个存档位 · 排行榜自动保留 Top10"));
    const auto rankings = game_.leaderboard().top10();
    for (int i = 0; i < 10; ++i) {
        const int y = 426 + i * 21;
        if (i % 2 == 0) {
            p.setBrush(QColor("#F6FAF9"));
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(QRect(562,y-2,362,20),4,4);
        }
        p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9, i < 3 ? QFont::DemiBold : QFont::Normal));
        p.setPen(i < 3 ? ACCENT : MUTED);
        p.drawText(QRect(568,y,30,17),Qt::AlignVCenter, QString::number(i+1).rightJustified(2,'0'));
        if (i < static_cast<int>(rankings.size())) {
            const auto &item = rankings[static_cast<std::size_t>(i)];
            p.setPen(INK);
            p.drawText(QRect(605,y,200,17),Qt::AlignVCenter,
                       QStringLiteral("%1   %2 · %3")
                           .arg(QString::fromStdString(item.player))
                           .arg(labelMode(item.mode)).arg(labelDifficulty(item.difficulty)));
            p.setPen(ACCENT);
            p.drawText(QRect(835,y,80,17),Qt::AlignVCenter | Qt::AlignRight,
                       QString::number(item.score));
        } else {
            p.setPen(QColor("#A1AFB1"));
            p.drawText(QRect(605,y,290,17),Qt::AlignVCenter,QStringLiteral("—"));
        }
    }
    p.setPen(MUTED);
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    p.drawText(QRect(543,658,395,20),Qt::AlignRight,
               QStringLiteral("单人：↑ ↓ ← →     双人：P1 WASD  /  P2 方向键"));
}
// 叠加提示遮罩；仅改变显示，不修改业务状态。
void Widget::drawOverlay(QPainter &p, const QString &title, const QString &detail) {
    QRect board(BOARD_X, BOARD_Y, game_.width()*CELL, game_.height()*CELL);
    p.setBrush(QColor("#112735"));
    p.setOpacity(0.77);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(board,11,11);
    p.setOpacity(1.0);
    p.setPen(QColor("#C9F1E2"));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 25, QFont::Bold));
    p.drawText(QRect(board.left()+10,board.top()+126,board.width()-20,70),
               Qt::AlignCenter,title);
    p.setPen(QColor("#F2FCFA"));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 12));
    p.drawText(QRect(board.left()+32,board.top()+218,board.width()-64,106),
               Qt::AlignCenter | Qt::TextWordWrap,detail);
}
