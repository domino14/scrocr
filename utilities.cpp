#include "utilities.h"

Utilities::Utilities()
{
}

QImage Utilities::IplImageToQImage(IplImage *image)
{

    // only works for rgb32 right now.
    int widthStep = image->widthStep;
    int width = image->width;
    int height = image->height;
    uchar* data = (uchar*) image->imageData;
    QImage qim(width, height, QImage::Format_RGB32);
    QRgb* outputbits = (QRgb*)qim.bits();

    int iplIndex, qtIndex;
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            iplIndex = j*widthStep + i*3;
            qtIndex = j*width + i;
            outputbits[qtIndex] = qRgb(data[iplIndex+2], data[iplIndex+1], data[iplIndex]);
        }
    }

    return qim;


}
