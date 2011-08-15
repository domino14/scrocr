#ifndef UTILITIES_H
#define UTILITIES_H

#include <QImage>
#include "cv.h"
class Utilities
{
public:
    Utilities();
    static QImage IplImageToQImage(IplImage*);
};

#endif // UTILITIES_H
