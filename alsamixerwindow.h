#ifndef ALSAMIXERWINDOW_H
#define ALSAMIXERWINDOW_H

#include <QDialog>
#include <QKeyEvent>
#include <QList>

#include "alsamixer.h"

class alsamixerwindow : public QDialog
{
    Q_OBJECT

    public:
        alsamixerwindow(QString alsaDevice, QWidget *parent = Q_NULLPTR);
        ~alsamixerwindow() Q_DECL_OVERRIDE;

    protected:
        void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
        void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

    private:
        alsamixer mixer;
        QLayout *buildCentralWidget();
        void selectWidget(QWidget* widget);
        void editWidget(QWidget* widget);
        void deselectWidget(QWidget* widget);
        bool handleKeyPressEvent(QKeyEvent* event);
        bool handleFocusInOut(QObject *object, QFocusEvent* event);
        bool handleLeftEvent();
        bool handleRightEvent();

        void nextCtrl();
        void prevCtrl();

        int activeCtrlIdx;
        bool selected;
        QList<QWidget*> ctrls;
};

#endif // ALSAMIXERWINDOW_H
