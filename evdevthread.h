#ifndef EVDEVTHREAD_H
#define EVDEVTHREAD_H

#include <QThread>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHash>

#include <bits/types.h>
#include <linux/input.h>
#include <libevdev.h>

class evdevthread : public QThread
{
    Q_OBJECT

public:
    evdevthread(QString evdev, bool eventOnSync = false);
    void run() Q_DECL_OVERRIDE;

signals:
    void keyEvent(QKeyEvent *event);
    void rotaryEvent(QMouseEvent *event);
    void touchEvent(QMouseEvent *event);

private:
    QString evdev;
    bool eventOnSync;
    QHash<int, Qt::Key> evDevTranslate;

    int touchX;
    int touchY;
    int touchPressure;
    int lastTouchPressure;
    __time_t eventSec;
    __suseconds_t eventUSec;

private:
    void handleTouchEvent(struct input_event* ev);
    void syncTouchEvent();
};

#endif // EVDEVTHREAD_H
