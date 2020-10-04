#include "webradiodatabase.h"
#include "webradiostream.h"

#include <QDebug>
#include <QDir>
#include <QUrlQuery>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>

#include "mainwindow.h"

webradiodatabase::webradiodatabase(QObject* parent, QList<FavoriteUser*>* favUsers) :
    favUsers(favUsers),
    favoriteSavePath("")
{
    this->setParent(parent);
    this->downloadData.clear();
}

void webradiodatabase::loadData(QUrl url)
{
    this->downloadData.clear();

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", QByteArray("XBMC Addon Radio"));
    reply = netAccMng.get(request);

    //connect(reply, &QNetworkReply::finished, this, &webradiodatabase::httpFinished);
    connect(reply, &QIODevice::readyRead, this, &webradiodatabase::httpReadyRead);

    static_cast<MainWindow*>(this->parent())->setProgressBarInfo(url);
    connect(reply, &QNetworkReply::downloadProgress, this, &webradiodatabase::networkReplyProgress);
    connect(reply, &QNetworkReply::finished, this, &webradiodatabase::networkReplyFinish);
    static_cast<MainWindow*>(this->parent())->showProgressBar(true);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
        qDebug() << reply->error() << reply->errorString();
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << status;
    qDebug() << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (status == 301)
    {
        qDebug() << "redirect to URL=" << reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    }
}

void webradiodatabase::networkReplyFinish()
{
    static_cast<MainWindow*>(this->parent())->showProgressBar(false);
}

void webradiodatabase::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    static_cast<MainWindow*>(this->parent())->setProgressBarState(bytesRead, totalBytes);
}

QStringList webradiodatabase::getCategories()
{
    QStringList categories;
    categories.append("Favorite");
    categories.append("Storage");
    categories.append("Genre");
    categories.append("Topic");
    categories.append("Language");
    categories.append("Country");
    categories.append("City");

    return categories;
}

QStringList webradiodatabase::getCategoryItems(QString category)
{
    QStringList items;

    if (category.toLower() == "favorite")
    {
        for (int i = 0; i < this->favUsers->length(); i++)
            items.append(this->favUsers->at(i)->userName);
    }
    else if (category.toLower() == "storage")
    {
        QFileInfoList entries = QDir(MUSIC_BASE_FOLDER).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
        foreach(QFileInfo entryInfo, entries)
        {
            items.append(entryInfo.fileName());
        }
    }
    else
    {        
        //QUrl url("http://radio.de/info/menu/broadcastsofcategory");
        QUrl url("http://radio.de/info/menu/valuesofcategory");
        QUrlQuery query;
        //query.addQueryItem("category", "_language");
        query.addQueryItem("category", "_" + category.toLower());

        url.setQuery(query.query());
        //qDebug() << url;

        loadData(url);
        //qDebug() << this->downloadData;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(this->downloadData);
        QJsonArray jsonArray = jsonResponse.array();

        foreach (const QJsonValue & value, jsonArray)
        {
            items.append(value.toString());
        }
    }
    return items;
}

webradiostream* webradiodatabase::getWebRadioStreamFromJsonObj(QJsonObject obj)
{
    webradiostream* stream = new webradiostream();
    stream->id = obj["id"].toInt();
    stream->name = obj["name"].toString();
    stream->type = static_cast<webradiostream::StreamType>(obj["type"].toInt());
    stream->streamURL = obj["streamURL"].toString();
    stream->pictureBaseURL = obj["pictureBaseURL"].toString();
    stream->picture1TransName = obj["picture1TransName"].toString();


    return stream;
}

webradiostreamlist* webradiodatabase::getCategoryItemStreams(QString category, QString categoryItem)
{
    webradiostreamlist* streams = new webradiostreamlist();

    if (category.toLower() == "favorite")
    {
        QDir dir(this->favoriteSavePath + "/" + categoryItem);
        dir.setNameFilters(QStringList() << "*.fav");
        QStringList files = dir.entryList();
        foreach (const QString & file, files)
        {
            QFile fav(dir.absolutePath() + "/" + file);
            fav.open(QIODevice::ReadOnly);
            QByteArray json = fav.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(json);
            QJsonObject obj = doc.object();
            streams->append(getWebRadioStreamFromJsonObj(obj));
            fav.close();
        }
    }
    else if (category.toLower() == "storage")
    {
        QDir musicPath(MUSIC_BASE_FOLDER);
        musicPath.cd(categoryItem);
        QFileInfoList entries = musicPath.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

        if (musicPath != MUSIC_BASE_FOLDER)
        {
            webradiostream* back = new webradiostream();
            back->id = -1;
            back->name = "..";
            back->type = webradiostream::StreamType::LocalFolder;
            QDir baseDir = musicPath;
            baseDir.cdUp();
            back->streamURL = baseDir.absolutePath();
            streams->append(back);
        }

        QFileInfoList fileEntries = musicPath.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
        if (fileEntries.length() > 0)
        {
            webradiostream* playAll = new webradiostream();
            playAll->id = -1;
            playAll->name = "[All]";
            playAll->type = webradiostream::StreamType::PlayList;
            playAll->streamURL = musicPath.absolutePath();
            streams->append(playAll);
        }

        foreach(QFileInfo entryInfo, entries)
        {
            webradiostream* stream = new webradiostream();
            stream->id = -1;
            stream->name = entryInfo.baseName();
            if (entryInfo.isDir())
                stream->type = webradiostream::StreamType::LocalFolder;
            else
                stream->type = webradiostream::StreamType::LocalFile;
            stream->streamURL = entryInfo.absoluteFilePath();
            streams->append(stream);
        }
    }
    else
    {
        QUrl url("http://radio.de/info/menu/broadcastsofcategory");
        QUrlQuery query;
        query.addQueryItem("category", "_" + category.toLower());
        query.addQueryItem("value", categoryItem.toUtf8());

        url.setQuery(query.query());
        loadData(url);

        //qDebug() << this->downloadData;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(this->downloadData);
        QJsonArray jsonArray = jsonResponse.array();

        foreach (const QJsonValue & value, jsonArray)
        {
            QJsonObject obj = value.toObject();
            streams->append(getWebRadioStreamFromJsonObj(obj));
        }
    }
    return streams;
}

