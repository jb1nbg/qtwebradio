#include "webradiostream.h"

webradiostream::webradiostream() :
    id(-1),
    type(StreamType::None)
{
}

webradiostreamlist::webradiostreamlist()
{
}

webradiostreamlist::~webradiostreamlist()
{
    for (int i = 0; i < this->count(); i++)
    {
        delete this->at(i);
    }
    this->clear();
}
