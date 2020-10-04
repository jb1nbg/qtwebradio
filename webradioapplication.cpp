#include "webradioapplication.h"
#include "alsamixerwindow.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QBluetoothAddress>
#include <QDateTime>

webradioapplication::webradioapplication(int &argc, char **argv)
    : QApplication (argc, argv),
      settings("qtwebradio", "WebRadio"),
#ifndef QT_BLUETOOTH_CONTROL
      bluetoothEnabled(false),
#endif      
      audioOutputDevice(defaultAudioOutputDevice),
      evDevReceiver(Q_NULLPTR),
      eventReceiver(Q_NULLPTR),      
      mute(false),      
      volValue(50)
{
#ifdef QT_BLUETOOTH_CONTROL
    blControl = new blctrl();
    blControl->power(false);
#endif
    this->evdev = new evdevlistener();
    connect(this->evdev, &evdevlistener::keyEvent, this, &webradioapplication::on_evDevKeyEvent);
    connect(this->evdev, &evdevlistener::rotaryEvent, this, &webradioapplication::on_evDevRotaryEvent);
    connect(this->evdev, &evdevlistener::touchEvent, this, &webradioapplication::on_evDevTouchEvent);
    this->evdev->run();

    this->player = new sysplayer(); // internal read volume value from system
    connect(this->player, &sysplayer::titleChanged, this, &webradioapplication::on_sysplayer_titleChanged);
    connect(this->player, &sysplayer::streamFinished, this, &webradioapplication::on_sysplayer_finished);

    this->saveVolumeDelayTimer.setSingleShot(true);
    connect(&this->saveVolumeDelayTimer, &QTimer::timeout, this, &webradioapplication::on_saveVolumeDelayTimer_timedout);

    loadVolumeSettings();
    this->vol = new volume(QRect(0, 0, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
    this->vol->setInitValue(this->volValue);
}

void webradioapplication::loadVolumeSettings()
{
    int lastVolume = this->settings.value("Volume_" + this->audioOutputDevice, "30").toInt();
    if (lastVolume != -1)
    {
        this->volValue = lastVolume;
    }
}

void webradioapplication::playerPlay(QUrl url)
{
    this->player->stop();
    this->player->setMedia(url);
    this->player->play(this->audioOutputDevice);
}

void webradioapplication::playerStop()
{
    this->player->stop();
}

void webradioapplication::volumeUp()
{
    if (!this->mute)
    {
        if (this->volValue < 100)
            this->volValue++;

        this->vol->setVolume(this->volValue);
        this->saveVolumeDelayTimer.start(VOLUME_SAVE_DELAY_MSEC);
    }
    else {
        this->vol->setVolume(this->volValue);
    }
}

void webradioapplication::volumeDown()
{
    if (!this->mute)
    {
        if (this->volValue > 0)
            this->volValue--;

        this->vol->setVolume(this->volValue);
        this->saveVolumeDelayTimer.start(VOLUME_SAVE_DELAY_MSEC);
    }
    else {
        this->vol->setVolume(this->volValue);
    }
}

QWidget* webradioapplication::getEvDevReciver()
{
    return this->evDevReceiver;
}

void webradioapplication::setEvDevReceiver(QWidget* widget)
{
    this->evDevReceiver = widget;
}

void webradioapplication::on_evDevKeyEvent(QKeyEvent* event)
{
    //qDebug() << "on_evDevKeyEvent" << this->evDevReceiver;
    if (this->evDevReceiver != Q_NULLPTR)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* ev = static_cast<QKeyEvent*>(event);
            // Application global handled key events
            if ((ev->key() == Qt::Key_Phone) ||       // Equalizer
                (ev->key() == Qt::Key_VolumeMute) ||  // Volume-Mute
                (ev->key() == Qt::Key_VolumeDown) ||  // Volume-Down
                (ev->key() == Qt::Key_VolumeUp))      // Volume-Up
            {
                this->handleGlobalKeyEvents(ev);
                return;
            }
        }
        this->notify(this->evDevReceiver, event);
    }
}

