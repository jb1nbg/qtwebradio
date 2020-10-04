#ifndef WEBRADIOAPPLICATION_H
#define WEBRADIOAPPLICATION_H

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QSettings>
#include <QTimer>

#include "evdevlistener.h"
#include "sysplayer.h"
#include "volume.h"

#define MAX_SCREEN_WIDTH    480
#define MAX_SCREEN_HEIGHT   320

// if the user changed the Audio Volume then we save not on each step,
// but after 2 sec if no new Volume change was received
#define VOLUME_SAVE_DELAY_MSEC      2000

#define QT_BLUETOOTH_CONTROL

#ifdef QT_BLUETOOTH_CONTROL
#include <blctrl.h>
#endif

class webradioapplication : public QApplication
{
    Q_OBJECT
public:
    webradioapplication(int &argc, char **argv);
    void playerPlay(QUrl url);
    void playerStop();
    void playerNext();
    void showEqualizer();
    QWidget* getEvDevReciver();
    void setEvDevReceiver(QWidget* widget);
    void simulateEvDevKeyEvent(QKeyEvent* event);
    void setAudioOutputDevice(QString outputDevice, QString volumeDevice, QString audioMasterChannelName);
    void setAudioOutputToDefaultDevice();
    void enableBluetoothAudio(bool enable);
    bool isBluetoothAudioEnabled();
    void connectBluetoothDevice(QString bluetoothDevice);
    void disconnectBluetoothDevice(QString bluetoothDevice);
    bool isBluetoothDeviceConnected(QString bluetoothDevice);

public:
    QSettings settings;
    const QString defaultVolumeMasterChannelName = "master";
    const QString defaultVolumeDevice = "default";
#ifdef __arm__
    const QString defaultAudioOutputDevice = "equal";
#else
    const QString defaultAudioOutputDevice = "default";
#endif

signals:
    void playerChangeTitle(QString title);
    void playerFinished();

private:    
#ifdef QT_BLUETOOTH_CONTROL
    blctrl *blControl;
#else
    bool bluetoothEnabled;
#endif
    QString audioOutputDevice;
    evdevlistener* evdev;
    QWidget* evDevReceiver;
    QWidget* eventReceiver;
    bool mute;    
    volume* vol;
    int volValue;
    sysplayer* player;
    QTimer saveVolumeDelayTimer;

private:
    void volumeUp();
    void volumeDown();
    void loadVolumeSettings();
    void handleGlobalKeyEvents(QKeyEvent* event);
    void handleGlobalMouseEvents(QMouseEvent* event);

private slots:
    void on_saveVolumeDelayTimer_timedout();    
    void on_evDevKeyEvent(QKeyEvent* event);
    void on_evDevRotaryEvent(QMouseEvent* event);
    void on_evDevTouchEvent(QMouseEvent* event);
    void on_sysplayer_titleChanged(QString title);
    void on_sysplayer_finished();
};

#endif // WEBRADIOAPPLICATION_H
