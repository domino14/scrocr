#ifndef _CVHOMOGRAPHY_H
#define _CVHOMOGRAPHY_H

#include "cv.h"
#include <QVector>
//
// FILE:   cvHomography.h
// copyright Kevin Atkinson, 2008
// kevin.atkinson@gmail.com
// http://methodart.blogspot.com
//
// You have permission to use this code to do whatever you wish, with
// the sole proviso that if you redistribute this source file, or
// modifications of this source file, you retain this notice.
//
// The other part of the license is a request, rather than a requirement:
// if you use it for something interesting, drop me a line (kevin.atkinson@gmail.com)
// to let me know about it.  After all, the only reward in giving your code
// away is knowing that someone somewhere is finding it useful.
//
// Finally, this is fairly new code, so it's entirely probable that you may discover
// some bugs.  If you do, please let me know about them so I can fix them.

struct CvHomography
{
   CvHomography()
      : _homog_found(false)
   {
      h_mat = cvMat(3, 3, CV_32F, h_dat);
   }

   void estimate(CvPoint2D32f *src_pts, CvPoint2D32f *dest_pts, int n)
   {
      CvMat src_mat = cvMat(n, 1, CV_32FC2, src_pts);
      CvMat dst_mat = cvMat(n, 1, CV_32FC2, dest_pts);
      cvFindHomography(&src_mat, &dst_mat, &h_mat);
      _homog_found = true;
   }

   void estimate(CvPoint *src_pts_, CvPoint2D32f *dest_pts, int n)
   {
      QVector<CvPoint2D32f> src_pts(n);
      for(int i = 0; i < n; i++)
      {
         src_pts[i].x = (float)(src_pts_[i].x);
         src_pts[i].y = (float)(src_pts_[i].y);
      }
      CvMat src_mat = cvMat(n, 1, CV_32FC2, &src_pts[0]);
      CvMat dst_mat = cvMat(n, 1, CV_32FC2, dest_pts);
      cvFindHomography(&src_mat, &dst_mat, &h_mat);
      _homog_found = true;
   }

   void estimate(CvSeq *src_pts_, CvPoint2D32f *dest_pts)
   {
      QVector<CvPoint2D32f> src_pts(src_pts_->total);
      for(int i = 0; i < src_pts_->total; i++)
      {
         CvPoint *pt = (CvPoint*)CV_GET_SEQ_ELEM(CvPoint, src_pts_, i);
         src_pts[i] = cvPoint2D32f(pt->x, pt->y);
      }
      estimate(&src_pts[0], dest_pts, src_pts_->total);
   }


   CvPoint2D32f apply(CvPoint2D32f &pt)
   {
      assert(_homog_found);

      // TODO: is returned h row-major?
      float denom = h_dat[6]*pt.x + h_dat[7]*pt.y + h_dat[8];
      return cvPoint2D32f((h_dat[0]*pt.x + h_dat[1]*pt.y + h_dat[2])/denom, (h_dat[3]*pt.x + h_dat[4]*pt.y + h_dat[5])/denom); 
   }

   CvPoint2D32f apply(CvPoint &pt)
   {
      assert(_homog_found);

      // TODO: is returned h row-major?
      float denom = h_dat[6]*pt.x + h_dat[7]*pt.y + h_dat[8];
      return cvPoint2D32f((h_dat[0]*pt.x + h_dat[1]*pt.y + h_dat[2])/denom, (h_dat[3]*pt.x + h_dat[4]*pt.y + h_dat[5])/denom); 
   }


   CvPoint2D32f apply(CvPoint2D32f *pts, CvPoint2D32f *mapped_pts, int n)
   {
      assert(_homog_found);

      for(int i = 0; i < n; i++)
      {
         float denom = h_dat[6]*pts[i].x + h_dat[7]*pts[i].y + h_dat[8];
         mapped_pts[i].x = (h_dat[0]*pts[i].x + h_dat[1]*pts[i].y + h_dat[2])/denom;
         mapped_pts[i].y = (h_dat[3]*pts[i].x + h_dat[4]*pts[i].y + h_dat[5])/denom;
      }
   }

   // returns a new sequence created using the passed-in seq's storage
   CvSeq* apply(CvSeq* seq)
   {
      assert(_homog_found);

      CvSeq* mapped_seq = cvCreateSeq(CV_SEQ_POLYGON | CV_32FC2/*CV_32FC2*/, sizeof(CvContour), sizeof(CvPoint2D32f), seq->storage);
      CvSeqWriter writer;
      cvStartAppendToSeq(mapped_seq, &writer);
      for(int i = 0; i < seq->total; i++)
      {
         CvPoint *pt = (CvPoint*)CV_GET_SEQ_ELEM(CvPoint, seq, i);
         CvPoint2D32f mapped_pt = apply(*pt);
         CV_WRITE_SEQ_ELEM(mapped_pt, writer);
      }
      cvEndWriteSeq(&writer);
      return mapped_seq;
   }

   float h_dat[9];
   CvMat h_mat;

   bool _homog_found;
};

#endif //_CVHOMOGRAPHY_H
