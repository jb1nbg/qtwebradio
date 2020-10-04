#include "alsamixer.h"

#include <QDebug>

alsamixer::alsamixer()
    : alsaDevice(""),
      mixerFd(Q_NULLPTR)
{
}

alsamixer::~alsamixer()
{
    close();
}

int alsamixer::open(QString device)
{
    if (mixerFd == Q_NULLPTR)
    {
        int ret = 0;

        if ((ret = snd_mixer_open(&mixerFd, 0)) < 0)
        {
            qWarning() << "snd_mixer_open error" << ret;
            goto err;
        }

        if ((ret = snd_mixer_attach(mixerFd, device.toUtf8())) < 0)
        {
            qWarning() << "snd_mixer_attach error" << ret;
            goto err;
        }

        if ((ret = snd_mixer_selem_register(mixerFd, Q_NULLPTR, Q_NULLPTR)) < 0)
        {
            qWarning() << "snd_mixer_selem_register error" << ret;
            goto err;
        }
        if ((ret = snd_mixer_load(mixerFd)) < 0)
        {
            qWarning() << "snd_mixer_load error" << ret;
            goto err;
        }

        goto ok;

err:
        if (mixerFd)
            snd_mixer_close(mixerFd);
        mixerFd = Q_NULLPTR;

ok:
        this->alsaDevice = device;
        return ret;
    }
    else {
        return -1;
    }
}

void alsamixer::close()
{
    if (mixerFd != Q_NULLPTR)
    {
        snd_mixer_detach(mixerFd, alsaDevice.toUtf8());
        snd_mixer_close(mixerFd);
        mixerFd = Q_NULLPTR;
    }
    this->alsaDevice = "";
}

QList<alsamixerchannel*>* alsamixer::getChannels()
{
    QList<alsamixerchannel*>* channels = new QList<alsamixerchannel*>();

    snd_mixer_elem_t *elem;
    for (elem = snd_mixer_first_elem(mixerFd); elem; elem = snd_mixer_elem_next(elem))
    {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(elem))
        {
            QString elemName = QString(snd_mixer_selem_get_name(elem));

            if (snd_mixer_selem_has_playback_volume(elem))
            {
                alsamixerchannel* channel = new alsamixerchannel(elem, elemName, alsamixerchannel::AlsaMixerChannelType::Value);
                channels->append(channel);
            }
            else if (snd_mixer_selem_has_playback_switch(elem))
            {
                alsamixerchannel* channel = new alsamixerchannel(elem, elemName, alsamixerchannel::AlsaMixerChannelType::Switch);
                channels->append(channel);
            }
        }
    }
    return channels;
}

void alsamixer::getValueRange(alsamixerchannel* channel, long* min, long* max)
{
    if (channel != Q_NULLPTR)
        snd_mixer_selem_get_playback_volume_range(channel->handle, min, max);
    else {
        *min = 0;
        *max = 0;
    }
}

int alsamixer::getValue(alsamixerchannel* channel)
{
    long min, max;

    if (channel != Q_NULLPTR)
    {
        getValueRange(channel, &min, &max);

        long currentValue;
        snd_mixer_selem_get_playback_volume(channel->handle, SND_MIXER_SCHN_FRONT_LEFT, &currentValue);

        if (currentValue < 0 || currentValue > 255)
            return 0;
        else
            return static_cast<int>((currentValue * 100) / max);
    }
    return 0;
}

void alsamixer::setValue(alsamixerchannel* channel, int value)
{
    long min, max;

    if (channel != Q_NULLPTR)
    {
        getValueRange(channel, &min, &max);

        int realValue = static_cast<int>((value * max) / 100);
        snd_mixer_selem_set_playback_volume(channel->handle, SND_MIXER_SCHN_FRONT_LEFT, realValue);
        snd_mixer_selem_set_playback_volume(channel->handle, SND_MIXER_SCHN_FRONT_RIGHT, realValue);
    }
}

bool alsamixer::getSwitchValue(alsamixerchannel* channel)
{
    int current = 0;

    if (channel != Q_NULLPTR)
        snd_mixer_selem_get_playback_switch(channel->handle, SND_MIXER_SCHN_FRONT_LEFT, &current);

    return (current > 0);
}

void alsamixer::setSwitchValue(alsamixerchannel* channel, bool value)
{
    if (channel != Q_NULLPTR)
        snd_mixer_selem_set_playback_switch(channel->handle, SND_MIXER_SCHN_FRONT_LEFT, ((value) ? 1 : 0));
}
