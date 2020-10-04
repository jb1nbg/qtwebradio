#ifndef CLOCK_H
#define CLOCK_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class Clock;
}

class Clock : public QDialog
{
    Q_OBJECT

public:
    explicit Clock(QRect screenRect, QWidget *parent = nullptr);
    ~Clock();
    QWidget* windowBefore;

protected:
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_clockUpdateTimerTimedout();

    void on_labelTime_clicked();

    void on_labelDate_clicked();

private:
    QTimer clockUpdateTimer;
    Ui::Clock *ui;
};

#endif // CLOCK_H
