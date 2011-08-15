#include "mainwindow.h"
#include "utilities.h"
#include <vector>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    ui.setupUi(this);
    ui.graphicsView->setScene(&scene);


    cameraHandler = new CameraHandler(this);
   // connect(cameraHandler, SIGNAL(displayImage(QImage)), SLOT(displayImage(QImage)));
    connect(cameraHandler, SIGNAL(updateLog(QString)), ui.plainTextEdit, SLOT(appendPlainText(QString)));
    connect(this, SIGNAL(updateLog(QString)), ui.plainTextEdit, SLOT(appendPlainText(QString)));
    cameraHandler->init();
    picItem = new ClickablePixmapItem;
    picItem->setScale(0.5);
    scene.addItem(picItem);
    connect(picItem, SIGNAL(clickedPt(QPoint)), this, SLOT(clickedPicture(QPoint)));
    fd = new FiducialDetector(this);
    connect(fd, SIGNAL(updateLog(QString)), ui.plainTextEdit, SLOT(appendPlainText(QString)));
    fd->saveIntermediatePictures(true);

}

void MainWindow::on_pushButtonGetPicture_clicked()
{
    latestMatImage = cameraHandler->requestImage();
    IplImage img = IplImage(latestMatImage);
 //   QImage im("Picture 7.jpg");

    picItem->setPixmap(QPixmap::fromImage(Utilities::IplImageToQImage(&img)));
}

void MainWindow::on_pushButtonLoadPicture_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, "Select picture filename");
    if (f != "")
    {
        picItem->setPixmap(QPixmap(f));
    }
    latestMatImage = cv::imread(f.toAscii().constData());
    cv::Size size = latestMatImage.size();

    updateLog(QString("ImageMat size %1 %2").arg(size.width).arg(size.height));
}

void MainWindow::on_pushButtonSavePicture_clicked()
{
    QString f = QFileDialog::getSaveFileName(this, "Select save filename");
    if (f != "")
        picItem->pixmap().save(f);
}

void MainWindow::on_pushButtonDetectFiducials_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, "Select picture filename");
    if (f != "")
    {
        fd->detectFiducials(QStringList() << f);
    }
}

void MainWindow::on_pushButtonWarpImage_clicked()
{
    latestMatImage = cv::imread("test4.jpg");
    fd->detectFiducials(QStringList() << "test4.jpg");
    while (fd->isRunning()) ;
    double topLeftR, topLeftC, botLeftR, botLeftC, topRightR, topRightC, botRightR, botRightC;
    bool s1, s2, s3, s4;
    qDebug() << "prior to get";
    s1 = fd->getFiducialLocation(3, 0, topLeftR, topLeftC);
    s2 = fd->getFiducialLocation(5, 0, botLeftR, botLeftC);
    s3 = fd->getFiducialLocation(8, 0, topRightR, topRightC);
    s4 = fd->getFiducialLocation(10, 0, botRightR, botRightC);
    qDebug() << "after get";
    if (! (s1 && s2 && s3 && s4))
    {
        updateLog("Need all 4 fiducials on board!");
        return;
    }
    // try with perspective transform first
    /*
    cv::Point2f board_pts_ideal[4];    // top left, bottom left, top right, bottom right
    board_pts_ideal[0].x = -1; board_pts_ideal[0].y = 4;
    board_pts_ideal[1].x = -1; board_pts_ideal[1].y = 11;
    board_pts_ideal[2].x = 16; board_pts_ideal[2].y = 3;
    board_pts_ideal[3].x = 16; board_pts_ideal[3].y = 11;

    cv::Point2f board_pts_found[4];
    board_pts_found[0].x = topLeftC; board_pts_found[0].y = topLeftR;
    board_pts_found[1].x = botLeftC; board_pts_found[1].y = botLeftR;
    board_pts_found[2].x = topRightC; board_pts_found[2].y = topRightR;
    board_pts_found[3].x = botRightC; board_pts_found[3].y = botRightR;
    */

    cv::Point2f pts1[] = {cv::Point2f(150, 150.), cv::Point2f(150, 300.), cv::Point2f(350, 300.), cv::Point2f(350, 150.)};
    cv::Point2f pts2[] = {cv::Point2f(200, 200.), cv::Point2f(150, 300.), cv::Point2f(350, 300.), cv::Point2f(300, 200.)};


    //cv::Mat transformMat = cv::getPerspectiveTransform(board_pts_found, board_pts_ideal);
    cv::Mat transformMat = cv::getPerspectiveTransform(pts1, pts2);
    qDebug() << "transformation mat" << transformMat.size().width << transformMat.size().height;

    cv::Mat newImage = latestMatImage.clone();
    qDebug() << newImage.size().width << newImage.size().height;
    qDebug() << latestMatImage.size().width << latestMatImage.size().height;

    cv::warpPerspective(latestMatImage, newImage, transformMat, latestMatImage.size(), cv::INTER_LINEAR);
    qDebug() << "here 5";
    IplImage img = IplImage(newImage);
    qDebug() << "here 6";
    picItem->setPixmap(QPixmap::fromImage(Utilities::IplImageToQImage(&img)));
}

void MainWindow::clickedPicture(QPoint pt)
{
    updateLog(QString("clicked %1 %2").arg(pt.x()).arg(pt.y()));
    std::vector<cv::Point2f> corner(1, cv::Point2f(pt.x(), pt.y()));

    cv::Mat gray;
    cvtColor(latestMatImage, gray, CV_BGR2GRAY);


    cv::cornerSubPix(gray, corner, cv::Size(5, 5), cv::Size(-1, -1), cv::TermCriteria(2, 0, 0.001));
    qDebug() << "corner";
    updateLog(QString("subpixel pos %1 %2").arg(corner[0].x).arg(corner[0].y));
}
/**************************************************/
void ClickablePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << "clicked" << event->pos();
    emit clickedPt(event->pos().toPoint());
}


