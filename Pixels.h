/*
 * Pixels. Graphics library for TFT displays.
 *
 * Copyright (C) 2012-2013  Igor Repinetski
 *
 * The code is written in C/C++ for Arduino and can be easily ported to any microcontroller by rewritting the low level pin access functions.
 *
 * Text output methods of the library rely on Pixelmeister's font data format. See: http://pd4ml.com/pixelmeister
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * Commercial use of the library is possible for licensees of "Pixelmeister (for industry)" product.
 *
 * This library includes some code portions and algoritmic ideas derived from works of
 * - Andreas Schiffler -- aschiffler at ferzkopp dot net (SDL_gfx Project)
 * - K. Townsend http://microBuilder.eu (lpc1343codebase Project)
 */

/*
 * Currently supported platforms:
 *
 * 1. Reference platform: Arduino Mega, TFT_PQ 2.4 (ILI9325 controller), ITDB02 MEGA Shield v1.1
 *
 * More platforms coming soon
 */

#ifndef PIXELS_H
#define PIXELS_H

// #define DISABLE_ANTIALIASING 1

#if defined(__AVR__)
    #include <Arduino.h>

    #define regtype volatile uint8_t
    #define regsize uint8_t

    #define cbi(reg, bitmask) *reg &= ~bitmask
    #define sbi(reg, bitmask) *reg |= bitmask
    #define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
    #define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);


#elif defined(__SAM3X8E__)
    #include <Arduino.h>
    #define PROGMEM
    #define prog_uchar const unsigned char
    #define pgm_read_byte(x)        (*((char *)x))
    #define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_byte_near(x)   (*((char *)x))
    #define pgm_read_byte_far(x)    (*((char *)x))
//  #define pgm_read_word(x)        (*((short *)(x & 0xfffffffe)))
//  #define pgm_read_word_near(x)   (*((short *)(x & 0xfffffffe))
//  #define pgm_read_word_far(x)    (*((short *)(x & 0xfffffffe)))
    #define pgm_read_word_near(x)   ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_word_far(x)    ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x))))
    #define PSTR(x)  x
#else
    #define PROGMEM
    #define prog_uchar byte

    #define regtype volatile uint32_t
    #define regsize uint16_t
#endif

#define swap(a, b) {int16_t buf = a; a = b; b = buf;}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define deviceWrite(hi, lo) PORTA = hi; pulse_low(registerWR, bitmaskWR); PORTA = lo; pulse_low(registerWR, bitmaskWR)
#define deviceWriteTwice(b) PORTA = b; pulse_low(registerWR, bitmaskWR); pulse_low(registerWR, bitmaskWR)
#else
#define deviceWrite(hi, lo) PORTD = hi; pulse_low(registerWR, bitmaskWR); PORTD = lo; pulse_low(registerWR, bitmaskWR)
#define deviceWriteTwice(b) PORTD = b; pulse_low(registerWR, bitmaskWR); pulse_low(registerWR, bitmaskWR)
#endif


#define BITMASK_FONT 1
#define ANTIALIASED_FONT 2
#define HEADER_LENGTH 5

#define SCROLL_SMOOTH 1
#define SCROLL_CLEAN 2

#define FILL_TOPDOWN 0
#define FILL_LEFTRIGHT 0
#define FILL_DOWNTOP 1
#define FILL_RIGHTLEFT 2

#define ORIGIN_RELATIVE true // origin relative to a current scroll position
#define ORIGIN_ABSOLUTE false // origin matches physical device pixel coordinates

#define PORTRAIT 0
#define LANDSCAPE 1
#define PORTRAIT_FLIP 2
#define LANDSCAPE_FLIP 3

#define ipart(X) ((uint16_t)(X))
#define round(X) ((uint16_t)(((double)(X))+0.5))
#define fpart(X) (((double)(X))-(double)ipart(X))
#define rfpart(X) (1.0-fpart(X))

#define CSELECT cbi(registerCS, bitmaskCS)
#define CDESELECT sbi(registerCS, bitmaskCS)

class RGB {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGB(uint8_t r, uint8_t g, uint8_t b);
    RGB();

    RGB convert565toRGB(uint16_t color);
    uint16_t convertRGBto565(RGB color);
    uint16_t convertTo565();
};


class Pixels {
protected:
    /* device physical dimension in portrait orientation */
    uint16_t deviceWidth;
    uint16_t deviceHeight;

