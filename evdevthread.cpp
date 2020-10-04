#include "evdevthread.h"
#include "webradioapplication.h"

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QFile>
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>

evdevthread::evdevthread(QString evdev, bool eventOnSync)
    : evdev(evdev),
    eventOnSync(eventOnSync),
    touchX(0),
    touchY(0),
    touchPressure(0),
    lastTouchPressure(0),
    eventSec(0),
    eventUSec(0)
{
        evDevTranslate[113] = Qt::Key_VolumeMute;
        evDevTranslate[114] = Qt::Key_VolumeDown;
        evDevTranslate[115] = Qt::Key_VolumeUp;
        evDevTranslate[139] = Qt::Key_Menu;
        evDevTranslate[105] = Qt::Key_Left;
        evDevTranslate[106] = Qt::Key_Right;
        evDevTranslate[103] = Qt::Key_Up;
        evDevTranslate[108] = Qt::Key_Down;
        evDevTranslate[128] = Qt::Key_Stop;
        evDevTranslate[169] = Qt::Key_Phone; // Button "Recall", Equalizer
        evDevTranslate[352] = Qt::Key_Return;
        evDevTranslate[407] = Qt::Key_MediaNext;
        evDevTranslate[412] = Qt::Key_MediaPrevious;
        evDevTranslate[59] = Qt::Key_Left;
        evDevTranslate[60] = Qt::Key_MediaPrevious;
        evDevTranslate[61] = Qt::Key_MediaNext;
        evDevTranslate[62] = Qt::Key_Phone; // Equalizer
        evDevTranslate[87] = Qt::Key_VolumeMute;
        evDevTranslate[88] = Qt::Key_Return;
}

void evdevthread::run()
{
    struct libevdev *dev = Q_NULLPTR;
    int rc = 1;

    QFile file(this->evdev);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << QString("Open %1 failed").arg(this->evdev);
        return;
    }

    rc = libevdev_new_from_fd(file.handle(), &dev);
    if (rc < 0) {

        qDebug() << QString("Failed to init libevdev (%1)\n").arg(strerror(-rc));
        goto out;
    }

    /*
    qDebug() << QString().sprintf("Input device ID: bus %#x vendor %#x product %#x\n",
            libevdev_get_id_bustype(dev),
            libevdev_get_id_vendor(dev),
            libevdev_get_id_product(dev));
    qDebug() << QString().sprintf("Evdev version: %x\n", libevdev_get_driver_version(dev));
    qDebug() << QString().sprintf("Input device name: \"%s\"\n", libevdev_get_name(dev));
    qDebug() << QString().sprintf("Phys location: %s\n", libevdev_get_phys(dev));
    qDebug() << QString().sprintf("Uniq identifier: %s\n", libevdev_get_uniq(dev));
    */

    do
    {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING, &ev);
        if (rc == LIBEVDEV_READ_STATUS_SYNC)
        {
            while (rc == LIBEVDEV_READ_STATUS_SYNC)
            {
                /*
                qDebug() << QString().sprintf("SYNC: Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
                    ev.input_event_sec,
                    ev.input_event_usec,
                    ev.type,
                    libevdev_event_type_get_name(ev.type),
                    ev.code,
                    libevdev_event_code_get_name(ev.type, ev.code),
                    ev.value);
                */
                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            }
        }
        else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
        {
            /*
            qDebug() << QString().sprintf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
                ev.input_event_sec,
                ev.input_event_usec,
                ev.type,
                libevdev_event_type_get_name(ev.type),
                ev.code,
                libevdev_event_code_get_name(ev.type, ev.code),
                ev.value);
            */
            if (ev.type == EV_SYN)
            {
                if (ev.code == SYN_REPORT)
                {
                    if (this->eventOnSync)
                        syncTouchEvent();
                }
            }
            else if (ev.type == EV_KEY)
            {
                if (evDevTranslate.contains(ev.code))
                {
                    Qt::Key key = evDevTranslate[ev.code];
                    //qDebug() << "keyEvent" << key;
                    emit keyEvent(new QKeyEvent(((ev.value == 1) ? QEvent::Type::KeyPress :  QEvent::Type::KeyRelease), key, Qt::NoModifier));
                }
            }
            else if (ev.type == EV_REL)
            {
                //qDebug() << "rotationEvent";
                // in der Position wird in X der Code, d.h. von welchem RotationEncoder es kommt kodiert
                // in der Position wird in Y der EncodeOffst (+1 / -1) gespeichert
                emit rotaryEvent(new QMouseEvent(QEvent::Type::MouseMove, QPoint(ev.code, -ev.value), Qt::NoButton, Qt::NoButton, Qt::NoModifier));
            }
            else if (ev.type == EV_ABS)
            {
                //qDebug() << "touchEvent";
                handleTouchEvent(&ev);
            }
        }
    } while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

    if (rc != LIBEVDEV_READ_STATUS_SUCCESS && rc != -EAGAIN)
        qDebug() << QString("Failed to handle events: %1\n").arg(strerror(-rc));

out:
    libevdev_free(dev);
}

void evdevthread::handleTouchEvent(struct input_event* ev)
{
    if (ev->code == ABS_X)
        this->touchX = ev->value;
    else if (ev->code == ABS_Y)
        this->touchY = ev->value;
    else if (ev->code == ABS_PRESSURE)
        this->touchPressure = ev->value;
}

void evdevthread::syncTouchEvent()
{
    QEvent::Type type;
    if ((this->lastTouchPressure == 0) && (this->touchPressure > 0))
        type = QEvent::MouseButtonPress;
    else if ((this->lastTouchPressure > 0) && (this->touchPressure == 0))
        type = QEvent::MouseButtonRelease;
    else
        type = QEvent::MouseMove;

    QPoint pos = QPoint(((this->touchX * MAX_SCREEN_WIDTH) / 4096), 320 - ((this->touchY * MAX_SCREEN_HEIGHT) / 4096));
    emit touchEvent(new QMouseEvent(type, pos, pos, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));

    this->lastTouchPressure = this->touchPressure;
}
