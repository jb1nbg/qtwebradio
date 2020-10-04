#ifndef WEBSTREAMLISTITEM_H
#define WEBSTREAMLISTITEM_H

#include <QListWidgetItem>

#include "webradiostream.h"

class webstreamlistitem : public QListWidgetItem
{
public:
    webstreamlistitem(QString text, webradiostream* stream);
    ~webstreamlistitem();
    webradiostream* _stream;
};

#endif // WEBSTREAMLISTITEM_H
