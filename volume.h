#ifndef VOLUME_H
#define VOLUME_H

#include <QWidget>
#include <QTimer>
#include <QPainterPath>

#include "alsamixer.h"

class volume : public QWidget
{
public:
    volume(QRect screenRect, QWidget* parent = Q_NULLPTR);
    ~volume() Q_DECL_OVERRIDE;
    void setInitValue(int v);
    void setDefaultAudioOutputDevice();
    void setAudioOutputDevice(QString audioDevice, QString masterChannelName);
    void setVolume(int value);
    int getVolume();
    void setMute(bool m);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private slots:
    void showTimerTimeout();
    void on_setVolumeTimer_timedout();
#ifdef __WITH_VOLUME_FADING__
    void fadeInOutTimeout();
#endif

private:
    alsamixer mixer;
    alsamixerchannel* master;
#ifdef __WITH_VOLUME_FADING__
    qreal opacity;
    bool fadeDirection;
    QTimer fadeInOut;
#endif
    int value;
    bool mute;
    QTimer showTimeout;

    int setVolumeValue;
    int lastTimerSetVolumeValue;
    QTimer setVolumeTimer;

    QPainterPath getRoundedCornersRectPath(int radius, int x, int y, int width, int height);
    QPainterPath getTrianglePath(int x, int y, int width, int height);
};

#endif // VOLUME_H
