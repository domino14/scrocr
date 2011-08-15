#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QtGui>
#include "camerahandler.h"
#include "fiducialdetector.h"

class ClickablePixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    ~ClickablePixmapItem()
    {
        qDebug() << "ClickablePixmapItem destroyed";
    }

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
signals:
    void clickedPt(QPoint point);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

signals:
    void updateLog(QString);
public slots:

private slots:
    void on_pushButtonGetPicture_clicked();
    void on_pushButtonLoadPicture_clicked();
    void on_pushButtonSavePicture_clicked();
    void on_pushButtonDetectFiducials_clicked();
    void on_pushButtonWarpImage_clicked();
    void clickedPicture(QPoint);
private:
    Ui::formMainWindow ui;
    QGraphicsScene scene;
    CameraHandler* cameraHandler;
    cv::Mat latestMatImage;
    ClickablePixmapItem* picItem;
    FiducialDetector* fd;
};



#endif // MAINWINDOW_H
