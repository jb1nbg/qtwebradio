#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webstreamlistitem.h"
#include "webradioapplication.h"

#include <QUrl>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QtNetwork>
#include <QScrollBar>
#include <QSizePolicy>

#define SCREENSAVER_TIME    5000 // 5.sec

MainWindow::MainWindow(QRect screenRect, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      ui(new Ui::MainWindow),      
      database(this, &favUsers),
      bStartWithLastStream(false),
      currentPlayStream(Q_NULLPTR)
{
    wApp = static_cast<webradioapplication*>(qApp);
    QFile style("://style");
    style.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(style.readAll());

    ui->setupUi(this);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setGeometry(screenRect);
    this->setFixedSize(screenRect.size());
    this->setMaximumSize(screenRect.size());

    showProgressBar(false);
    listWidgetCategoryItemsVisible(false);
    listWidgetStreamsVisible(false);
    widgetPlayVisible(false);
    listWidgetCategoryVisible(true);
    ui->labelStreamIcon->setText("");

    ui->labelCategory->setText("");
    ui->labelCategoryItem->setText("");
    ui->homeIcon->setText("");
    ui->homeIcon->setPixmap(QPixmap(":/images/images/home.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    ui->homeIcon->setFixedSize(48, 48);

    ui->listWidgetCategory->installEventFilter(this);
    ui->listWidgetCategoryItems->installEventFilter(this);
    ui->listWidgetStreams->installEventFilter(this);

    fillCategories();
    QFileInfo fi(wApp->settings.fileName());
    database.setFavoriteSavePath(fi.absolutePath());
    //qDebug() << settings.fileName() << fi.absolutePath();

    wApp->setEvDevReceiver(this);
    QObject::connect(wApp, &webradioapplication::playerChangeTitle, this, &MainWindow::on_playerChangeTitle);
    QObject::connect(wApp, &webradioapplication::playerFinished, this, &MainWindow::on_playerFinished);
#ifdef __arm__
    // we receive RotaryEncode Signals as MouseEvent
    this->setMouseTracking(true);
#endif

    QDir favUsers = QFileInfo(wApp->settings.fileName()).absoluteDir();
    //qDebug() << favUsers.absolutePath();
    QFileInfoList entries = favUsers.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    foreach(QFileInfo entryInfo, entries)
    {
        FavoriteUser* user = new FavoriteUser();
        user->favoriteStreamDir = entryInfo.absoluteFilePath();
        user->userName = entryInfo.fileName();
        user->favoriteToggleButton = new QPushButton(QIcon(":/images/images/no_favorite.png"), user->userName, this);
        user->favoriteToggleButton->setObjectName("pushButtonToggleFavorite");
        user->favoriteToggleButton->setCheckable(true);
        QObject::connect(user->favoriteToggleButton, &QPushButton::toggled, this, &MainWindow::on_pushButtonToggleFavorite_toggled);
        this->favUsers.append(user);
        //qDebug() << entryInfo.absoluteFilePath();
        //qDebug() << entryInfo.fileName();
        ui->horizontalLayoutBottom->insertWidget(0, user->favoriteToggleButton);
    }

    this->screenSaveTimer.setSingleShot(true);
    QObject::connect(&this->screenSaveTimer, &QTimer::timeout, this, &MainWindow::on_screenSaveTimerTimedOut);
    this->clock = new Clock(screenRect, this);

    int lastStreamId = wApp->settings.value("lastStreamId", "-1").toInt();
    if (lastStreamId != -1)
    {
        playStream(lastStreamId);
        bStartWithLastStream = true;
    }
    else {
        QString lastStreamUrl = wApp->settings.value("lastStreamUrl", "1").toString();
        if (lastStreamUrl != "")
        {
            webradiostream* lastStream = new webradiostream();
            lastStream->id = -1;
            lastStream->type = webradiostream::StreamType::LocalFile;
            lastStream->streamURL = lastStreamUrl;
            playStream(lastStream);
            delete lastStream;
        }
    }

    connect(&this->playlistMgn, &playlist::changeStream, this, &MainWindow::on_playListChangeStream);
    QObject::connect(wApp, &webradioapplication::playerFinished, &this->playlistMgn, &playlist::streamFinished);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* ev = static_cast<QKeyEvent*>(event);
        if ((ev->key() == Qt::Key_Return) ||
            (ev->key() == Qt::Key_Plus)   || // PC testing: Volume Up
            (ev->key() == Qt::Key_Minus)  || // PC testing: Volume Down
            (ev->key() == Qt::Key_M))        // PC testing: Volume-Mute
        {
            this->keyPressEvent(ev);
            return true;
        }
        return false;
    }
    return false;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    this->clock->hide();
    this->resetScreensaveTimer();

    if (event->key() == Qt::Key_Left) // Button Back
    {
        QListWidget* widget = getActiveListWidget();
        if ((widget != Q_NULLPTR) &&
            (widget != ui->listWidgetCategory)) // we are already on the first page
        {
            listWidgetCategoryVisible(widget == ui->listWidgetCategoryItems);
            listWidgetCategoryItemsVisible(widget == ui->listWidgetStreams);
            listWidgetStreamsVisible(false);
            if (widget == ui->listWidgetStreams)
                ui->labelCategoryItem->setText("");
            if (widget == ui->listWidgetCategoryItems)
                ui->labelCategory->setText("");
        }
        else if (isWidgetPlayVisible())
        {
            on_labelCategoryItem_clicked();
        }
    }
    else if (event->key() == Qt::Key_MediaPrevious) // Button Prev
    {
        if (playlistMgn.isActive())
            playlistMgn.prev();
    }
    else if (event->key() == Qt::Key_MediaNext) // Button Next
    {
        if (playlistMgn.isActive())
            playlistMgn.next();
    }
    else if (event->key() == Qt::Key_Return) // Button of Select-RotationEncoder
    {
        QListWidget* widget = getActiveListWidget();
        if (widget != Q_NULLPTR)
        {
            QListWidgetItem* c = widget->currentItem();
            if (c != Q_NULLPTR)
            {
                QRect r = widget->visualItemRect(widget->currentItem());
                if ((r.width() > 0) && (r.height() > 0)) // bei w=0 und h=0 ist kein Item selektiert
                {
                    QPointF pos = r.center();
                    qApp->sendEvent(widget->viewport(), new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
                    qApp->sendEvent(widget->viewport(), new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
                }
            }
        }
        else if (this->isWidgetPlayVisible())
        {
            this->stopStream();
            if (this->isWidgetPlayVisible()) // only if the PlayerWidget is visible
                on_labelCategoryItem_clicked();
        }
    }
    else if (event->key() == Qt::Key_Menu)
    {
        goHome();
    }
    else if (event->key() == Qt::Key_Up)
    {
        goUp();
    }
    else if (event->key() == Qt::Key_Down)
    {
        goDown();
    }
    else if (event->key() == Qt::Key_Stop)
    {
        doStop();
    }
    else if ((event->key() == Qt::Key_Plus) || // only for testing on PC
             (event->key() == Qt::Key_Minus) ||
             (event->key() == Qt::Key_M)) // Volume-Mute
    {
        wApp->simulateEvDevKeyEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->clock->hide();

    if (event->pos().x() == 1) // select ListItem Up/Down
    {
      if (event->pos().y() == 1)
          goUp();
      else if (event->pos().y() == -1)
          goDown();
    }
}

void MainWindow::showProgressBar(bool visible)
{
    ui->widgetDownload->setVisible(visible);
}

void MainWindow::setProgressBarInfo(QUrl url)
{
    ui->labelDownloadInfo->setText("Download " + url.toString());
}

void MainWindow::setProgressBarState(qint64 value, qint64 max)
{
    ui->progressBarDownload->setMaximum(static_cast<int>(max));
    ui->progressBarDownload->setValue(static_cast<int>(value));
}

void MainWindow::fillCategories()
{
    ui->listWidgetCategory->setEnabled(false);
    ui->listWidgetCategory->clear();
    QStringList categories = database.getCategories();
    for (int i = 0; i< categories.count(); i++)
        ui->listWidgetCategory->addItem(new QListWidgetItem(categories.at(i)));
    ui->listWidgetCategory->setEnabled(true);
    ui->listWidgetCategory->setCurrentItem(Q_NULLPTR);
}

void MainWindow::downloadCategoryItems(QString category)
{
    listWidgetUpDownVisible(false);
    ui->listWidgetCategoryItems->clear();
    QStringList genre = database.getCategoryItems(category);
    for (int i = 0; i< genre.count(); i++)
        ui->listWidgetCategoryItems->addItem(new QListWidgetItem(genre.at(i)));
}

bool MainWindow::isWidgetPlayVisible()
{
    return ui->widgetPlay->isVisible();
}

void MainWindow::widgetPlayVisible(bool visible)
{
    if (visible)
    {
        this->startScreensaveTimer();
        listWidgetCategoryVisible(false);
        listWidgetCategoryItemsVisible(false);
        listWidgetStreamsVisible(false);
        navigationVisible(true);
        listWidgetUpDownVisible(false);
    }
    else {
        this->stopScreensaveTimer();
    }
    ui->widgetPlay->setVisible(visible);
}

void MainWindow::listWidgetUpDownVisible(bool visible)
{
    ui->pushButtonUp->setVisible(visible);
    ui->pushButtonDown->setVisible(visible);
}

void MainWindow::listWidgetVisible(QListWidget* listWidget, bool visible)
{
    listWidget->setVisible(visible);
    if (visible)
    {
        if ((listWidget->count() > 0) &&
            (listWidget->currentItem() == Q_NULLPTR))
        {
            listWidget->setCurrentItem(listWidget->item(0));
        }
        navigationVisible(true);
        listWidgetUpDownVisible(true);
    }
}

void MainWindow::listWidgetCategoryVisible(bool visible)
{
    listWidgetVisible(ui->listWidgetCategory, visible);
}

void MainWindow::listWidgetCategoryItemsVisible(bool visible)
{
    listWidgetVisible(ui->listWidgetCategoryItems, visible);
}

void MainWindow::listWidgetStreamsVisible(bool visible)
{
    listWidgetVisible(ui->listWidgetStreams, visible);
}

void MainWindow::navigationVisible(bool visible)
{
    ui->homeIcon->setVisible(visible);
    ui->labelCategory->setVisible(visible);
    ui->labelCategoryItem->setVisible(visible);
}

void MainWindow::on_listWidgetCategory_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetCategoryItems->setCurrentItem(Q_NULLPTR);
    ui->listWidgetCategoryItems->clear();
    ui->listWidgetStreams->setCurrentItem(Q_NULLPTR);
    ui->listWidgetStreams->clear();

    listWidgetCategoryVisible(false);
    ui->labelCategory->setText(item->text());
    downloadCategoryItems(item->text());    
    listWidgetCategoryItemsVisible(true);
}

void MainWindow::on_listWidgetCategoryItems_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetStreams->setCurrentItem(Q_NULLPTR);
    ui->listWidgetStreams->clear();
    ui->labelCategoryItem->setText(item->text());
    listWidgetCategoryItemsVisible(false);
    listWidgetUpDownVisible(false);
    webradiostreamlist* streams = database.getCategoryItemStreams(ui->labelCategory->text(), item->text());
    for (int i = 0; i< streams->count(); i++)
    {
        webstreamlistitem* item = new webstreamlistitem(streams->at(i)->name, streams->at(i));
        //item->setIcon()
        ui->listWidgetStreams->addItem(item);
    }    
    listWidgetStreamsVisible(true);
}

void MainWindow::on_listWidgetStreams_itemClicked(QListWidgetItem *item)
{
    if (item != Q_NULLPTR)
    {
        webstreamlistitem* streamitem = dynamic_cast<webstreamlistitem*>(item);
        if (streamitem != Q_NULLPTR)
        {            
            webradiostream::StreamType type = streamitem->_stream->type;
            if (type == webradiostream::StreamType::LocalFile)
            {
                playStream(streamitem->_stream);
            }
            else if (type == webradiostream::StreamType::LocalFolder)
            {
                webradiostreamlist* streams = database.getCategoryItemStreams(ui->labelCategory->text(), streamitem->_stream->streamURL);
                ui->listWidgetStreams->setCurrentItem(Q_NULLPTR);
                ui->listWidgetStreams->clear();

                for (int i = 0; i< streams->count(); i++)
                {
                    webstreamlistitem* item = new webstreamlistitem(streams->at(i)->name, streams->at(i));
                    //item->setIcon()
                    ui->listWidgetStreams->addItem(item);
                }
                listWidgetStreamsVisible(true);
            }
            else
            {
                webradiostream* stream = streamitem->_stream;
                if (stream->id > 0)
                    stream = new webradiostream(database.getStreamInfo(stream->id));

                if ((stream->type == webradiostream::StreamType::Stream) ||
                    (stream->type == webradiostream::StreamType::PlayList))
                {
                    playStream(stream);
                }
            }
        }
    }
}

void MainWindow::on_playListChangeStream(webradiostream* newStream)
{
    wApp->playerStop();
    wApp->playerPlay(newStream->streamURL);
    ui->labelStreamIcon->setPixmap(this->database.getStreamIcon(newStream));
}

void MainWindow::playStream(webradiostream* streamInfo)
{
    if (streamInfo->type == webradiostream::StreamType::PlayList)
    {        
        playlistMgn.playAll(streamInfo);
    }
    else
    {
        wApp->playerPlay(QUrl(streamInfo->streamURL));
        ui->labelStreamIcon->setPixmap(this->database.getStreamIcon(streamInfo));
    }
    qDebug() << streamInfo->streamURL;
    widgetPlayVisible(true);

    ui->labelChannelName->setText(streamInfo->name);
    ui->labelTitle->setText(streamInfo->currentTrack);
    this->currentPlayStream = new webradiostream(*streamInfo);    
    wApp->settings.setValue("lastStreamId", streamInfo->id);
    wApp->settings.setValue("lastStreamUrl", streamInfo->streamURL); // required from /etc/init.d/S41play

    setFavoriteToggleButton(streamInfo);
}

void MainWindow::playStream(int streamId)
{
    ui->labelTitle->setText("");

    webradiostream streamInfo = database.getStreamInfo(streamId);    
    playStream(new webradiostream(streamInfo));
}

void MainWindow::setFavoriteToggleButton(webradiostream *stream)
{
    foreach(FavoriteUser* user, this->favUsers)
    {
        setFavoriteToggleButton(user->favoriteToggleButton, user->userName, stream);
    }
}

void MainWindow::setFavoriteToggleButton(QPushButton* favButton, QString user, webradiostream* stream)
{
    bool isFavorite = database.isFavoriteStream(user, stream);
    favButton->setChecked(isFavorite);

    QIcon icon;
    if (isFavorite)
        icon.addPixmap(QPixmap(":/images/images/favorite.png"));
    else
        icon.addPixmap(QPixmap(":/images/images/no_favorite.png"));

    favButton->setIcon(icon);
}

void MainWindow::doStop()
{
    this->stopStream();
    on_labelCategoryItem_clicked();
}

void MainWindow::stopStream()
{
    this->screenSaveTimer.stop();
    wApp->playerStop();
    playlistMgn.stop();
    if (this->currentPlayStream != Q_NULLPTR)
        delete this->currentPlayStream;
    this->currentPlayStream = Q_NULLPTR;
}

void MainWindow::on_pushButtonStop_clicked()
{
    doStop();
}

void MainWindow::goHome()
{
    listWidgetCategoryItemsVisible(false);
    listWidgetStreamsVisible(false);
    widgetPlayVisible(false);
    listWidgetCategoryVisible(true);
    ui->labelCategoryItem->setText("");
    ui->labelCategory->setText("");
}

void MainWindow::on_homeIcon_clicked()
{
    goHome();
}

void MainWindow::on_labelCategory_clicked()
{
    listWidgetCategoryVisible(false);
    listWidgetStreamsVisible(false);
    widgetPlayVisible(false);
    listWidgetCategoryItemsVisible(true);
    ui->labelCategoryItem->setText("");
}

void MainWindow::on_labelCategoryItem_clicked()
{
    widgetPlayVisible(false);
    listWidgetStreamsVisible(!bStartWithLastStream);
    listWidgetCategoryVisible(bStartWithLastStream);

    if (bStartWithLastStream)
        bStartWithLastStream = false; // only at the first
}

QListWidget* MainWindow::getActiveListWidget()
{
    QListWidget* widget = Q_NULLPTR;
    if (ui->listWidgetCategory->isVisible())
        widget = ui->listWidgetCategory;
    else if (ui->listWidgetCategoryItems->isVisible())
        widget = ui->listWidgetCategoryItems;
    else if (ui->listWidgetStreams->isVisible())
        widget = ui->listWidgetStreams;

    return widget;
}

void MainWindow::on_pushButtonUp_clicked()
{
    goUp();
}

void MainWindow::goUp()
{
    QListWidget* widget = getActiveListWidget();
    if (widget != Q_NULLPTR)
    {
        //qDebug() << widget->currentRow();
        //qDebug() << widget->count();
        if (widget->currentRow() > 0)
        {
            qApp->postEvent(widget, new QKeyEvent(QEvent::Type::KeyPress, Qt::Key_Up, Qt::NoModifier));
            qApp->postEvent(widget, new QKeyEvent(QEvent::Type::KeyRelease, Qt::Key_Up, Qt::NoModifier));
        }
    }
}

void MainWindow::on_pushButtonDown_clicked()
{
    goDown();
}

void MainWindow::goDown()
{
    QListWidget* widget = getActiveListWidget();
    if (widget != Q_NULLPTR)
    {
        //qDebug() << widget->currentRow();
        //qDebug() << widget->count();
        if (widget->currentRow() < (widget->count() - 1))
        {
            qApp->postEvent(widget, new QKeyEvent(QEvent::Type::KeyPress, Qt::Key_Down, Qt::NoModifier));
            qApp->postEvent(widget, new QKeyEvent(QEvent::Type::KeyRelease, Qt::Key_Down, Qt::NoModifier));
        }
    }
}

void MainWindow::toggleFavorite_toggled(QPushButton* favButton, QString user, bool checked)
{
    if (this->currentPlayStream != Q_NULLPTR)
    {
        if (checked)
            database.setFavoriteStream(user, this->currentPlayStream);
        else
            database.removeFavorityStream(user, this->currentPlayStream);
        setFavoriteToggleButton(favButton, user, this->currentPlayStream);
    }
}

void MainWindow::on_pushButtonToggleFavorite_toggled(bool checked)
{
    QPushButton* favButton = dynamic_cast<QPushButton*>(QObject::sender());
    if (favButton != Q_NULLPTR)
        toggleFavorite_toggled(favButton, favButton->text(), checked);
}

void MainWindow::goEqualizer()
{
    wApp->showEqualizer();
}

void MainWindow::on_pushButtonEqualizer_clicked()
{
    goEqualizer();
}

void MainWindow::on_playerChangeTitle(QString title)
{
    ui->labelTitle->setText(title);
}

void MainWindow::on_playerFinished()
{
    // If a single audio file or stream ends then
    // we switch the screen back to stream select list.
    // If a stream ends and is a part of a playlist then
    // we switch not back to the stream select list, the
    // playWidget is still active
    if (!playlistMgn.isActive())
        on_labelCategoryItem_clicked();
}

void MainWindow::showClock()
{
    this->clock->windowBefore = wApp->getEvDevReciver();
    wApp->setEvDevReceiver(this->clock);
    this->clock->exec();
    wApp->setEvDevReceiver(this->clock->windowBefore);
    this->clock->windowBefore = Q_NULLPTR;
    this->startScreensaveTimer();
}

void MainWindow::startScreensaveTimer()
{
    this->screenSaveTimer.start(SCREENSAVER_TIME);
}

void MainWindow::stopScreensaveTimer()
{
    this->screenSaveTimer.stop();
}

void MainWindow::resetScreensaveTimer()
{
    this->screenSaveTimer.stop();
    this->screenSaveTimer.start(SCREENSAVER_TIME);
}

void MainWindow::on_screenSaveTimerTimedOut()
{
    this->showClock();
}

void MainWindow::on_pushButtonToggleOutputDevice_clicked()
{
    wApp->enableBluetoothAudio(!wApp->isBluetoothAudioEnabled());
    if (wApp->isBluetoothAudioEnabled())
    {
        qDebug() << "Enable Bluetooth";
        wApp->connectBluetoothDevice("88:D0:39:24:82:EC");
        QTime start = QTime::currentTime();
        bool bTimeout = true;
        while (QTime::currentTime() < start.addSecs(20))
        {
            qApp->processEvents();
            QThread::msleep(100);
            if (wApp->isBluetoothDeviceConnected("88:D0:39:24:82:EC"))
            {
                wApp->setAudioOutputDevice("jbl", "jbl", "JBL E65BTNC - A2DP");
                bTimeout = false;
                break;
            }
        }
        if (bTimeout)
        {
            qDebug() << "Timeout Bluetooth device connect";
            wApp->enableBluetoothAudio(false);
            wApp->setAudioOutputToDefaultDevice();
        }
    }
    else
    {
        qDebug() << "Disable Bluetooth";
        wApp->disconnectBluetoothDevice("88:D0:39:24:82:EC");
        wApp->enableBluetoothAudio(false);
        wApp->setAudioOutputToDefaultDevice();
    }
}
