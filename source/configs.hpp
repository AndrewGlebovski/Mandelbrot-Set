/**
 * \file
 * \brief This file contains import constant values
*/

const int SCREEN_W = 1080;                      ///< Screen width in pixels
const int SCREEN_H = 1080;                      ///< Screen height in pixels

const unsigned int COLOR = 0x00E200FF;          ///< Progress bar and text color

#define FONT_FILE "assets/MainFont.ttf"         ///< Path to font file

const unsigned int FONT_SIZE = 24;              ///< Font size

const float RMAX = 4 * (float)SCREEN_W;         ///< Max distance from center
const int NMAX = 255;                           ///< Max iteration number

const float MOVE_FACTOR = 0.05f;                ///< Camera moving factor
const float ZOOM_FACTOR = 0.5f;                 ///< Camera zooming factor

const int POSSIBLE_COLORS = 16;                 ///< Possible non black colors

const float CENTER_X = -0.75;                   ///< Initial X0 additional offset
const float CENTER_Y = 0;                       ///< Initial Y0 additional offset

const float SET_W = 3.5;                        ///< Initial X scale
const float SET_H = 3.5;                        ///< Initial Y scale