    /* device logical dimension in current orientation */
    uint16_t width;
    uint16_t height;

    boolean landscape;

    uint8_t orientation;

    boolean relativeOrigin;

    /* currently selected font */
    prog_uchar* currentFont;

    RGB foreground;
    RGB background;

    double lineWidth;

    uint8_t fillDirection;
    boolean antialiasing;

    boolean scrollSupported;
    boolean scrollEnabled;

    int16_t currentScroll;
    int16_t flipScroll;
    boolean scrollCleanMode;

    regtype *registerCS; // chip select
    regsize bitmaskCS;

    void printString(int16_t xx, int16_t yy, String text, boolean clean, int8_t kerning[] = NULL);

    virtual void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {};
    void setCurrentPixel(RGB color);
    void setCurrentPixel(int16_t color);
    void fill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    virtual void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {};
    void putColor(int16_t x, int16_t y, boolean steep, double weight);
    RGB computeColor(RGB, double weight);
    RGB computeColor(RGB fg, uint8_t opacity);

    void resetRegion();

    void hLine(int16_t x1, int16_t y1, int16_t x2);
    void vLine(int16_t x1, int16_t y1, int16_t y2);

    void deviceWriteCmd(uint8_t b) {};
    virtual void deviceWriteData(uint8_t hi, uint8_t lo) {};
    void deviceWriteCmdData(uint8_t cmd, uint16_t data) {};

    virtual void scrollCmd() {};

#ifndef DISABLE_ANTIALIASING
    void drawCircleAntialiaced(int16_t x, int16_t y, int16_t radius, boolean bordermode);
    void drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode);

    int16_t* loadFileBytes(String);
#endif

