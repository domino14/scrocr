#
# FILE:   gen_target.py
# copyright Kevin Atkinson, 2008
# kevin.atkinson@gmail.com
# http://methodart.blogspot.com
#
# You have permission to use this code to do whatever you wish, with
# the sole proviso that if you redistribute this source file, or
# modifications of this source file, you retain this notice.
#
# The other part of the license is a request, rather than a requirement:
# if you use it for something interesting, drop me a line (kevin.atkinson@gmail.com)
# to let me know about it.  After all, the only reward in giving your code 
# away is knowing that someone somewhere is finding it useful.
#
# Finally, this is fairly new code, so it's entirely probable that you may discover
# some bugs.  If you do, please let me know about them so I can fix them.
#

import sys
import cairo
from math import pi

squaresize = 1
filename = "fiducials.pdf"

def do_poly(cr, pts):
    cr.move_to(*(pts[0]))
    for pt in pts[1:]:
        cr.line_to(*pt)
    
# draws a code square in (0,0), (1,1)
# n is dimension of square; code area is (n-1)x(n-1)
def draw_code(cr, code_val, n):
    assert code_val < (2**((n-1)*(n-2)-1))
    print "Drawing code", code_val
    
    incr = 1./(n)
    do_poly(cr, [(0,0),(0,1),(1,1),(1,1-incr),(incr,1-incr),(incr,0)])  # draws the L polygon
    cr.fill()                                                           # and fills it
    ones_count = 0
    bit = 0
    code_val <<= 1 # want the first bit to be clear
    for j in range(n-2):
        for i in range(n-1):
            if (code_val & (1<<bit)):
                cr.rectangle((i+1)*incr, j*incr, incr, incr)        # confusing, j is x and i is y
                cr.fill()
                ones_count += 1
            bit += 1
    
    #checksum
    for i in range(n-2):
        if ones_count & (1<<i):
            cr.rectangle((i+1)*incr, (n-2)*incr, incr, incr)
            cr.fill()
    
            
def draw_cb_square(cr, code, n, border=.2):
    cr.set_source_rgb(0,0,0)
    cr.rectangle(0,0,1,1)       # draws filled black square 0,0 -> 1, 1
    cr.fill()
    cr.save()
    cr.translate(border, border) 
    cr.scale(1-2*border, 1-2*border)
    cr.set_source_rgb(1,1,1)
    cr.rectangle(0,0,1,1)
    cr.fill()                   # draws filled white square inside rectangle
    cr.restore()
    cr.save()
    cr.translate(1.5*border, 1.5*border)
    cr.scale(1-3*border, 1-3*border)
    cr.set_source_rgb(0,0,0)    # prepare to draw code
    draw_code(cr, code, n)
    cr.restore()
    cr.rectangle(-border, -border, border, border)  # draws little black squares in corners
    cr.rectangle(1, -border, border, border)
    cr.rectangle(1, 1, border, border)
    cr.rectangle(-border, 1, border, border)
    
    
           
##if __name__=='__main__':
##    w,h = (width*squaresize + 2*border + .4*squaresize)*72., (height*squaresize + 2*border + .4*squaresize)*72. 
##    surface = cairo.PDFSurface(filename, w, h)
##    cr = cairo.Context(surface)
##    cr.translate(border*72, border*72)
##    cr.scale(squaresize*72, squaresize*72)
##    draw_cb(cr, width, height)

def draw_fiducials(code1, code2, code3, code4):
    # draws 4 fiducials. codes must range from 0 to 31
    # this is optimized to print out 5 rows of fiducials on STAPLES shipping label paper (with 4 by 2 labels!)
    # row is 0 thru 4

    w, h = 8.5*72., 11.*72.
    surface = cairo.PDFSurface(filename, w, h)
    cr = cairo.Context(surface)
    cr.scale(squaresize*72, squaresize*72)
    blackborder = 0.2
    cr.translate(0.5, 0.75)
    cr.translate(blackborder, blackborder)
    for row in range(5):
        

        cr.save()
        draw_cb_square(cr, code1, 4, blackborder)
        cr.fill()
        cr.restore()

        cr.save()
        cr.translate(2, 0)
        draw_cb_square(cr, code2, 4, blackborder)
        cr.fill()
        cr.restore()

        cr.save()
        cr.translate(4.125, 0)
        draw_cb_square(cr, code3, 4, blackborder)
        cr.fill()
        cr.restore()

        cr.save()
        cr.translate(6, 0)
        draw_cb_square(cr, code4, 4, blackborder)
        cr.fill()
        cr.restore()

        cr.translate(0, 2)
    
    
##    w, h = (2*squaresize + 2*border + .4*squaresize + 1*squaresize)*72., (2*squaresize + 2*border + .4*squaresize + 1*squaresize)*72.
##    surface = cairo.PDFSurface(filename, w, h)
##    cr = cairo.Context(surface)
##    cr.translate(border*72, border*72)
##    cr.scale(squaresize*72, squaresize*72)  # i guess cairo assumes 72 dpi?
##    
##    blackborder = 0.2
##    cr.translate(blackborder, blackborder)
##
##    # we are using squares with a code of size 4 -> 2**((4-1)*(4-2)-1) = 32 codes. Use 3 bits for width and 2 bits for height.
##    # max number of fiducials is 32.
##    for i in range(0, 3, 2):
##        for j in range(i%2,3,2):
##            cr.save()
##            cr.translate(j, 3-i-1)
##            draw_cb_square(cr, (j<<2)|i, 4, blackborder)
##            cr.fill()
##            cr.restore()