void webradioapplication::on_evDevRotaryEvent(QMouseEvent* event)
{
    //qDebug() << "on_evDevRotaryEvent" << this->evDevReceiver;
    if (this->evDevReceiver != Q_NULLPTR)
    {
        if (event->type() == QEvent::MouseMove)
        {
            // Application global handled mouse move events (rotary encoder events)
            QMouseEvent* ev = static_cast<QMouseEvent*>(event);
            if (ev->pos().x() == 0) // Volume Up/Down
            {
                this->handleGlobalMouseEvents(ev);
                return;
            }
        }
        this->notify(this->evDevReceiver, event);
    }
}

void webradioapplication::on_evDevTouchEvent(QMouseEvent* event)
{
    if (this->evDevReceiver != Q_NULLPTR)
    {
        if (event->type() == QEvent::MouseButtonPress)
            this->eventReceiver = this->evDevReceiver->childAt(static_cast<int>(event->screenPos().x()), static_cast<int>(event->screenPos().y()));

        if (this->eventReceiver != Q_NULLPTR)
        {
            QPointF localPos = this->eventReceiver->mapFromGlobal(QPoint(static_cast<int>(event->screenPos().x()), static_cast<int>(event->screenPos().y())));
            //qDebug() << this->eventReceiver << "localPos" << localPos << "screenPos" << event->screenPos() << "geometry" << this->eventReceiver->geometry();
            //qDebug() << "on_evDevTouchEvent" << this->evDevReceiver << event;
            QMouseEvent forwardEvent(event->type(),
                                     localPos,
                                     event->windowPos(),
                                     event->screenPos(),
                                     event->button(),
                                     event->buttons(),
                                     event->modifiers());

            QWidget* focusWidget = this->focusWidget();
            if (focusWidget != this->eventReceiver)
            {
                this->eventReceiver->setFocus();
            }
            this->notify(this->eventReceiver, &forwardEvent);
        }
    }
}

void webradioapplication::on_sysplayer_finished()
{
    emit playerFinished();
}

void webradioapplication::on_sysplayer_titleChanged(QString title)
{
    emit playerChangeTitle(title);
}

void webradioapplication::showEqualizer()
{
    QWidget* w = this->getEvDevReciver();
    alsamixerwindow alsamixer("equal");
    this->setEvDevReceiver(&alsamixer);
    alsamixer.exec();
    this->setEvDevReceiver(w);
}

void webradioapplication::simulateEvDevKeyEvent(QKeyEvent* event)
{
    handleGlobalKeyEvents(event);
}

void webradioapplication::handleGlobalKeyEvents(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_VolumeMute) || // Button of Volume-RotationEncoder
        (event->key() == Qt::Key_M)) // only for testing on PC
    {
        this->mute = !this->mute;
        this->vol->setMute(this->mute);
    }
    if (event->key() == Qt::Key_VolumeUp)
    {
        volumeUp();
    }
    if (event->key() == Qt::Key_VolumeDown)
    {
        volumeDown();
    }
    else if (event->key() == Qt::Key_Phone)
    {
        this->showEqualizer();
    }
    else if (event->key() == Qt::Key_Plus) // only for testing on PC
    {
        this->volumeUp();
    }
    else if (event->key() == Qt::Key_Minus) // only for testing on PC
    {
        this->volumeDown();
    }
}

void webradioapplication::handleGlobalMouseEvents(QMouseEvent* event)
{
    if (event->pos().x() == 0) // Volume Up/Down
    {
        if (event->pos().y() == 1)
          volumeUp();
        else if (event->pos().y() == -1)
          volumeDown();
    }
}

void webradioapplication::on_saveVolumeDelayTimer_timedout()
{
    this->settings.setValue("Volume_" + this->audioOutputDevice, this->volValue);
}

