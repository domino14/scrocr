#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <QObject>
#include "cv.h"
#include "highgui.h"
#include <QImage>
class CameraHandler : public QObject
{
    Q_OBJECT
public:
    explicit CameraHandler(QObject *parent = 0);
    void init();
    ~CameraHandler();
    cv::Mat requestImage();
signals:
    void updateLog(QString);
    void displayImage(QImage);
public slots:

private:
    cv::VideoCapture *cap;
    int curWidth;
    int curHeight;
};

#endif // CAMERAHANDLER_H
