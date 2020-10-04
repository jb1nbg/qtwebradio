#ifndef MIXERSLIDER_H
#define MIXERSLIDER_H

#include <QSlider>
#include "alsamixer.h"

class MixerSlider : public QSlider
{
    Q_OBJECT

    public:
        MixerSlider(QWidget *parent, alsamixer* mixer, alsamixerchannel* channel);

    private slots:
        void setSndCurrentValue(int value);

    private:
        alsamixer* mixer;
        alsamixerchannel* channel;
        long min, max;
};

#endif /* MIXERSLIDER_H */
