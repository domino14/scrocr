#include "fiducialdetector.h"
#include <QtDebug>
#include "utilities.h"
// SOME OF THIS CODE -- copyright Kevin Atkinson, 2008
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


#define DIST_PTS(p1, p2)  sqrtf((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y))
#define DIST2_PTS(p1, p2) ((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y))
#define IS_NEAR(p1, p2, tol) (DIST2_PTS(p1, p2) < tol*tol)
#define IN_IMG(img, x, y) ((x)>0 && (x)<img->width && (y)>0 && (y)<img->height)
#define GET_PIXEL_8U(img, x, y) (*(unsigned char*)(img->imageData + (y)*img->widthStep + (x)))




#include <algorithm>

QList <int> thresholds = QList <int>() << 75 << 150 << 200;


CvSeq* quad_approx(CvSeq* contour, int low_val, int high_val)
{
    for(int i = low_val; i <= high_val; i++)
    {
        CvSeq *approx_contour = cvApproxPoly(contour, sizeof(CvContour), NULL, CV_POLY_APPROX_DP, i);
        if(approx_contour->total == 4)
        {
            return approx_contour;
        }
    }

    return NULL;
}

CvPoint firstPt(CvSeq *seq)
{
    CvPoint ret;
    if(CV_SEQ_ELTYPE(seq) == CV_32FC2)
    {
        CvPoint2D32f *pt = (CvPoint2D32f*)CV_GET_SEQ_ELEM(CvPoint2D32f, seq, 0);
        ret.x = (int)pt->x, ret.y = (int)pt->y;
    }
    else if(CV_SEQ_ELTYPE(seq) == CV_32SC2)
    {
        CvPoint *pt = (CvPoint*)CV_GET_SEQ_ELEM(CvPoint, seq, 0);
        ret.x = (int)pt->x, ret.y = (int)pt->y;
    }
    return ret;
}

void seq2pts(CvSeq *seq, CvPoint2D32f pts[], int max_pts)
{
    if(CV_SEQ_ELTYPE(seq) == CV_32FC2)
    {
        for(int i = 0; i < seq->total && i < max_pts; i++)
        {
            CvPoint2D32f *pt = (CvPoint2D32f*)CV_GET_SEQ_ELEM(CvPoint2D32f, seq, i);
            pts[i].x = pt->x, pts[i].y = pt->y;
        }
    }
    else if(CV_SEQ_ELTYPE(seq) == CV_32SC2)
    {
        for(int i = 0; i < seq->total && i < max_pts; i++)
        {
            CvPoint *pt = (CvPoint*)CV_GET_SEQ_ELEM(CvPoint, seq, i);
            pts[i].x = (float)pt->x, pts[i].y = (float)pt->y;
        }
    }
}

int find_closest_pt(CvPoint2D32f *pts, int n, CvPoint2D32f the_other_pt)
{
    int idx_of_closest = 0;
    float min_dist = DIST_PTS(pts[0], the_other_pt);
    for(int i = 1; i < n; i++)
    {
        float dist = DIST_PTS(pts[i], the_other_pt);
        if(dist < min_dist)
        {
            min_dist = dist;
            idx_of_closest = i;
        }
    }
    return idx_of_closest;
}


float get_pixel_bilerp(IplImage *img, float x, float y)
{
    int x_low = (int)floorf(x);
    float x_rem = x - x_low;
    int y_low = (int)floorf(y);
    float y_rem = y - y_low;
    float bottom_left =  GET_PIXEL_8U(img, x_low, y_low);
    float bottom_right = GET_PIXEL_8U(img, x_low+1, y_low);
    float top_left =     GET_PIXEL_8U(img, x_low, y_low+1);
    float top_right =    GET_PIXEL_8U(img, x_low+1, y_low+1);

    float bottom = bottom_left + x_rem*(bottom_right - bottom_left);
    float top = top_left + x_rem*(top_right - top_left);
    return bottom + y_rem*(top - bottom);
}

// walk through image, sampling code vals
void get_raw_code(IplImage *img, CvHomography &h, float raw_code[16])
{
    /* this code modified to read a 4x4 instead of a 5x5 square */
    int code_idx = 0;
    float incr = .1f;   // used to be .08f. should be 0.4/codeSquareSize, which used to be a 5x5 code square but now is 4x4.
    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < 4; i++)
        {
            CvPoint2D32f pt = cvPoint2D32f(.3f+(i+.5f)*incr, .3f+(j+.5f)*incr);
            pt = h.apply(pt);
            raw_code[code_idx++] = get_pixel_bilerp(img, pt.x, pt.y);
            //qDebug() << pt.x << pt.y << raw_code[code_idx-1];
        }
    }
    //qDebug() << "done";
}

