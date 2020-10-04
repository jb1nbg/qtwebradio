#include "evdevlistener.h"

#include <QDebug>

evdevlistener::evdevlistener()
{
    this->touchscreen = new evdevthread("/dev/input/touchscreen", true);
    connect(this->touchscreen, &evdevthread::touchEvent, this, &evdevlistener::on_threadTouchEvent);
    this->button_mute = new evdevthread("/dev/input/mute");
    connect(this->button_mute, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->button_select = new evdevthread("/dev/input/return");
    connect(this->button_select, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->rotary_volume = new evdevthread("/dev/input/volume");
    connect(this->rotary_volume, &evdevthread::rotaryEvent, this, &evdevlistener::on_threadRotaryEvent);
    this->rotary_select = new evdevthread("/dev/input/select");
    connect(this->rotary_select, &evdevthread::rotaryEvent, this, &evdevlistener::on_threadRotaryEvent);
    //this->button_key1= new evdevthread("/dev/input/key1");
    //connect(this->button_key1, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->button_key2 = new evdevthread("/dev/input/key2");
    connect(this->button_key2, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->button_key3 = new evdevthread("/dev/input/key3");
    connect(this->button_key3, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->button_key4 = new evdevthread("/dev/input/key4");
    connect(this->button_key4, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);
    this->button_remote = new evdevthread("/dev/input/remote");
    connect(this->button_remote, &evdevthread::keyEvent, this, &evdevlistener::on_threadKeyEvent);

}

void evdevlistener::run()
{
    this->touchscreen->start();
    this->button_mute->start();
    this->button_select->start();
    this->rotary_volume->start();
    this->rotary_select->start();
    //this->button_key1->start();
    this->button_key2->start();
    this->button_key3->start();
    this->button_key4->start();
    this->button_remote->start();
}

void evdevlistener::on_threadKeyEvent(QKeyEvent* event)
{
    //qDebug() << "on_threadKeyEvent";
    emit keyEvent(event);
}

void evdevlistener::on_threadRotaryEvent(QMouseEvent* event)
{
    //qDebug() << "on_threadRotaryEvent";
    emit rotaryEvent(event);
}

void evdevlistener::on_threadTouchEvent(QMouseEvent* event)
{
    //qDebug() << "on_threadTouchEvent" << event;
    emit touchEvent(event);
}
