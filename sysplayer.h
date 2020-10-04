#ifndef SYSPLAYER_H
#define SYSPLAYER_H

#include <QObject>
#include <QUrl>
#include <QProcess>
#include <QString>

class sysplayer : public QObject
{
    Q_OBJECT

public:
    sysplayer();

    void play(QString audioOutputDevice);
    void stop();
    void setMedia(QUrl url);
    QUrl getMedia() { return currentMedia; }
    bool isActive() { return (playProcess != Q_NULLPTR); }

signals:
    void titleChanged(QString title);
    void streamFinished();

private:
    QUrl currentMedia;
    QProcess* playProcess;

private slots:
    void readStdOut();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SYSPLAYER_H