// looks at the extracted raw code to determine
// 1) orientation and 2) (x, y) embedded in code
bool fixup_code(float raw_code[16], int &rot_idx, int &code)
{
    /* this code modified to read a 4x4 instead of a 5x5 square */
    float min_val = raw_code[0], max_val = raw_code[0];
    for(int i = 0; i < 16; i++)
    {
        if(raw_code[i] < min_val)
        {
            min_val = raw_code[i];
        }
        else if(raw_code[i] > max_val)
        {
            max_val = raw_code[i];
        }
    }

    float median = (min_val + max_val)/2;

    int code_bits[16];
    for(int i = 0; i < 16; i++)
    {
        code_bits[i] = (raw_code[i] > median) ? 0 : 1;          // white is 0, black is 1
    }

    QString codeBits;
    for (int i = 0; i < 16; i++)
        codeBits += QString::number(code_bits[i]);


    // need to find the two consecutive sides that are all dark
    bool side_solid[4];

    memset(side_solid, true, sizeof(side_solid));
    for(int i = 0; i < 4; i++)
    {
        if(!code_bits[i]) side_solid[0] = false;
        if(!code_bits[4*(i+1) - 1]) side_solid[1] = false;
        if(!code_bits[12+i]) side_solid[2] = false;
        if(!code_bits[4*i]) side_solid[3] = false;
    }

    // looking for two consecutive true values. side_solid is true if side is WHITE.
    if(side_solid[0] && side_solid[1] && !side_solid[2] && !side_solid[3])
    {
        rot_idx = 3;
    }
    else if(!side_solid[0] && side_solid[1] && side_solid[2] && !side_solid[3])
    {
        rot_idx = 2;
    }
    else if(!side_solid[0] && !side_solid[1] && side_solid[2] && side_solid[3])
    {
        rot_idx = 1;
    }
    else if(side_solid[0] && !side_solid[1] && !side_solid[2] && side_solid[3])
    {
        rot_idx = 0;
    }
    else return false;  // added by cesar to prevent crash on false positives

    static int code_bit_indices[][7] = { { 9, 5, 14, 10, 6, 15, 11},
                                         { 10, 9, 7, 6, 5, 3, 2},
                                         { 6, 10, 1, 5, 9, 0, 4},
                                         { 5, 6, 8, 9, 10, 12, 13}};


    // get the code:
    code = 0;
    int ones_count = 0;
    for(int i = 0; i < 7; i++)
    {
        code <<= 1;
        code |= code_bits[code_bit_indices[rot_idx][i]];
        if((i > 1) && (code & 0x01))
        {
            ones_count++;
        }
    }
    int checksum = code >> 5;
    code = code & 0x1f; // 11111 <- just take last 5 bits
    return ((ones_count & 0x03) == (checksum)); // last 2 bits is a checksum
}

bool operator<(const CvPoint &lhs, const CvPoint &rhs)
{
    if(lhs.x > rhs.x) return false;
    if(lhs.x < rhs.x) return true;

    // get here if x vals are equal
    return lhs.y < rhs.y;
}


FiducialDetector::FiducialDetector(QObject* parent) : QThread(parent)
{
    saveIntPics = false;
}

void FiducialDetector::detectFiducials(QStringList filenamesToScan)
{
    if (!isRunning())
    {
        this->filenamesToScan = filenamesToScan;
        identifiedFiducials.clear();
        start();
    }
    else
    {
        updateLog("Fiducial detector is currently busy. Please wait until it finishes.");
    }
}



void FiducialDetector::run()
{
    for (int i = 0; i < filenamesToScan.size(); i++)
    {
        updateLog("Scanning " + filenamesToScan.at(i) + " for fiducials.");
        scanFilenameForFiducials(filenamesToScan.at(i), i);
    }
    updateLog("Fiducial scanning is done!");


    /* */

    emit fiducialsDetectedSuccessfully(true);
}

