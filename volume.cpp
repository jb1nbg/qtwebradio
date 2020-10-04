#include "volume.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

#include "webradioapplication.h"

volume::volume(QRect screenRect, QWidget* parent)
    : QWidget(parent),
#ifdef __WITH_VOLUME_FADING__
      opacity(0.0),
#endif
      master(Q_NULLPTR),
      value(0),
      mute(false),
      setVolumeValue(0),
      lastTimerSetVolumeValue(-1)
#ifdef __WITH_VOLUME_FADING__
      ,fadeDirection(true)
#endif
{    
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QWidget { background-color: rgb(0, 0, 0); }");
    const int w = 200;
    const int h = 200;
    QRect r = screenRect;
    QPoint topLeft = r.center() - QPoint(w / 2, h / 2);
    this->setGeometry(topLeft.x(), topLeft.y(), w, h);
    connect(&this->showTimeout, &QTimer::timeout, this, &volume::showTimerTimeout);
    this->showTimeout.setSingleShot(true);
    this->showTimeout.setInterval(1500);

#ifdef __WITH_VOLUME_FADING__
    connect(&this->fadeInOut, &QTimer::timeout, this, &volume::fadeInOutTimeout);
    this->fadeInOut.setSingleShot(true);
    this->fadeInOut.setInterval(25);
#endif
    this->setVolumeTimer.setInterval(500);
    connect(&this->setVolumeTimer, &QTimer::timeout, this, &volume::on_setVolumeTimer_timedout);

    this->setDefaultAudioOutputDevice();
    this->hide();
}

volume::~volume()
{
    delete this->master;
    mixer.close();
}

void volume::setDefaultAudioOutputDevice()
{
    setAudioOutputDevice(static_cast<webradioapplication*>(qApp)->defaultVolumeDevice,
                         static_cast<webradioapplication*>(qApp)->defaultVolumeMasterChannelName);
}

void volume::setAudioOutputDevice(QString audioDevice, QString masterChannelName)
{
    mixer.close();
    if (this->master != Q_NULLPTR)
    {
        delete this->master;
        this->master = Q_NULLPTR;
    }

    mixer.open(audioDevice);
    QList<alsamixerchannel*>* channels = mixer.getChannels();
    for (int i = 0; i< channels->length(); i++)
    {
        alsamixerchannel* channel = channels->at(i);
        qDebug() << channel->name;
        if (channel->name.toLower() == masterChannelName.toLower())
        {
            qDebug() << "Master Channel found: " << masterChannelName;
            this->master = channel;
        }
        else {
            delete channel;
        }
    }
    delete channels;
    qDebug() << "Master Channel " << ((this->master != Q_NULLPTR) ? "found" : "not found");
}

QPainterPath volume::getRoundedCornersRectPath(int radius, int x, int y, int width, int height)
{
    QPainterPath path;
    path.moveTo(x + width, y + radius);
    path.arcTo(QRect(x + width - radius, y, radius, radius), 0.0, +90.0);
    path.lineTo(x + radius, y);
    path.arcTo(QRect(x, y, radius, radius), 90.0, +90.0);
    path.lineTo(x, y + height - radius);
    path.arcTo(QRect(x, y + height - radius, radius, radius), 180.0, +90.0);
    path.lineTo(x + radius, y + height);
    path.arcTo(QRect(x + width- radius, y + height - radius, radius, radius), 270.0, +90.0);
    path.closeSubpath();

    return path;
}

void volume::setInitValue(int v)
{
    this->value = v;
    this->setVolumeValue = v;
    mixer.setValue(this->master, this->setVolumeValue);
}

int volume::getVolume()
{
    return mixer.getValue(this->master);
}

