#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QStringList>
#include <QList>

#include "webradiostream.h"

class playlist : public QObject
{
    Q_OBJECT

public:
    playlist();
    void playAll(webradiostream* streamInfo);
    void stop();
    void prev();
    void next();
    bool isActive() { return playListActive; }

signals:
    void changeStream(webradiostream* newStream);

public slots:
    void streamFinished();

private slots:
    void httpReadyRead();    

private:
    QNetworkAccessManager netAccMng;
    QNetworkReply *reply;
    QByteArray downloadData;
    QList<QUrl> playlistFiles;
    bool playListActive;
    int playListIndex;

private:
    QStringList downloadPlaylist(QUrl playlistFile);
    webradiostream* getPlayListFileStreamInfo(QUrl url);
};

#endif // PLAYLIST_H
