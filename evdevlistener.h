#ifndef EVDEVLISTENER_H
#define EVDEVLISTENER_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include "evdevthread.h"

class evdevlistener : public QObject
{
    Q_OBJECT

public:
    evdevlistener();
    void run();

signals:
    void keyEvent(QKeyEvent* event);
    void rotaryEvent(QMouseEvent* event);
    void touchEvent(QMouseEvent* event);

private slots:
    void on_threadKeyEvent(QKeyEvent* event);
    void on_threadRotaryEvent(QMouseEvent* event);
    void on_threadTouchEvent(QMouseEvent* event);

private:
    evdevthread* rotary_volume;
    evdevthread* rotary_select;
    evdevthread* button_mute;
    evdevthread* button_select;
    evdevthread* touchscreen;
    //evdevthread* button_key1;
    evdevthread* button_key2;
    evdevthread* button_key3;
    evdevthread* button_key4;
    evdevthread* button_remote;
};

#endif // EVDEVLISTENER_H
