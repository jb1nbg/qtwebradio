#ifndef WEBRADIODATABASE_H
#define WEBRADIODATABASE_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QPixmap>

#include "webradiostream.h"
#include "favoriteuser.h"

#define MUSIC_BASE_FOLDER   QString("/media")

class webradiodatabase : public QObject
{
    Q_OBJECT

public:
    webradiodatabase(QObject* parent, QList<FavoriteUser*>* favUsers);

    QStringList getCategories();
    QStringList getCategoryItems(QString category);
    webradiostreamlist* getCategoryItemStreams(QString category, QString genre);
    webradiostream getStreamInfo(int id);
    QPixmap getStreamIcon(webradiostream* stream);
    void setFavoriteSavePath(QString savePath);
    void setFavoriteStream(QString user, webradiostream* stream);
    void removeFavorityStream(QString user, webradiostream* stream);
    bool isFavoriteStream(QString user, webradiostream* stream);

private slots:
    void httpReadyRead();
    void slotAuthenticationRequired(QNetworkReply*, QAuthenticator *authenticator);
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
    void networkReplyFinish();

private:
    QList<FavoriteUser*>* favUsers;
    QString favoriteSavePath;
    QNetworkAccessManager netAccMng;
    QNetworkReply *reply;
    QByteArray downloadData;

private:
    void loadData(QUrl url);
    QString getFavoriteFilename(QString user, webradiostream* stream);
    webradiostream* getWebRadioStreamFromJsonObj(QJsonObject obj);
};

#endif // WEBRADIODATABASE_H
