#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _WlzImage_h[] = "MRC HGU $Id$";
#endif
#endif
/*!
* \file         WlzImage.h
* \author       Zsolt Husz
* \date         June 2008
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C) 2008 Medical research Council, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	The WlzImage class to handle 3D Woolz objects sectioning
* \ingroup	WlzIIPServer
* \todo         -
* \bug          None known.
*/


#ifndef _WLZIMAGE_H
#define _WLZIMAGE_H


#include "IIPImage.h"
#include <Wlz.h>

#include "ViewParameters.h"

#include "WlzViewStructCache.h"
#include "WlzObjectCache.h"


/*! 
* \brief	Provides the WlzImage classs. Implementation is based on
*               TPTImage class (Tiled Pyramidal TIFF), part of the IIP Server
* \ingroup	WlzIIPServer
* \todo         -
* \bug          None known.
*/
class WlzImage : public IIPImage {


 protected:

  WlzObject                   *wlzObject;         /*!< current object */
  WlzThreeDViewStruct         *wlzViewStr;        /*!< current view structure */

  ViewParameters              *curViewParams;     /*!< current view parameters, partly 
                                                     redundant with wlzViewStr, however used to 
                                                     minimise compuations*/

  const ViewParameters        *viewParams;        /*!< view parameters set by the user.
                                                     These might not be reflected yet in wlzViewStr*/


  unsigned char               *tile_buf;          /*!< Tile data buffer */
  int                         number_of_tiles;    /*!< Number of tiles */
  static const WlzInterpolationType interp ;      /*!< Type of interpollation */

  int                         lastTileWidth;      /*!< Width for last column tiles */
  int                         lastTileHeight;     /*!< Height for last row tiles */

  int                         ntlx;               /*!< Number of tiles per row */
  int                         ntly;               /*!< Number of tiles per columns */
  unsigned char               background[4];      /*!< Background value */
 public:
  static WlzObjectCache       cacheWlzObject;     /*!< object cache*/
  static WlzViewStructCache   cacheViewStruct;    /*!< view structure cache*/

  //constructors and destructor
  WlzImage();
  WlzImage( const WlzImage& image );
  WlzImage( const std::string& path);


  /*!
  * \ingroup      WlzIIPServer
  * \brief        Destructor
  * \return       void
  * \par      Source:
  *                WlzImage.cc
  */
  ~WlzImage() {
      closeImage();
  };

  //overloaded IIPImage methods
  void openImage() ;
  void loadImageInfo( int x, int y ) throw (std::string);
  void closeImage();
  RawTile getTile( int x, int y, unsigned int r, unsigned int t) throw (std::string);
  string getFileName( );
  const std::string getHash();

  // Woolz operations
  void prepareViewStruct() throw (std::string);
  void prepareObject() throw (std::string);
  bool isViewChanged();

  //query functions
  float* getTrueVoxelSize();
  int getVolume();
  int getGreyValue(int *points);
  void getDepthRange(double& min, double& max);
  void getAngles(double& theta, double& phi, double& zeta);
  void get3DBoundingBox(int &plane1, int &lastpl, int &line1, int &lastln, int &kol1, int &lastkol);
  WlzDVertex3 getCurrentPointIn3D();
  WlzDVertex3 getTransformed3DPoint();
  int getForegroundObjects(int *values);
  int getCompoundNo();

 /*!
  * \ingroup      WlzIIPServer
  * \brief        Sets pointer to task parameters
  * \param        viewP pointer to view parameters
  * \return       void
  * \par      Source:
  *                WlzImage.cc
  */
  void setView( const ViewParameters  *viewP ){ viewParams = viewP; };

 /*!
  * \ingroup      WlzIIPServer
  * \brief        Forces channel no update to alpha value.
  * \param        alpha new alpha value
  * \return       void
  * \par      Source:
  *                WlzImage.cc
  */
  virtual void recomputeChannel(bool alpha) {
    if (channels == 3 || channels == 4)
      channels = alpha ? (unsigned int)4 : (unsigned int)3;
    else 
    if (channels == 1 || channels == 2)
      channels = alpha ? (unsigned int)2 : (unsigned int)1;
  };

  //internal functions
 protected:
  WlzErrorNum convertObjToRGB(unsigned char * cbuffer, WlzObject* obj, WlzIVertex2  pos, WlzIVertex2  size);
  WlzErrorNum convertDomainObjToRGB(unsigned char *cbuffer, WlzObject* obj, WlzIVertex2  pos, WlzIVertex2  size, CompoundSelector *sel);
  WlzErrorNum convertValueObjToRGB(unsigned char *cbuffer, WlzObject* obj, WlzIVertex2  pos, WlzIVertex2  size, CompoundSelector *sel);
  WlzErrorNum sectionObject(unsigned char* tile_buf, WlzObject *wlzObject, WlzObject *tileObject, WlzIVertex2  pos, WlzIVertex2  size, CompoundSelector *sel);
  WlzDVertex3 getCurrentPointInPlane();
  const std::string generateHash(const ViewParameters *view);
  const std::string selString(const ViewParameters* view );

  /*!
   * \ingroup      WlzIIPServer
   * \brief        Return the number of channels for the output
   * \return       void
   * \par      Source:
   *                WlzImage.cc
   */
  unsigned int getNumChannels() {
      // forces RGB/RGBA output if a selector is specified
      return (viewParams->selector) ? ((viewParams->alpha) ? 4:3) : channels;
  };
};

#endif