void volume::setVolume(int value)
{    
    if (value > 100)
        value = 100;
    if (value < 0)
        value = 0;

    if (this->mute)
        this->setVolumeValue = 0;
    else
        this->setVolumeValue = value;

    if (!this->setVolumeTimer.isActive())
    {
        qDebug() << "setVolume" << "setVolumeTimer.start()";
        this->setVolumeTimer.start();
    }

#ifdef __WITH_VOLUME_FADING__

    if (!this->showTimeout.isActive())
    {
        this->setWindowOpacity(0.0);
        this->fadeDirection = true;
        fadeInOut.start();
        this->show();
    }
    this->showTimeout.start();
#else
    this->setWindowOpacity(1.0);
    this->show();
    this->showTimeout.start();
#endif
    if (this->value != value)
        this->value = value;

    this->update();
}

void volume::setMute(bool m)
{
    this->mute = m;        
    this->setVolume(this->value);

}

#ifdef __WITH_VOLUME_FADING__
void volume::fadeInOutTimeout()
{
    if ((this->fadeDirection) && (this->opacity < 0.7))
    {
        this->opacity += 0.05;
        this->setWindowOpacity(this->opacity);
        this->fadeInOut.start();
    }
    else if (!this->fadeDirection)
    {
        if (this->opacity > 0.0)
        {
            this->opacity -= 0.05;
            this->setWindowOpacity(this->opacity);
            this->fadeInOut.start();
        }
        else
            this->hide();
    }
}
#endif

void volume::showTimerTimeout()
{
#ifdef __WITH_VOLUME_FADING__
    this->fadeDirection = false;
    this->fadeInOut.start();
#else
    this->hide();
#endif
}

void volume::on_setVolumeTimer_timedout()
{
    qDebug() << "on_setVolumeTimer_timedout" << this->setVolumeValue;
    if (this->lastTimerSetVolumeValue != this->setVolumeValue)
    {
        mixer.setValue(this->master, this->setVolumeValue);
        this->lastTimerSetVolumeValue = this->setVolumeValue;
    }
    else
    {
        qDebug() << "on_setVolumeTimer_timedout" << "setVolumeTimer.stop()";
        this->setVolumeTimer.stop();
    }
}

QPainterPath volume::getTrianglePath(int x, int y, int width, int height)
{
    QPainterPath path;
    path.moveTo(x, y + height);
    path.lineTo(x + width, y + height);
    path.lineTo(x + width, y);
    //path.lineTo(x, y);
    path.lineTo(x, y + height);

    return path;
}

void volume::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect r = this->rect();
    painter.fillRect(r, Qt::transparent);// QColor().fromRgb(255, 255, 255, 255 * this->windowOpacity()));

    QPainterPath bg = getRoundedCornersRectPath(25, 0, 0, this->width(), this->height());
    painter.fillPath(bg, Qt::white);

    painter.setPen(QPen(Qt::darkGray, 2));
    painter.drawPath(getRoundedCornersRectPath(15, 5, 5, this->width() - 10, this->height() - 10));

    int penWidth = 2;
    painter.setPen(QPen(Qt::black, penWidth));
    painter.drawPath(getTrianglePath(25 - (3 + penWidth), 60 - 3, 160, 108));
    if (!this->mute)
    {
        QLinearGradient gradient(25, 60, 160, 60);
        gradient.setColorAt(0, Qt::green);
        gradient.setColorAt(0.7, Qt::yellow);
        gradient.setColorAt(0.9, QColor().fromRgb(255, 128, 0)); // orange
        gradient.setColorAt(1, Qt::red);

        painter.fillPath(getTrianglePath(25 + 3, 60 + 3 + 100 - this->value, (150 * this->value) / 100, this->value), gradient);

        // show Value
        QFont f("Tahoma", 30);
        painter.setFont(f);
        QFontMetrics fm(f);
        QString sValue = QString().setNum(this->value) + " %";
        QRectF bounding = fm.boundingRect(sValue);
        painter.drawText((this->width() - bounding.width()) / 2, bounding.height() + 10, sValue);
    }
    else
    {
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QPixmap icon(":/images/images/mute.png");
        qreal aspectRadio = qreal((qreal)icon.width() / (qreal)icon.height());
        icon = icon.scaled(60, 60 * aspectRadio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap((this->width() - icon.width()) / 2, 20, icon);
    }
}
