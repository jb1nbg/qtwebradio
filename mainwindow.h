#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QList>
#include <QListWidgetItem>
#include <QPushButton>
#include <QDir>
#include <QUrl>
#include <QKeyEvent>

#include "webradiodatabase.h"
#include "webradiostream.h"
#include "playlist.h"
#include "webradioapplication.h"
#include "favoriteuser.h"
#include "clock.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QRect screenRect, QWidget *parent = Q_NULLPTR);
    ~MainWindow() Q_DECL_OVERRIDE;    
    void showProgressBar(bool visible);
    void setProgressBarInfo(QUrl url);
    void setProgressBarState(qint64 value, qint64 max);

private:
    void fillCategories();
    void downloadCategoryItems(QString category);
    void playStream(int streamId);
    void playStream(webradiostream* streamInfo);
    void stopStream();
    void setFavoriteToggleButton(webradiostream *stream);
    void setFavoriteToggleButton(QPushButton* favButton, QString user, webradiostream* stream);
    void toggleFavorite_toggled(QPushButton* favButton, QString user, bool checked);
    QListWidget* getActiveListWidget();
    void listWidgetVisible(QListWidget* listWidget, bool visible);
    void listWidgetCategoryVisible(bool visible);
    void listWidgetCategoryItemsVisible(bool visible);
    void listWidgetStreamsVisible(bool visible);
    bool isWidgetPlayVisible();    
    void widgetPlayVisible(bool visible);
    void navigationVisible(bool visible);
    void listWidgetUpDownVisible(bool visible);

    void goHome();
    void goUp();
    void goDown();
    void doStop();
    void goEqualizer();
    void showClock();
    void startScreensaveTimer();
    void stopScreensaveTimer();
    void resetScreensaveTimer();

private slots:
    void on_listWidgetCategory_itemClicked(QListWidgetItem *item);
    void on_listWidgetCategoryItems_itemClicked(QListWidgetItem *item);
    void on_listWidgetStreams_itemClicked(QListWidgetItem *item);
    void on_pushButtonStop_clicked();
    void on_homeIcon_clicked();
    void on_labelCategory_clicked();
    void on_labelCategoryItem_clicked();
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_pushButtonToggleFavorite_toggled(bool checked);
    void on_pushButtonEqualizer_clicked();

    void on_playerChangeTitle(QString title);
    void on_playerFinished();
    void on_playListChangeStream(webradiostream* newStream);
    void on_screenSaveTimerTimedOut();

    void on_pushButtonToggleOutputDevice_clicked();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;
    webradiodatabase database;
    webradioapplication* wApp;
    playlist playlistMgn;
    bool bStartWithLastStream;
    webradiostream* currentPlayStream;    
    QList<FavoriteUser*> favUsers;
    Clock* clock;
    QTimer screenSaveTimer;
};

#endif // MAINWINDOW_H