#ifdef QT_BLUETOOTH_CONTROL
void webradioapplication::enableBluetoothAudio(bool enable)
{
    this->blControl->power(enable);
    if (enable) // wait only if enable bluetooth
    {
        while(!isBluetoothAudioEnabled())
        {
            qDebug() << ".";
            qApp->processEvents();
            QThread::msleep(100);
        }
    }
}
#else
void webradioapplication::enableBluetoothAudio(bool enable)
{
    QString cmd;
    if (enable)
        cmd = "on";
    else
        cmd = "off";

    QProcess process;
    QString program = "/bin/sh -c \"echo -e 'power " + cmd + "' | /usr/bin/bluetoothctl\"";
    qDebug() << program;
    process.start(program);
    process.waitForFinished(5000);
    QString output = QString(process.readAllStandardOutput());
    qDebug() << output;
    if (process.exitCode() == 0)
    {
        this->bluetoothEnabled = enable;
        QThread::msleep(500);
    }
}
#endif

#ifdef QT_BLUETOOTH_CONTROL
void webradioapplication::connectBluetoothDevice(QString bluetoothDevice)
{
    this->blControl->connectService(QBluetoothAddress(bluetoothDevice), QBluetoothUuid::ServiceClassUuid::AudioSink);
}
#else
void webradioapplication::connectBluetoothDevice(QString bluetoothDevice)
{
    QProcess process;
    QString program = "/bin/sh -c \"echo -e 'connect " + bluetoothDevice + "' | /usr/bin/bluetoothctl\"";

    qDebug() << program;
    process.start(program);
    process.waitForFinished(5000);
    QString output = QString(process.readAllStandardOutput());
    qDebug() << output;
}
#endif

#ifdef QT_BLUETOOTH_CONTROL
void webradioapplication::disconnectBluetoothDevice(QString bluetoothDevice)
{
    Q_UNUSED(bluetoothDevice);

    this->blControl->disconnectService();
}
#else
void webradioapplication::disconnectBluetoothDevice(QString bluetoothDevice)
{
    QProcess process;
    QString program = "/bin/sh -c \"echo -e 'disconnect " + bluetoothDevice + "' | /usr/bin/bluetoothctl\"";

    qDebug() << program;
    process.start(program);
    process.waitForFinished(5000);
    QString output = QString(process.readAllStandardOutput());
    qDebug() << output;
}
#endif

#ifdef QT_BLUETOOTH_CONTROL
bool webradioapplication::isBluetoothDeviceConnected(QString bluetoothDevice)
{
    return this->blControl->isDeviceConnected(QBluetoothAddress(bluetoothDevice));
}
#else
bool webradioapplication::isBluetoothDeviceConnected(QString bluetoothDevice)
{
    QProcess process;
    QString program = "/bin/sh -c \"echo -e 'info " + bluetoothDevice + "' | /usr/bin/bluetoothctl | grep Connected:\"";
    qDebug() << program;
    process.start(program);
    process.waitForFinished(5000);
    QString output = QString(process.readAllStandardOutput());
    qDebug() << output;
    return (output.indexOf("yes") >= 0);
}
#endif

#ifdef QT_BLUETOOTH_CONTROL
bool webradioapplication::isBluetoothAudioEnabled()
{
    return this->blControl->powerMode();
}
#else
bool webradioapplication::isBluetoothAudioEnabled()
{
    return bluetoothEnabled;
}
#endif

void webradioapplication::setAudioOutputToDefaultDevice()
{
    this->setAudioOutputDevice(defaultAudioOutputDevice, defaultVolumeDevice, defaultVolumeMasterChannelName);
}

void webradioapplication::setAudioOutputDevice(QString outputDevice, QString volumeDevice, QString audioMasterChannelName)
{
    this->audioOutputDevice = outputDevice;
    this->vol->setAudioOutputDevice(volumeDevice, audioMasterChannelName);
    loadVolumeSettings();
    this->vol->setInitValue(this->volValue);
    if (this->player->isActive())
    {
        QUrl currentMediaUrl = this->player->getMedia();
        this->player->stop();
        this->player->setMedia(currentMediaUrl);
        this->player->play(this->audioOutputDevice);
    }
}
