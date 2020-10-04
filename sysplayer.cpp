#include "sysplayer.h"
#include <QDebug>
#include <QStringList>

sysplayer::sysplayer()
    : QObject(),
      playProcess(Q_NULLPTR)
{
}

void sysplayer::play(QString audioOutputDevice)
{
    if (playProcess != Q_NULLPTR)
        this->stop();

    this->playProcess = new QProcess();
    QString outputDevice = "-a " + audioOutputDevice;
    QString outputDriver = "-o alsa";
    QString program = "mpg123 " + outputDriver + " " + outputDevice + " \"" + this->currentMedia.toString() + "\"";
    qDebug() << program;
    //connect(this->playProcess, &QProcess::readyReadStandardOutput, this, &sysplayer::readStdOut);
    connect(this->playProcess, &QProcess::readyReadStandardError, this, &sysplayer::readStdOut);
    connect(this->playProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &sysplayer::processFinished);
    this->playProcess->start(program);
}

void sysplayer::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    if (exitCode == 0) // normal exit without a crash
        emit streamFinished();
}

void sysplayer::readStdOut()
{
    const QString patternStreamTitle = "ICY-META: StreamTitle='";
    const QString fileTitle = "Title:";
    const QString fileArtist = "Artist:";
    QString title = "";
    QString artist = "";

    if (this->playProcess != Q_NULLPTR)
    {
        //QTextCodec* codec = QTextCodec::codecForName("ISO 8859-1");
        //QByteArray encodedString = codec->fromUnicode(QString(this->playProcess->readAll()));
        QStringList output = QString(this->playProcess->readAllStandardError()).split('\n', QString::SkipEmptyParts);
        //qDebug() << output;
        foreach(const QString & line, output)
        {
            int posStream = line.indexOf(patternStreamTitle);
            if (posStream >= 0)
            {
                int startpos = posStream + patternStreamTitle.length();
                QString title = line.mid(startpos, line.indexOf("';") - startpos).trimmed();
                //qDebug() << title;
                emit titleChanged(title);
            }
            else
            {
                int posFileTitle = line.indexOf(fileTitle);
                if (posFileTitle >= 0)
                {
                    int startpos = posFileTitle + fileTitle.length();
                    title = line.mid(startpos, line.indexOf(fileArtist) - startpos).trimmed();
                    //qDebug() << title;
                }
                int posFileArtist = line.indexOf(fileArtist);
                if (posFileArtist >= 0)
                {
                    int startpos = posFileArtist + fileArtist.length();
                    artist = line.mid(startpos).trimmed();
                    qDebug() << artist;
                    emit titleChanged(title + " - " + artist);
                }
            }
        }
    }
}

void sysplayer::stop()
{
    if (this->playProcess != Q_NULLPTR)
    {
        QObject::disconnect(this->playProcess, &QProcess::readyReadStandardOutput, this, &sysplayer::readStdOut);
        this->playProcess->kill();
    }
    this->playProcess = Q_NULLPTR;
}

void sysplayer::setMedia(QUrl url)
{
    this->currentMedia = url;
}
