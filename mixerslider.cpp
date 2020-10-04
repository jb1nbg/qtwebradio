#include "mixerslider.h"

MixerSlider::MixerSlider(QWidget *parent, alsamixer* _mixer, alsamixerchannel* _channel)
  : QSlider(Qt::Vertical, parent),
    mixer(_mixer),
    channel(_channel)
{
    setObjectName(channel->name);
    setMinimum(0);
    setMaximum(100);
    this->setSliderPosition(mixer->getValue(channel));

    connect(this, &MixerSlider::valueChanged, this, &MixerSlider::setSndCurrentValue);
}

void MixerSlider::setSndCurrentValue(int value)
{
    this->mixer->setValue(this->channel, value);
}