public:
    /**
     * Constructs a new <code>Pixels</code> object for the reference platform TFT_PQ 2.4 (ILI9325 controller) + ITDB02 MEGA Shield v1.1.
     * This constructor is the default contructor for a graphics
     * context.
     * Implicitly sets the target device width to 240, height to 320
     */
    Pixels();
    /**
     * Constructs a new <code>Pixels</code> object for the reference platform TFT_PQ 2.4 (ILI9325 controller) + ITDB02 MEGA Shield v1.1.
     * @param width target device width (in pixels)
     * @param height target device height (in pixels)
     */
    Pixels(uint16_t width, uint16_t height);
    /**
     * Initializes hardware with defaults.
     */
    virtual void init() {};
    /**
     * Sets the current coordinate space orientation.
     * Default value depends on initially given device width and height (PORTRAIT if height > width, otherwise LANDSCAPE).
     * @param orientation accepts PORTRAIT, LANDSCAPE, PORTRAIT_FLIP or LANDSCAPE_FLIP
     */
    void setOrientation(uint8_t orientation);
    /**
     * Returns the current coordinate space orientation.
     * @return the current orientation
     * @see setOrientation(uint8_t)
     */
    uint8_t getOrientation();
    /**
     * Enables or disables antialiasing by a drawing of graphical primitives. The metod does not impact antialiased fonts.
     * Antialiased output in general requires more resources/time to output comparing to "grainy" output mode
     * @param enable a boolean value that determines whether the antialiasing should be enabled or not
     */
    void enableAntialiasing(boolean enable);
    /**
     * Gets the current antialiasing mode.
     * @return a boolean value that shows whether the antialiasing is be enabled or not
     */
    boolean isAntialiased();
    /**
     * Enables or disables scroll feature.
     * @param enable a boolean value that determines whether the scroll feature should be enabled or not
     * @see scroll(int16_t,int8_t)
     */
    void enableScroll(boolean enable);
    /**
     * @returns <i>true</i> if the target device can scroll and the scroll feature is enabled.
     * @see scroll(int16_t,int8_t)
     * @see enableScroll(boolean)
     */
    boolean canScroll();
    /**
     * Sets the current line width. For time being the line width is respected by
     * drawLine(int16_t,int16_t,int16_t,int16_t) and a line width control is still "under construction".
     * @param width new line width.
     */
    void setLineWidth(double width);
    /**
     * Returns the current line width.
     * @return the current line width.
     */
    double getLineWidth();
    /**
     * Bounds the coordinate space to the device controller video RAM. The physical output depends on the actual scroll position.
     */
    void setOriginRelative();
    /**
     * Bounds the coordinate space to physical device pixels, ignoring actual scroll position.
     * @see scroll(int16_t,int8_t)
     */
    void setOriginAbsolute();
    /**
     * @return <i>true</i> if the current positioning is relative.
     * @see setOriginRelative()
     * @see setOriginAbsolute()
     */
    boolean isOriginRelative();
    /**
     * Outout fine tuning method for slow devices
     * @param direction accepts FILL_TOPDOWN, FILL_LEFTRIGHT, FILL_DOWNTOP or FILL_RIGHTLEFT
     */
    virtual void setFillDirection(uint8_t direction) {};
    /**
     * Fills the screen with the current background color
     */
    void clear();
    /**
     * Sets the current background color to the specified color.
     * All subsequent relevant graphics operations use this specified color.
     * @param r the red component
     * @param g the green component
     * @param b the blue component
     */
    void setBackground(uint8_t r, uint8_t g, uint8_t b);
    /**
     * Sets the current color to the specified color.
     * All subsequent graphics operations use this specified color.
     * @param r the red component
     * @param g the green component
     * @param b the blue component
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    /**
     * Sets the current background color to the specified color.
     * All subsequent relevant graphics operations use this specified color.
     * @param color color object
     */
    void setBackground(RGB color);
    /**
     * Sets the current color to the specified color.
     * All subsequent graphics operations use this specified color.
     * @param color color object
     */
    void setColor(RGB color);
    /**
     * Gets the graphics context's current background color.
     * @return    the graphics context's current background color.
     */
    RGB getBackground();
    /**
     * Gets the graphics context's current color.
     * @return    the graphics context's current color.
     */
    RGB getColor();
    /**
     * Gets a pixel color at the point
     * <code>(x,&nbsp;y)</code> in the current coordinate system.
     * If video RAM read is not supported by the hardware, returns the
     * graphics context's current background color
     * @param   x  <i>x</i> coordinate.
     * @param   y  <i>y</i> coordinate.
     * @return  pixel color or the graphics context's current background color.
     */
    RGB getPixel(int16_t x, int16_t y);
    /**
     * Draws a pixel, using the current color, at the point
     * <code>(x,&nbsp;y)</code>
     * in the current coordinate system.
     * @param   x  <i>x</i> coordinate.
     * @param   y  <i>y</i> coordinate.
     */
    void drawPixel(int16_t x, int16_t y);
    /**
     * Draws a line, using the current color, between the points
     * <code>(x1,&nbsp;y1)</code> and <code>(x2,&nbsp;y2)</code>
     * in current coordinate system.
     * @param   x1  the first point's <i>x</i> coordinate.
     * @param   y1  the first point's <i>y</i> coordinate.
     * @param   x2  the second point's <i>x</i> coordinate.
     * @param   y2  the second point's <i>y</i> coordinate.
     * @see     setOriginRelative()
     * @see     setOriginAbsolute()
     */
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    /**
     * Draws the outline of a circle, defined by center coordinates and a radius,
     * with the current color.
     * @param       x the <i>x</i> coordinate of the circle center.
     * @param       y the <i>y</i> coordinate of the circle center.
     * @param       radius circle radius.
     * @see         fillCircle(int16_t,int16_t,int16_t)
     */
    void drawCircle(int16_t x, int16_t y, int16_t radius);
    /**
     * Draws the outline of an oval.
     * The result is a circle or ellipse that fits within the
     * rectangle specified by the <code>x</code>, <code>y</code>,
     * <code>width</code>, and <code>height</code> arguments.
     * <p>
     * The oval covers an area that is
     * <code>width&nbsp;+&nbsp;1</code> pixels wide
     * and <code>height&nbsp;+&nbsp;1</code> pixels tall.
     * @param       x the <i>x</i> coordinate of the upper left
     *                     corner of the oval to be drawn.
     * @param       y the <i>y</i> coordinate of the upper left
     *                     corner of the oval to be drawn.
     * @param       width the width of the oval to be drawn.
     * @param       height the height of the oval to be drawn.
     * @see         fillOval(int16_t,int16_t,int16_t,int16_t)
     */
    void drawOval(int16_t x, int16_t y, int16_t width, int16_t height);
    /**
     * Draws the outline of the specified rectangle.
     * The left and right edges of the rectangle are at
     * <code>x</code> and <code>x&nbsp;+&nbsp;width</code>.
     * The top and bottom edges are at
     * <code>y</code> and <code>y&nbsp;+&nbsp;height</code>.
     * The rectangle is drawn using the current color.
     * @param         x   the <i>x</i> coordinate
     *                         of the rectangle to be drawn.
     * @param         y   the <i>y</i> coordinate
     *                         of the rectangle to be drawn.
     * @param         width   the width of the rectangle to be drawn.
     * @param         height   the height of the rectangle to be drawn.
     * @see           fillRectangle(int16_t,int16_t,int16_t,int16_t)
     */
    void drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    /**
     * Draws an outlined round-cornered rectangle using the current color.
     * The left and right edges of the rectangle
     * are at <code>x</code> and <code>x&nbsp;+&nbsp;width</code>,
     * respectively. The top and bottom edges of the rectangle are at
     * <code>y</code> and <code>y&nbsp;+&nbsp;height</code>.
     * @param      x the <i>x</i> coordinate of the rectangle to be drawn.
     * @param      y the <i>y</i> coordinate of the rectangle to be drawn.
     * @param      width the width of the rectangle to be drawn.
     * @param      height the height of the rectangle to be drawn.
     * @param      r horizontal and vertical diameters of the arc
     *                    at the four corners.
     * @see        fillRoundRectangle(int16_t,int16_t,int16_t,int16_t,int16_t)
     */
    void drawRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r);
    /**
     * Fills a circle, defined by center coordinates and a radius, with the current color.
     * @param       x the <i>x</i> coordinate of the circle center.
     * @param       y the <i>y</i> coordinate of the circle center.
     * @param       radius circle radius.
     * @see         drawCircle(int16_t,int16_t,int16_t)
     */
    void fillCircle(int16_t x, int16_t y, int16_t radius);
    /**
     * Fills an oval bounded by the specified rectangle with the
     * current color.
     * @param       x the <i>x</i> coordinate of the upper left corner
     *                     of the oval to be filled.
     * @param       y the <i>y</i> coordinate of the upper left corner
     *                     of the oval to be filled.
     * @param       width the width of the oval to be filled.
     * @param       height the height of the oval to be filled.
     * @see         drawOval(int16_t,int16_t,int16_t,int16_t)
     */
    void fillOval(int16_t x, int16_t y, int16_t width, int16_t height);
    /**
     * Fills the specified rectangle.
     * The left and right edges of the rectangle are at
     * <code>x</code> and <code>x&nbsp;+&nbsp;width&nbsp;-&nbsp;1</code>.
     * The top and bottom edges are at
     * <code>y</code> and <code>y&nbsp;+&nbsp;height&nbsp;-&nbsp;1</code>.
     * The resulting rectangle covers an area
     * <code>width</code> pixels wide by
     * <code>height</code> pixels tall.
     * The rectangle is filled using the current color.
     * @param         x   the <i>x</i> coordinate
     *                         of the rectangle to be filled.
     * @param         y   the <i>y</i> coordinate
     *                         of the rectangle to be filled.
     * @param         width   the width of the rectangle to be filled.
     * @param         height   the height of the rectangle to be filled.
     * @see           drawRectangle(int16_t,int16_t,int16_t,int16_t)
     */
    void fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    /**
     * Fills the specified rounded corner rectangle with the current color.
     * The left and right edges of the rectangle
     * are at <code>x</code> and <code>x&nbsp;+&nbsp;width&nbsp;-&nbsp;1</code>,
     * respectively. The top and bottom edges of the rectangle are at
     * <code>y</code> and <code>y&nbsp;+&nbsp;height&nbsp;-&nbsp;1</code>.
     * @param       x the <i>x</i> coordinate of the rectangle to be filled.
     * @param       y the <i>y</i> coordinate of the rectangle to be filled.
     * @param       width the width of the rectangle to be filled.
     * @param       height the height of the rectangle to be filled.
     * @param       r horizontal and vertical diameters of the arc at the four corners.
     * @see         drawRoundRectangle(int16_t,int16_t,int16_t,int16_t,int16_t)
     */
    void fillRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r);
    /**
     * Draws specified bitmap image.
     * The image is drawn with its top-left corner at
     * (<i>x</i>,&nbsp;<i>y</i>) in the current coordinate
     * space.
     * @param    data the specified bitmap image to be drawn. This method does
     *               nothing if <code>img</code> is null.
     * @param    x   the <i>x</i> coordinate.
     * @param    y   the <i>y</i> coordinate.
     * @param    width   the width of the image.
     * @param    height   the height of the image.
     * @see      loadBitmap(int16_t,int16_t,int16_t,int16_t,String)
     */
    int8_t drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, int data[]);
    /**
     * Loads from an external FAT-drive and draws specified bitmap image.
     * The image is drawn with its top-left corner at
     * (<i>x</i>,&nbsp;<i>y</i>) in the current coordinate
     * space.
     * @param    path to the image.
     * @param    x   the <i>x</i> coordinate.
     * @param    y   the <i>y</i> coordinate.
     * @param    width   the width of the image.
     * @param    height   the height of the image.
     * @see      drawBitmap(int16_t,int16_t,int16_t,int16_t,int[])
     */
    int8_t loadBitmap(int16_t x, int16_t y, int16_t width, int16_t height, String path);
    /**
     * Under construction
     */
    void scroll(int16_t dy, int16_t x1, int16_t x2, int8_t flags);
    /**
     * Scrolls the display content to a given number of pixels (if the device controller
     * supports scrolling). Scroll axis is vertical by portrait orientation and horizontal
     * by landscape orientation (hardware scpecifics).
     * SCROLL_CLEAN flag forces to paint wrapped regions with background color. SCROLL_SMOOTH
     * eases begin and end of a scroll movement (by big dy values).
     * @param dy negative or positive scroll distance
     * @param flags can be 0, SCROLL_SMOOTH or/and SCROLL_CLEAN
     */
    void scroll(int16_t dy, int8_t flags);
    /**
     * Sets this graphics context's font to the specified font.
     * All subsequent text operations using the context use this font.
     * @param  font   a font, converted from TTF with
     * <a href="http://pd4ml.com/pixelmeister/">Pixelmeister</a>.
     */
    int setFont(prog_uchar font[]);
    /**
     * Draws the text given by the specified string, using current font and color.
     * The baseline of the leftmost character is at position (<i>x</i>,&nbsp;<i>y</i>)
     * in the current coordinate system.
     * @param       text      the string to be drawn.
     * @param       xx        the <i>x</i> coordinate.
     * @param       yy        the <i>y</i> coordinate.
     * @param       kerning   optional array of integer kerning values in a range from
     * -99 to 99 to be applied to the string glyphs correspondingly. "-100" value ends
     * the kerning data. If the string has more characters than the array length, a value
     * precedes "-100" is used for the rest of the string glyphs.
     * @see         setFont(prog_uchar font[])
     * @see         setOriginRelative()
     * @see         setOriginAbsolute()
     * @see         cleanText(int16_t,int16_t,String,int8_t[])
     */
    void print(int16_t xx, int16_t yy, String text, int8_t kerning[] = NULL);
    /**
     * The method oposes print(int16_t,int16_t,String,int8_t[]) Erases the text given by
     * the specified string by filling glyph shapes with the current background color.
     * The baseline of the leftmost character is at position (<i>x</i>,&nbsp;<i>y</i>)
     * in the current coordinate system.
     * @param       text      the string to be erased.
     * @param       xx        the <i>x</i> coordinate.
     * @param       yy        the <i>y</i> coordinate.
     * @param       kerning   aptional array of integer kerning values in a range from
     * -99 to 99 to be applied to the string glyphs correspondingly. "-100" value ends
     * the kerning data. If the string has more characters than the array length, a value
     * precedes "-100" is usedas a kerning hint for the rest of the string glyphs.
     * @see         setFont(prog_uchar font[])
     * @see         setOriginRelative()
     * @see         setOriginAbsolute()
     * @see         print(int16_t,int16_t,String,int8_t[])
     */
    void cleanText(int16_t xx, int16_t yy, String text, int8_t kerning[] = NULL);
    /**
     * Gets the current font text line height
     * @return text line height in pixels
     */
    int16_t getTextLineHeight();
    /**
     * Gets the current font baseline offsett
     * @return text baseline offset
     */
    int16_t getTextBaseline();
    /**
     * Computes needed horizontal space to print the given text with print(int16_t,int16_t,String,int8_t)
     * @param       text      the text string.
     * @param       kerning   optional array of integer kerning values in a range from
     * -99 to 99 to be applied to the string glyphs correspondingly. "-100" value ends
     * the kerning data. If the string has more characters than the array length, a value
     * precedes "-100" is used as a kerning hint for the rest of the string glyphs.
     * @see         print(int16_t,int16_t,String,int8_t[])
     * @return text baseline offset
     */
    int16_t getTextWidth(String text, int8_t kerning[] = NULL);
};

#endif
