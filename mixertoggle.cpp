#include "mixertoggle.h"

MixerToggle::MixerToggle(QWidget *parent, alsamixer* _mixer, alsamixerchannel* _channel)
  : QCheckBox(_channel->name, parent),
    mixer(_mixer),
    channel(_channel)
{
    setChecked(mixer->getSwitchValue(_channel));
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(setSndState(int)));
}

void MixerToggle::setSndState(int state)
{
    this->mixer->setSwitchValue(this->channel, ((state == Qt::Checked) ? 1 : 0));
}
