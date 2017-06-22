/*
 * Copyright (c) 2017 Hackdirt Ltd.
 * Author: David Petrie (david@davidpetrie.com)
 *
 * License: https://www.gnu.org/licenses/gpl-3.0.en.html
 */

#ifndef SMASHED_IPHONE_CONFIG_H
#define SMASHED_IPHONE_CONFIG_H

#define IPHONE_BUILD 1


#define kPixelsPerGameUnit 106.666667


/* Landscape */
#define kLandscapePixelAspectRatio 1.5f

#define kLandscapePixelScreenWidth	480.0f
#define kLandscapePixelScreenHeight	320.0f

#define kLandscapeGameScreenWidth	4.8f
#define kLandscapeGameScreenHeight	3.2f

#define kLandscapeGameMinScreenWidth	7.5f
#define kLandscapeGameMinScreenHeight	5.0f
#define kLandscapeGameMaxScreenWidth	15.0f
#define kLandscapeGameMaxScreenHeight	15.0f


#define kDefaultScreenWidth		kPortraitGameScreenWidth
#define kDefaultScreenHeight	kPortraitGameScreenHeight
#define kDefaultScreenDepth		50.0f



#define kZoomLoRatioX 1.0f
#define kZoomLoRatioY 1.5f
#define kZoomHiRatioX 2.0f
#define kZoomHiRatioY 3.0f


#endif // SMASHED_IPHONE_CONFIG_H
