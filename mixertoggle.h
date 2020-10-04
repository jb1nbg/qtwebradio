#ifndef MIXERTOGGLE_H
#define MIXERTOGGLE_H

#include <QCheckBox>
#include "alsamixer.h"

class MixerToggle : public QCheckBox
{
    Q_OBJECT

    public:
        MixerToggle(QWidget *parent, alsamixer* _mixer, alsamixerchannel* _channel);

    private slots:
        void setSndState(int state);

    private:
        alsamixer* mixer;
        alsamixerchannel* channel;
};

#endif /* MIXERTOGGLE_H */
