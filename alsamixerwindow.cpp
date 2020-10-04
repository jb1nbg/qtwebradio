#include "alsamixerwindow.h"
#include "mixerslider.h"
#include "mixertoggle.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDebug>

alsamixerwindow::alsamixerwindow(QString alsaDevice, QWidget *parent)
    : QDialog(parent),
      activeCtrlIdx(0),
      selected(false)
{
    this->setWindowFlag(Qt::FramelessWindowHint, true);

    this->setGeometry(0, 0, 480, 320);
    this->setMaximumSize(QSize(480, 320));
    this->setFixedSize(QSize(480, 320));
    this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    if (this->mixer.open(alsaDevice) != 0)
        qWarning("Init mixer problem");
    else
    {
        //setCentralWidget(buildCentralWidget());
        QLayout* w = buildCentralWidget();
        this->setLayout(w);
    }

    if (this->ctrls.length() > 0)
    {
        this->ctrls.at(0)->setFocus();
        selectWidget(this->ctrls.at(0)->parentWidget());
    }

#ifdef __arm__
    // we receive RotaryEncode Signals as MouseEvent
    this->setMouseTracking(true);
#endif
}

alsamixerwindow::~alsamixerwindow()
{
    this->mixer.close();
}

QLayout *alsamixerwindow::buildCentralWidget()
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(1);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(1);

    QList<alsamixerchannel*>* channels = this->mixer.getChannels();
    for (int i = 0; i < channels->length(); i++)
    {
        alsamixerchannel* channel = channels->at(i);

        if (channel->type == alsamixerchannel::AlsaMixerChannelType::Value)
        {
            QGroupBox *internalGroupBox = new QGroupBox(channel->name.mid(4));
            deselectWidget(internalGroupBox);
            QHBoxLayout *internalHbox = new QHBoxLayout;
            internalHbox->setSpacing(1);
            MixerSlider* mixer = new MixerSlider(this, &this->mixer, channel);
            internalHbox->addWidget(mixer);
            this->ctrls.append(mixer);
            mixer->installEventFilter(this);
            internalGroupBox->setLayout(internalHbox);
            hbox->addWidget(internalGroupBox);
        }
        else if (channel->type == alsamixerchannel::AlsaMixerChannelType::Switch)
        {
            hbox->addWidget(new MixerToggle(this, &this->mixer, channel));
        }
    }
    delete channels;

    vbox->addLayout(hbox);
    QPushButton* pushButtonOk = new QPushButton(this);
    pushButtonOk->setObjectName("pushButtonOk");
    pushButtonOk->setText("Ok");
    pushButtonOk->setDefault(true);
    connect(pushButtonOk, &QPushButton::clicked, this, &QDialog::accept);
    this->ctrls.append(pushButtonOk);
    vbox->addWidget(pushButtonOk);
    return vbox;
}

void alsamixerwindow::selectWidget(QWidget* widget)
{
    widget->setStyleSheet("QGroupBox { background-color: rgba(0, 0, 255, 32); }");
}

void alsamixerwindow::editWidget(QWidget* widget)
{
    widget->setStyleSheet("QGroupBox { background-color: rgba(255, 0, 0, 32); }");
}

void alsamixerwindow::deselectWidget(QWidget* widget)
{
    widget->setStyleSheet("QGroupBox { background-color: rgba(128, 128, 128, 32); }");
}

bool alsamixerwindow::handleFocusInOut(QObject *object, QFocusEvent* event)
{
    //qDebug() << object << event;

    QWidget* w = dynamic_cast<QWidget*>(object);
    if (w != Q_NULLPTR)
    {
        int idx = this->ctrls.indexOf(w);
        if (idx >= 0)
        {
            if (event->type() == QEvent::FocusIn)
            {
                selectWidget(this->ctrls.at(idx)->parentWidget());
                this->activeCtrlIdx = idx;
                this->selected = false;
            }
            else if (event->type() == QEvent::FocusOut)
                deselectWidget(this->ctrls.at(idx)->parentWidget());

            return true;
        }
    }
    return false;
}

void alsamixerwindow::nextCtrl()
{
    if (activeCtrlIdx < (this->ctrls.length() - 1))
        activeCtrlIdx++;
    else
        activeCtrlIdx = 0;

    //qDebug() << this->ctrls.at(activeCtrlIdx)->objectName();
    this->ctrls.at(activeCtrlIdx)->setFocus();
}

void alsamixerwindow::prevCtrl()
{
    if (activeCtrlIdx > 0)
        activeCtrlIdx--;
    else
        activeCtrlIdx = (this->ctrls.length() - 1);

    //qDebug() << this->ctrls.at(activeCtrlIdx)->objectName();
    this->ctrls.at(activeCtrlIdx)->setFocus();
}

bool alsamixerwindow::handleLeftEvent()
{
    if (!this->selected)
        prevCtrl();
    else
    {
        QSlider* slider = dynamic_cast<QSlider*>(this->ctrls.at(activeCtrlIdx));
        if (slider != Q_NULLPTR)
        {
            if (slider->value() > slider->minimum())
                slider->setValue(slider->value() - 1);
        }
    }
    return true;
}

bool alsamixerwindow::handleRightEvent()
{
    if (!this->selected)
        nextCtrl();
    else
    {
        QSlider* slider = dynamic_cast<QSlider*>(this->ctrls.at(activeCtrlIdx));
        if (slider != Q_NULLPTR)
        {
            if (slider->value() < slider->maximum())
                slider->setValue(slider->value() + 1);
        }
    }
    return true;
}

bool alsamixerwindow::handleKeyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Right) // only for testing on PC
    {
        return handleRightEvent();
    }
    else if (event->key() == Qt::Key_Left) // only for testing on PC
    {
        return handleLeftEvent();
    }
    else if (event->key() == Qt::Key_Back)
    {
        this->accept();
        return true;
    }
    else if (event->key() == Qt::Key_Return)
    {
        if (dynamic_cast<QPushButton*>(this->ctrls.at(activeCtrlIdx)) != Q_NULLPTR)
        {
            QPushButton* btn = dynamic_cast<QPushButton*>(this->ctrls.at(activeCtrlIdx));
            btn->click();
        }
        else if (dynamic_cast<QSlider*>(this->ctrls.at(activeCtrlIdx)) != Q_NULLPTR)
        {
            this->selected = !this->selected;
            if (selected)
            {
                editWidget(this->ctrls.at(activeCtrlIdx)->parentWidget());
            }
            else
                selectWidget(this->ctrls.at(activeCtrlIdx)->parentWidget());
        }
        return true;
    }
    return false;
}

void alsamixerwindow::keyPressEvent(QKeyEvent* event)
{
    handleKeyPressEvent(event);
}

void alsamixerwindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->pos().x() == 1) // select ListItem Up/Down
    {
        if (event->pos().y() == 1)
            handleRightEvent();
        else if (event->pos().y() == -1)
            handleLeftEvent();
    }
}

bool alsamixerwindow::eventFilter(QObject *object, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress)
    {
        QKeyEvent* event = static_cast<QKeyEvent*>(ev);
        return this->handleKeyPressEvent(event);
    }
    else if ((ev->type() == QEvent::FocusIn) ||
             (ev->type() == QEvent::FocusOut))
    {
        QFocusEvent* event = static_cast<QFocusEvent*>(ev);
        return handleFocusInOut(object, event);
    }
    return false;

}