void FiducialDetector::scanFilenameForFiducials(QString filename, int cameraLocation)
{
    /* cameraLocation is the index in the list of filenames of this particular filename.
       we are going to be pushing in filenames in the same order that pictures are taken, so
       for example the first picture taken will have camera location 0, and since we keep track of the order
       in which pics are taken, then we can associate the extrinsic camera matrices accordingly  */
    qDebug() << "Scanning" << filename << cameraLocation;

    CvPoint2D32f square_pts[4];
    square_pts[0].x = 0; square_pts[0].y = 0;
    square_pts[1].x = 0; square_pts[1].y = 1;
    square_pts[2].x = 1; square_pts[2].y = 1;
    square_pts[3].x = 1; square_pts[3].y = 0;

    CvPoint2D32f inner_square_pts[4];
    inner_square_pts[0].x = .2f; inner_square_pts[0].y = .2f;
    inner_square_pts[1].x = .2f; inner_square_pts[1].y = .8f;
    inner_square_pts[2].x = .8f; inner_square_pts[2].y = .8f;
    inner_square_pts[3].x = .8f; inner_square_pts[3].y = .2f;



    IplImage* myImage = cvLoadImage(filename.toAscii().constData());
    IplImage* dilatedImage = cvCreateImage(cvGetSize(myImage), IPL_DEPTH_8U, 1);
    IplImage* threshedImage = cvCreateImage(cvGetSize(myImage), IPL_DEPTH_8U, 1);
    IplImage* grayImage = cvCreateImage(cvGetSize(myImage), IPL_DEPTH_8U, 1);

    QImage qtImage = Utilities::IplImageToQImage(myImage);

    cvCvtColor(myImage, grayImage, CV_BGR2GRAY);

    if (saveIntPics) cvSaveImage(QByteArray("gray" + QByteArray::number(cameraLocation) + ".png"), grayImage);

    cvDilate(grayImage, dilatedImage);

    if (saveIntPics) cvSaveImage(QByteArray("dilated" + QByteArray::number(cameraLocation) + ".jpg"), dilatedImage);


    /* try three different thresholds, for lighting purposes */

    for (int t = 0; t < thresholds.size(); t++)
    {

        cvThreshold(dilatedImage, threshedImage, thresholds.at(t), 255, CV_THRESH_BINARY);

        if (saveIntPics)
            cvSaveImage(QByteArray("threshed" + QByteArray::number(cameraLocation) + "_" + QByteArray::number(thresholds.at(t)) + ".jpg"), threshedImage);


        /* find contours */

        Contours contours;
        contours.Process(threshedImage);

        CvHomography h, h_inv;

        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0);

        // find the special square
        contours.startIter();

        while (CvSeq* contour = contours.nextContour())
        {
            if (!CV_IS_SEQ_HOLE(contour)) continue;


            // look for large, rectangular contours
            CvSeq *quad_contour = quad_approx(contour, 2, 4);
            if(!(quad_contour &&  // well-approximated by a quad
                 contour->v_next && // has subcontours
                 contour->v_next->h_next == 0 && // in fact has one subcontour
                 cvCheckContourConvexity(quad_contour) && // is convex
                 cvContourArea(quad_contour) > 60)) // TODO: what's a good minimum area
                continue;

            // is sub-contour well approximated by a quad?
            CvSeq *quad_sub_contour = quad_approx(contour->v_next, 2, 4);
            if(!quad_sub_contour) continue;

            CvPoint2D32f quad_contour_pts[4];
            int invSeq1 = 0, invSeq2 = 0;
            if(0 < cvContourArea(quad_contour)) // fix winding order if nec
            {
                cvSeqInvert(quad_contour);
                invSeq1 = 1;
            }

            if(0 < cvContourArea(quad_sub_contour)) // same for quad sub-contour
            {
                cvSeqInvert(quad_sub_contour);
                invSeq2 = 1;
            }

            // refine corners of big quad
            seq2pts(quad_contour, quad_contour_pts, 4);
            cvFindCornerSubPix(grayImage, quad_contour_pts, 4, cvSize(4,4), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 50, 1e-3));

            // find the homography and it's inverse (h maps image position to ideal position)
            h.estimate(quad_contour_pts, square_pts, 4);
            h_inv.estimate(square_pts, quad_contour_pts, 4);




            // apply h to the quad sub-contour:
            CvSeq *rectified_quad_subcontour = h.apply(quad_sub_contour);

            CvPoint2D32f rectified_quad_subcontour_pts[4];
            seq2pts(rectified_quad_subcontour, rectified_quad_subcontour_pts, 4);

            // reorder sub-contour quad pts so that the first pt is closest to (0,0)
            int idx_of_closest = find_closest_pt(rectified_quad_subcontour_pts, 4, cvPoint2D32f(0,0));
            std::rotate(rectified_quad_subcontour_pts, &rectified_quad_subcontour_pts[idx_of_closest], &rectified_quad_subcontour_pts[3]+1);

            // test that the quad subcontour pts are close to where they should be:
            if(!IS_NEAR(rectified_quad_subcontour_pts[0], inner_square_pts[0], .1)) continue;
            if(!IS_NEAR(rectified_quad_subcontour_pts[1], inner_square_pts[1], .1)) continue;
            if(!IS_NEAR(rectified_quad_subcontour_pts[2], inner_square_pts[2], .1)) continue;
            if(!IS_NEAR(rectified_quad_subcontour_pts[3], inner_square_pts[3], .1)) continue;

            if (saveIntPics) cvDrawContours(myImage, quad_contour, CV_RGB(0, 0, 255), CV_RGB(255, 0, 0), 0, 2);

            // looks like we found a square; lets try to read a code
            // (keep in mind we still don't know the orientation of the square)
            float raw_code[16];
            get_raw_code(grayImage, h_inv, raw_code);



            int rot_idx;
            int code;
            // fixup_code returns false if the checksum is wrong
            bool error = false;
            if(!fixup_code(raw_code, rot_idx, code))
            {
                error = true;
                continue;
            }


            // now rotate the quad pts so that the origin is near the the corner of the fiducial "L"
            //qDebug() << "rot_idx" << rot_idx;
            std::rotate(quad_contour_pts, quad_contour_pts + rot_idx, quad_contour_pts + 4);
            // recompute the homography
            h.estimate(quad_contour_pts, square_pts, 4);
            h_inv.estimate(square_pts, quad_contour_pts, 4);


            // which square did we find?
            //        int x_pos = code>>5;
            //        int y_pos = (code&0x1f);

            //        // okay, we've found a square and its fiducial; let's add any refined points
            //        // not added to our tally
            //        for(int i = 0; i < 4; i++)
            //        {
            //            CvPoint obj_pt = cvPoint(x_pos + (int)square_pts[i].x, y_pos + (int)square_pts[i].y);
            //            if(found_pts_map.end() == found_pts_map.find(obj_pt))
            //            {
            //                found_pts_map[obj_pt] = quad_contour_pts[i];
            //            }
            //        }

            if (saveIntPics)
            {
                int shift = 8;
                cvCircle(myImage, cvPoint((int)(quad_contour_pts[0].x*(1<<shift)), (int)(quad_contour_pts[0].y*(1<<shift))),
                         4*(1<<shift), error ? CV_RGB(255, 0, 0) : CV_RGB(0,255, 0), 1, CV_AA, shift);
                cvPutText(myImage, QByteArray(QByteArray::number(code) + "_" + QByteArray::number(thresholds.at(t))),
                          cvPoint(quad_contour_pts[0].x, quad_contour_pts[0].y), &font,
                          error ? CV_RGB(255, 0, 0) : CV_RGB(255,0, 0));
            }
            Fiducial f;
            f.camera = cameraLocation;
            f.code = code;
            f.thresholdLevel = thresholds.at(t);
            f.column = quad_contour_pts[0].x;
            f.row = quad_contour_pts[0].y;

            identifiedFiducials << f;
        }
    }

    if (saveIntPics) cvSaveImage("output.png", myImage);

    cvReleaseImage(&myImage);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&threshedImage);
    cvReleaseImage(&dilatedImage);

    qDebug() << "done scanning!";
}

bool FiducialDetector::getFiducialLocation(int code, int filenameIndex, double &row, double &column)
{
   // qDebug() << "Calling getFiducialLocation" << code << filenameIndex;
    row = 0;
    column = 0;
    int numFids = 0;
    for (int i = 0; i < identifiedFiducials.size(); i++)
    {
        if (filenameIndex == identifiedFiducials.at(i).camera && code == identifiedFiducials.at(i).code)
        {
            numFids++;
            row += identifiedFiducials.at(i).row;
            column += identifiedFiducials.at(i).column;
        }
    }

    if (numFids > 0)
    {
        row /= (double)numFids;
        column /= (double)numFids;

      //  qDebug () << "..Found" << numFids << "matching fiducials with code" << code << "and filename Index" << filenameIndex << row << column;

        return true;
    }
    else
    {
       // qDebug() << "..NO fiducials found with code" << code << "and filename Index" << filenameIndex;
        return false;
    }
}

void FiducialDetector::saveIntermediatePictures(bool s)
{
    saveIntPics = s;
}
