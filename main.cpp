
#include <QtCore>
#include "mainwindow.h"
/*

const int defFrameWidth = 1280;
const int defFrameHeight = 1024;
using namespace cv;*/
QTextStream *outFile = 0;

void myMessageOutput(QtMsgType type, const char *msg)
{
    QString debugdate =
            QDateTime::currentDateTime().toString(QLatin1String("[dd.MM.yy hh:mm:ss.zzz] "));
    switch (type)
    {
    case QtDebugMsg:
        debugdate += "[D]";
        break;
    case QtWarningMsg:
        debugdate += "[W]";
        break;
    case QtCriticalMsg:
        debugdate += "[C]";
        break;
    case QtFatalMsg:
        debugdate += "[F]";
    }

    (*outFile) << debugdate << " " << msg << endl;
}

int main (int argc, char **argv)
{
    QFile *log = new QFile("debug.txt");
    if (log->open(QIODevice::WriteOnly))
    {
        outFile = new QTextStream (log);
        qInstallMsgHandler(myMessageOutput);
    }
    else
    {
        delete log;
        qDebug("Can't open log.txt file, all message will be output to debugger and console.");
    }

    QApplication a(argc, argv);
    MainWindow mainwindow;
    mainwindow.show();

    return a.exec();
  /*
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
    // return a.exec()*/
}
