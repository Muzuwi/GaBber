#pragma once

/*
 *  CPU definitions
 */

//  TODO: Think over if this is correct
//  Multiple sources claim different frequencies for the processor
//  16.8MHz, 16.78MHz, or derived from base crystal 16.777216MHz
//  However, 16.777216 MHz is a multiple of the horizontal lcd scan freq, which could make things a bit easier
#define BASE_CLOCK_FREQ 4194304
#define ARM7TDMI_CPU_FREQ (BASE_CLOCK_FREQ * 4)

/*
 *  LCD definitions
 */
#define LCD_DOTS_PER_LINE 240
#define LCD_DOTS_PER_HBLANK 68
#define LCD_DOTS_PER_SCANLINE (LCD_DOTS_PER_LINE + LCD_DOTS_PER_HBLANK)

#define LCD_VERTICAL_LINES 160
#define LCD_VBLANK_LINES 68
#define LCD_LINES_PER_FRAME (LCD_VERTICAL_LINES + LCD_VBLANK_LINES)

#define LCD_RESOLUTION_WIDTH LCD_DOTS_PER_LINE
#define LCD_RESOLUTION_HEIGHT LCD_VERTICAL_LINES

#define LCD_HORIZONTAL_SCAN_FREQUENCY 13618