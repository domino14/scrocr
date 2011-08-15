#include "camerahandler.h"
#include "utilities.h"
const int defFrameWidth = 1280;
const int defFrameHeight = 1024;
CameraHandler::CameraHandler(QObject *parent) :
    QObject(parent)
{
    cap = NULL;
    curWidth = 0;
    curHeight = 0;
}
CameraHandler::~CameraHandler()
{
    if (cap)
        delete cap;
}

void CameraHandler::init()
{
    cap = new cv::VideoCapture(1);
    if(!cap->isOpened())  // check if we succeeded
    {
        emit updateLog("No camera detected!");
        delete cap;
        cap = NULL;
    }

    bool s1 = cap->set(CV_CAP_PROP_FRAME_WIDTH, defFrameWidth);
    bool s2 = cap->set(CV_CAP_PROP_FRAME_HEIGHT, defFrameHeight);
    double h = cap->get(CV_CAP_PROP_FRAME_WIDTH);
    double v = cap->get(CV_CAP_PROP_FRAME_HEIGHT);
    emit updateLog(QString("Set resolution of camera to %1x%2").arg(h).arg(v));

    curWidth = (int)h;
    curHeight = (int)v;
            /*cvtColor(frame, edges, CV_BGR2GRAY);
             GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
             Canny(edges, edges, 0, 30, 3);
             imshow("edges", frame);
             if(waitKey(30) >= 0) break;*/


}

cv::Mat CameraHandler::requestImage()
{

    cv::Mat frame;
    (*cap) >> frame; // get a new frame from camera
    // repeat the grab - this is an ugly hack, but VideoCapture seems to buffer everything!
    (*cap) >> frame;
   /* IplImage iplImage(frame);



    return Utilities::IplImageToQImage(&iplImage);*/
    return frame;

}
