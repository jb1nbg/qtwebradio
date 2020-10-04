#ifndef ALSAMIXER_H
#define ALSAMIXER_H

#include <QString>
#include <QList>
#include <alsa/asoundlib.h>

class alsamixerchannel
{
    public:
        enum AlsaMixerChannelType
        {
            Value,
            Switch
        };
        alsamixerchannel(snd_mixer_elem_t* _handle, QString _name, AlsaMixerChannelType _type)
        {
          handle = _handle;
          name = _name;
          type = _type;
        }
        snd_mixer_elem_t *handle;
        QString name;
        AlsaMixerChannelType type;
};

class alsamixer
{
    public:
        alsamixer();
        virtual ~alsamixer();
        int open(QString device);
        void close();
        QList<alsamixerchannel*>* getChannels();
        int getValue(alsamixerchannel* channel);
        void setValue(alsamixerchannel* channel, int value);
        bool getSwitchValue(alsamixerchannel* channel);
        void setSwitchValue(alsamixerchannel* channel, bool value);

    private:
        void getValueRange(alsamixerchannel* channel, long* min, long* max);

    private:
        QString alsaDevice;
        snd_mixer_t *mixerFd;
};

#endif /* ALSAMIXER_H */
