#ifndef WEBRADIOSTREAM_H
#define WEBRADIOSTREAM_H

#include <QList>

class webradiostream
{
public:
    webradiostream();

    enum StreamType
    {
        None,
        Stream,
        PlayList,
        LocalFolder,
        LocalFile,
    };
    int id;
    QString name;
    QString streamURL;
    StreamType type;
    QString pictureBaseURL;
    QString picture1TransName;
    QString currentTrack;
};

class webradiostreamlist : public QList<webradiostream*>
{
public:
    webradiostreamlist();
    ~webradiostreamlist();
};

#endif // WEBRADIOSTREAM_H
