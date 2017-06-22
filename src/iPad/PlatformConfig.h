/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

/**
 * Platform specific settings.
 *
 * Be good if we could just define a single set of
 * values and just use the aspect ratio to alter them
 * according to whether the device is in landscape or
 * portrait mode. 
 */

#ifndef SMASHED_IPAD_CONFIG_H
#define SMASHED_IPAD_CONFIG_H

// #define kPixelsPerGameUnit 106.667
/* Can't figure out what this value is useful for */
#define kPixelsPerGameUnit 213.3333333 

/* Landscape */
#define kLandscapePixelAspectRatio 1.33333333f

#define kLandscapePixelScreenWidth 1024.0f
#define kLandscapePixelScreenHeight 768.0f

#define kLandscapeGameScreenWidth 4.8f
#define kLandscapeGameScreenHeight 3.6f

#define kLandscapeGameMinScreenWidth 7.5f
#define kLandscapeGameMinScreenHeight 5.625f
#define kLandscapeGameMaxScreenWidth 18.0f
#define kLandscapeGameMaxScreenHeight 13.5f

#define kDefaultScreenWidth  kPortraitGameScreenWidth
#define kDefaultScreenHeight kPortraitGameScreenHeight
#define kDefaultScreenDepth  50.0f

#define kZoomLoRatioX 1.0f
#define kZoomLoRatioY 1.33333333f
#define kZoomHiRatioX 3.0f
#define kZoomHiRatioY 4.0f

#endif