QPixmap webradiodatabase::getStreamIcon(webradiostream* stream)
{
    QPixmap pixmap;
    const QString coverImagePath = "/tmp/cover.png";
    if (stream->type == webradiostream::StreamType::Stream)
    {
        QUrl url(stream->pictureBaseURL + "/" + stream->picture1TransName);
        loadData(url);

        pixmap.loadFromData(this->downloadData);
    }
    else if (stream->type == webradiostream::StreamType::LocalFile)
    {
        QProcess process;
        //QString program = "ffmpeg -y -i \"" + stream->streamURL + "\" " + coverImagePath;
        QString program = "ffmpeg -y -i \"" + stream->streamURL + "\" -filter:v scale=128:128 -an " + coverImagePath;
        //qDebug() << program;
        process.start(program);
        process.waitForFinished(5000);
        if (process.exitCode() == 0)
            pixmap.load(coverImagePath);
    }

    if (!pixmap.isNull())
        return pixmap.scaled(128, 128, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);

    return QPixmap();
}

webradiostream webradiodatabase::getStreamInfo(int id)
{
    QUrl url("http://radio.de//info/broadcast/getbroadcastembedded");
    QUrlQuery query;
    query.addQueryItem("broadcast", QString().setNum(id));

    url.setQuery(query.query());
    loadData(url);

    //qDebug() << this->downloadData;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(this->downloadData);
    QJsonObject obj = jsonResponse.object();

    webradiostream streamInfo;
    streamInfo.id = id;
    streamInfo.name = obj["name"].toString();
    streamInfo.streamURL = obj["streamURL"].toString();

    streamInfo.type = webradiostream::StreamType::Stream;
    if (streamInfo.streamURL.endsWith(".pls") || streamInfo.streamURL.endsWith(".m3u"))
        streamInfo.type = webradiostream::StreamType::PlayList;
    streamInfo.pictureBaseURL = obj["pictureBaseURL"].toString();
    streamInfo.picture1TransName = obj["picture1TransName"].toString();
    streamInfo.currentTrack = obj["currentTrack"].toString();

    return streamInfo;
}

void webradiodatabase::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    this->downloadData.append(reply->readAll());
}

void webradiodatabase::slotAuthenticationRequired(QNetworkReply*, QAuthenticator* authenticator)
{
    authenticator->setUser("user");
    authenticator->setPassword("password");
}

void webradiodatabase::setFavoriteSavePath(QString savePath)
{
    this->favoriteSavePath = savePath;
    for (int i = 0; i < this->favUsers->length(); i++)
    {
        QDir dir(this->favoriteSavePath + "/" + this->favUsers->at(i)->userName);
        if (!dir.exists())
            dir.mkpath(".");
    }
}

QString webradiodatabase::getFavoriteFilename(QString user, webradiostream* stream)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(stream->name.toUtf8());
    return this->favoriteSavePath + "/" + user + "/" + hash.result().toHex() + ".fav";
}

void webradiodatabase::removeFavorityStream(QString user, webradiostream* stream)
{
    QString filename = getFavoriteFilename(user, stream);
    if (QFile::exists(filename))
            QFile::remove(filename);
}

void webradiodatabase::setFavoriteStream(QString user, webradiostream* stream)
{
    QString filename = getFavoriteFilename(user, stream);
    QFile favorite(filename);
    favorite.open(QIODevice::WriteOnly);
    QJsonDocument doc;
    QJsonObject fav;
    fav.insert("id", stream->id);
    fav.insert("name", stream->name);
    fav.insert("type", stream->type);
    fav.insert("streamURL", stream->streamURL);
    fav.insert("pictureBaseURL", stream->pictureBaseURL);
    fav.insert("picture1TransName", stream->picture1TransName);
    doc.setObject(fav);
    favorite.write(doc.toJson());
    favorite.flush();
    favorite.close();
}

bool webradiodatabase::isFavoriteStream(QString user, webradiostream* stream)
{
    QString filename = getFavoriteFilename(user, stream);
    return QFile::exists(filename);
}
