#include "clock.h"
#include "ui_clock.h"

#include "webradioapplication.h"

#include <QDateTime>

Clock::Clock(QRect screenRect, QWidget *parent) :
    QDialog(parent),
    windowBefore(Q_NULLPTR),
    ui(new Ui::Clock)
{
    this->setObjectName("Clock");
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setGeometry(screenRect);
    this->setFixedSize(screenRect.size());
    this->setMaximumSize(screenRect.size());

    QObject::connect(&this->clockUpdateTimer, &QTimer::timeout, this, &Clock::on_clockUpdateTimerTimedout);
    this->clockUpdateTimer.setInterval(1000);
    this->clockUpdateTimer.start();
}

Clock::~Clock()
{
    delete ui;
}

void Clock::on_clockUpdateTimerTimedout()
{
    QDateTime dt = QDateTime::currentDateTime();
    this->ui->labelDate->setText(dt.date().toString("dd.MM.yyyy"));
#ifdef WITH_BLINKING_DOT
    if ((dt.time().second() % 2) == 0)
        this->ui->labelTime->setText(dt.time().toString("HH.mm"));
    else
        this->ui->labelTime->setText(dt.time().toString("HH:mm"));
#else
    this->ui->labelTime->setText(dt.time().toString("HH:mm"));
#endif
}

void Clock::keyPressEvent(QKeyEvent* event)
{
    Q_UNUSED(event);

    this->accept();
}

void Clock::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    this->accept();
}

bool Clock::eventFilter(QObject *object, QEvent *ev)
{
    Q_UNUSED(object);

    if (ev->type() == QEvent::KeyPress)
    {
        this->accept();
        return true;
    }
    return false;

}

void Clock::on_labelTime_clicked()
{
    this->accept();
}

void Clock::on_labelDate_clicked()
{
    this->accept();
}
