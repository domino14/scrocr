#ifndef FIDUCIALDETECTOR_H
#define FIDUCIALDETECTOR_H

#include <QObject>
#include <QThread>
#include <QStringList>

#include "cv.h"
#include "highgui.h"


#include "cvHomography.h"

class Contours
{
 /*
 *	 Copyright 2008 Kevin Atkinson
 *  email: kevin.atkinson@gmail.com
 *  blog:  methodart.blogspot.com
 *  web:   methodart.net
 */
public:

   Contours()
      : _contours(0),
        _contour_type(CV_RETR_TREE),
        _approx(CV_CHAIN_APPROX_SIMPLE),
        _cur_contour(0)
   {
      _stg = cvCreateMemStorage(0);
   }

   ~Contours()
   {
      if(_stg) cvReleaseMemStorage(&_stg);
   }

   Contours& operator()(int contour_type, int approx=CV_CHAIN_APPROX_SIMPLE)
   {
      _contour_type = contour_type;
      _approx = approx;
   }

   void Process(IplImage *img)
   {
      cvFindContours(img, _stg, &_contours, sizeof(CvContour), _contour_type, _approx);
   }

   CvSeq* Output() { return _contours; }

//   void Visit(ContourVisitor &visitor)
//   {
//      CvSeq *contour = _contours;
//      while(contour)
//      {
//         if(!visitor.visit(contour)) break;
//         contour = _get_next_depth_first(contour);
//      }
//   }

   void startIter()
   {
      _cur_contour = _contours;
   }

   CvSeq* nextContour()
   {
      if(!_cur_contour) return 0;

      CvSeq *next_contour = _get_next_depth_first(_cur_contour);
      CvSeq *ret_contour = _cur_contour;
      _cur_contour = next_contour;
      return ret_contour;
   }

   CvMemStorage* storage() { return _stg; }

   void clear_mem()
   {
      if(_stg) cvReleaseMemStorage(&_stg);
      _stg = cvCreateMemStorage();
   }

protected:

   CvSeq* _get_next_depth_first(CvSeq *seq)
   {
      if(seq->v_next) return seq->v_next;

      for(;;)
      {
         if(seq->h_next) return seq->h_next;

         if(seq->v_prev)
            seq = seq->v_prev;
         else
            return 0;
      }
   }

   CvSeq *_cur_contour;

   int _contour_type, _approx;
   CvMemStorage *_stg;
   CvSeq *_contours;
};

struct Fiducial
{
    int code;
    double row, column; // x y location of relevant corner
    int camera; // which camera took this pic?
    int thresholdLevel; // black-white threshold level that this fiducial was detected at
};

class FiducialDetector : public QThread
{

Q_OBJECT
public:
    FiducialDetector(QObject* parent);
    void detectFiducials(QStringList filenames);
    bool getFiducialLocation(int code, int filenameIndex, double &row, double &column);
private:
    void run();
    QStringList filenamesToScan;

    void scanFilenameForFiducials(QString, int);
    bool saveIntPics;
    QList <Fiducial> identifiedFiducials;
signals:
    void fiducialsDetectedSuccessfully(bool);
    void updateLog(QString);
public slots:
    void saveIntermediatePictures(bool);
};

#endif // FIDUCIALDETECTOR_H
