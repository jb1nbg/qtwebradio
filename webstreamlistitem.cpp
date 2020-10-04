#include "webstreamlistitem.h"

#include <QDebug>

webstreamlistitem::webstreamlistitem(QString text, webradiostream* stream)
    : QListWidgetItem (text)
{
    this->_stream = stream;
}

webstreamlistitem::~webstreamlistitem()
{
    //qDebug() << "delete stream object";
    if (this->_stream != Q_NULLPTR)
        delete this->_stream;
}
