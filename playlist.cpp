#include "playlist.h"

#include <QUrl>
#include <QDir>
#include <QUrlQuery>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>

playlist::playlist()
    : playListActive(false),
    playListIndex(-1)
{
}


QStringList playlist::downloadPlaylist(QUrl playlistFile)
{
    downloadData.clear();

    QNetworkRequest request(playlistFile);
    request.setRawHeader("User-Agent", QByteArray("XBMC Addon Radio"));
    reply = netAccMng.get(request);

    //connect(reply, &QNetworkReply::finished, this, &webradiodatabase::httpFinished);
    connect(reply, &QIODevice::readyRead, this, &playlist::httpReadyRead);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return QString(this->downloadData).split('\n');
}

void playlist::playAll(webradiostream* streamInfo)
{
    this->stop();

    if (streamInfo->streamURL.endsWith(".pls"))
    {
        QStringList data = downloadPlaylist(streamInfo->streamURL);
        foreach(const QString & line, data)
        {
            if (line.indexOf("File") >= 0)
            {
                QString url = line.mid(line.indexOf("=") + 1).trimmed();

                webradiostream* ws = new webradiostream(*streamInfo);
                ws->type = webradiostream::StreamType::Stream;
                ws->streamURL = url;
                emit changeStream(ws);
                delete ws;
                break;
            }
        }
    }
    else if (streamInfo->streamURL.endsWith(".m3u"))
    {
        QStringList data = downloadPlaylist(streamInfo->streamURL);
        foreach(const QString & line, data)
        {
            if ((line.indexOf("http://") >= 0) || (line.indexOf("https://") >= 0))
            {
                webradiostream* ws = new webradiostream(*streamInfo);
                ws->type = webradiostream::StreamType::Stream;
                ws->streamURL = line.trimmed();
                emit changeStream(ws);
                delete ws;
                break;
            }
        }
    }
    else
    {
        QDir dir(streamInfo->streamURL);
        if (dir.exists())
        {
            QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
            foreach(QFileInfo entryInfo, entries)
            {
                this->playlistFiles.append(QUrl(entryInfo.absoluteFilePath()));
            }
            if (this->playlistFiles.length() > 0)
            {
                this->playListIndex = 0;
                this->playListActive = true;

                webradiostream* ws = getPlayListFileStreamInfo(this->playlistFiles.at(0));
                emit changeStream(ws);
                delete ws;
            }
        }
    }
}

webradiostream* playlist::getPlayListFileStreamInfo(QUrl url)
{
    webradiostream* ws = new  webradiostream();
    ws->id = -1;
    ws->type = webradiostream::StreamType::LocalFile;
    ws->streamURL = url.path();

    return ws;
}

void playlist::stop()
{
    this->playlistFiles.clear();
    this->playListActive = false;
    this->playListIndex = -1;
}

//#error "weitermachen - testen"

void playlist::prev()
{
    this->playListIndex--;
    if (this->playListIndex < 0)
        this->playListIndex = this->playlistFiles.length() -1;

    webradiostream* ws = getPlayListFileStreamInfo(this->playlistFiles.at(this->playListIndex));
    emit changeStream(ws);
    delete ws;
}

void playlist::next()
{
    this->playListIndex++;
    if (this->playListIndex >= this->playlistFiles.length())
        this->playListIndex = 0;

    webradiostream* ws = getPlayListFileStreamInfo(this->playlistFiles.at(this->playListIndex));
    emit changeStream(ws);
    delete ws;
}

void playlist::streamFinished()
{
    if (this->playListActive)
    {
        this->next();
    }
}

void playlist::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    this->downloadData.append(reply->readAll());
}
