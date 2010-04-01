#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _OBJ_cc[] = "MRC HGU $Id$";
#endif
#endif

/*
    IIP OJB Command Handler Class Member Functions

    Copyright (C) 2008 Zsolt Husz, Medical research Council, UK.
    Copyright (C) 2006 Ruven Pillay.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "Task.h"
#include <iostream>
#include <algorithm>


using namespace std;



void OBJ::run( Session* s, std::string a )
{

  argument = a;
  // Convert to lower case the argument supplied to the OBJ command
  transform( argument.begin(), argument.end(), argument.begin(), ::tolower );

  session = s;

  // Log this
  if( session->loglevel >= 3 ) *(session->logfile) << "OBJ :: " << argument << " to be handled" << endl;

  // Time this command
  if( session->loglevel >= 2 ) command_timer.start();


  if( argument == "iip,1.0" ) iip();
  else if( argument == "basic-info" ){
    iip_server();
    max_size();
    resolution_number();
    colorspace( "*,*" );
  }
  else if( argument == "iip-server" ) iip_server();
  // IIP optional commands
  else if( argument == "iip-opt-comm" ) session->response->addResponse( "IIP-opt-comm:CVT CNT QLT JTL JTLS WID HEI RGN SHD WLZ DST FXP MOD PAB PIT PRL ROL SEL SCL UPV YAW");

  // IIP optional objects
  else if( argument == "iip-opt-obj" ) session->response->addResponse( "IIP-opt-obj:Horizontal-views Vertical-views Tile-size Wlz-3d-bounding-box Wlz-foreground-objects Wlz-true-voxel-size Wlz-distance-range Wlz-coordinate-3d Wlz-grey-value Wlz-volume Wlz-sectioning-angles Wlz-transformed-coordinate-3d");

  // Resolution-number
  else if( argument == "resolution-number" ) resolution_number();
  // Max-size
  else if( argument == "max-size" ) max_size();
  // Tile-size
  else if( argument == "tile-size" ) tile_size();
  // Vertical-views
  else if( argument == "vertical-views" ) vertical_views();
  // Horizontal-views
  else if( argument == "horizontal-views" ) horizontal_views();

  // Colorspace
  /* The request can have a suffix, which we don't need, so do a
     like scan
  */
  else if( argument.find( "colorspace" ) != string::npos ){
    colorspace( "*,*" );
  }

  // Image Metadata
  else if( argument == "summary-info" ){

    metadata( "copyright" );
    metadata( "subject" );
    metadata( "author" );
    metadata( "create-dtm" );
    metadata( "app-name" );
  }

  else if( argument == "copyright" || argument == "title" || 
	   argument == "subject" || argument == "author" ||
	   argument == "keywords" || argument == "comment" ||
	   argument == "last-author" || argument == "rev-number" ||
	   argument == "edit-time" || argument == "last-printed" ||
	   argument == "create-dtm" || argument == "last-save-dtm" ||
	   argument == "app-name" ){

    metadata( argument );
  }

  //////////////////////////////////
  // Woolz queries start

  // Voxel size
  else if( argument == "wlz-true-voxel-size" ) wlz_true_voxel_size();

  // Distance range
  else if( argument == "wlz-distance-range" ) wlz_distance_range();

  // 3D coordinate
  else if( argument == "wlz-coordinate-3d" ) wlz_coordinate_3d();

  // Grey value of the selected pixel
  else if( argument == "wlz-grey-value" ) wlz_grey_value();

  // Volume
  else if( argument == "wlz-volume" ) wlz_volume();

  // 3D Bounding Box
  else if( argument == "wlz-3d-bounding-box" ) wlz_3d_bounding_box();

  // Sectioning angles
  else if( argument == "wlz-sectioning-angles" ) wlz_sectioning_angles();

  // 3D point transformation
  else if( argument == "wlz-transformed-coordinate-3d" ) wlz_transform_3d();

  //object index of a compund with selected point in the foreground
  else if( argument == "wlz-foreground-objects" ) wlz_foreground_objects();


  // Woolz queries end
  //////////////////////////////////

  // None of the above!
  else{

    if( session->loglevel >= 1 ){
      *(session->logfile) << "OBJ :: Unsupported argument: " << argument << " received" << endl;
    }

    // Unsupported object error code is 3 2
    session->response->setError( "3 2", argument );
  }


  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Total command time " << command_timer.getTime() << " microseconds" << endl;
  }


}



void OBJ::iip(){
  session->response->setProtocol( "IIP:1.0" );
}


void OBJ::iip_server(){
  // The binary capability code is 1000001 == 65 in integer
  // ie can do CVT jpeg and JTL, but no transforms
  session->response->addResponse( "IIP-server:255.552255" );
}


void OBJ::max_size(){

  checkImage();
  openIfWoolz();
  int x = (*session->image)->getImageWidth();
  int y = (*session->image)->getImageHeight();
  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Max-size is " << x << " " << y << endl;
  }
  session->response->addResponse( "Max-size", x, y );
}


void OBJ::resolution_number(){

  checkImage();
  openIfWoolz();
  int no_res = (*session->image)->getNumResolutions();
  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Resolution-number handler returning " << no_res << endl;
  }
  session->response->addResponse( "Resolution-number", no_res );

}

void OBJ::wlz_distance_range(){

  checkImage();
  checkIfWoolz();
  double min,max;

  ((WlzImage*)(*session->image))->getDepthRange(min,max);

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-section-size handler returning " << min << " -" << max << endl;
  }
  session->response->addResponse( "Wlz-distance-range", min, max);
}

void OBJ::wlz_sectioning_angles(){

  checkImage();
  checkIfWoolz();

  double theta = 0.0, phi = 0.0, zeta = 0.0;

  ((WlzImage*)(*session->image))->getAngles( theta, phi, zeta );

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-sectioning-angles handler returning " << phi << " -" << theta << " -" << zeta << endl;
  }
  session->response->addResponse( "Wlz-sectioning-angles", phi, theta, zeta );
}

void OBJ::wlz_3d_bounding_box(){

  checkImage();
  checkIfWoolz();

  int plane1 = 0, lastpl = 0, line1 = 0, lastln = 0, kol1 = 0, lastkol = 0;

  ((WlzImage*)(*session->image))->get3DBoundingBox( plane1, lastpl, line1, lastln, kol1, lastkol);

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-3d-bounding-box handler returning " << plane1 << " -" << lastpl << " -" << line1 << " -" << lastln << " -" << kol1 << " -" <<  lastkol << endl;
  }
  session->response->addResponse( "Wlz-3d-bounding-box", plane1, lastpl, line1, lastln, kol1, lastkol );
}

void OBJ::wlz_coordinate_3d(){

  checkImage();
  checkIfWoolz();

  WlzDVertex3 result = ((WlzImage*)(*session->image))->getCurrentPointIn3D();

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-coordinate-3d handler returning " << result.vtX<< "," << result.vtY << "," << result.vtZ << endl;
  }
  session->response->addResponse( "Wlz-coordinate-3d", result.vtX, result.vtY, result.vtZ );
}

void OBJ::wlz_transform_3d(){

  checkImage();
  checkIfWoolz();

  WlzDVertex3 result = ((WlzImage*)(*session->image))->getTransformed3DPoint();

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-transformed-coordinate-3d handler returning " << round(result.vtX)<< "," << round(result.vtY) << "," << result.vtZ << endl;
  }
  session->response->addResponse( "Wlz-transformed-coordinate-3d", round(result.vtX), round(result.vtY), result.vtZ );
}


void OBJ::wlz_true_voxel_size(){

  checkImage();
  checkIfWoolz();

  float *voxel_size = ((WlzImage*)(*session->image))->getTrueVoxelSize();

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-voxel-size handler returning " << voxel_size[0] << "," << voxel_size[1] << "," << voxel_size[2] << endl;
  }
  session->response->addResponse( "Wlz-voxel-size", voxel_size[0], voxel_size[1], voxel_size[2] );
}

void OBJ::wlz_volume(){

  checkImage();
  checkIfWoolz();

  int volume = ((WlzImage*)(*session->image))->getVolume();

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Wlz-volume handler returning " << volume << endl;
  }
  session->response->addResponse( "Wlz-volume", volume );
}

void OBJ::wlz_grey_value(){

  checkImage();
  checkIfWoolz();
  int values[4];
  int channels = ((WlzImage*)(*session->image))->getGreyValue(values);

  if( session->loglevel >= 2 ){
    switch (channels) {
      case 1:
        *(session->logfile) << "OBJ :: Wlz-grey-value handler returning: " << values[0]  << endl;
        break;
      case 3:
        *(session->logfile) << "OBJ :: Wlz-grey-value handler returning: " << values[0] << ' ' << values[1] << ' '<< values[2] << endl;
        break;
      case 4:
        *(session->logfile) << "OBJ :: Wlz-grey-value handler returning: " << values[0] << ' ' << values[1] << ' '<< values[2] << ' '<< ' ' << values[3] << endl;
        break;
      default:
        throw string( "Wlz-grey-value: unknow channel number" );
        break;
    }
  }
  switch (channels) {
    case 1:
      session->response->addResponse( "Wlz-grey-value", values[0] );
      break;
    case 3:
      session->response->addResponse( "Wlz-colour-value", values[0], values[1], values[2]);
      break;
    default:
      throw string( "Wlz-grey-value: unknow channel number" );
      break;
  }
}


void OBJ::wlz_foreground_objects(){
  checkImage();
  checkIfWoolz();
  int objects = ((WlzImage*)(*session->image))->getCompoundNo();
  int *values = new int [objects];
  if (values != NULL) {
      int foregroundNo = ((WlzImage*)(*session->image))->getForegroundObjects(values);
      int i = 0;
      if( session->loglevel >= 2 ) {
          *(session->logfile) << "OBJ :: Wlz-foreground-objects handler returning: ";
          for (i=0; i<foregroundNo; i++) {
              *(session->logfile) << values[i]  << ' ';
          }
         *(session->logfile) << endl;
      }
      session->response->addResponse( "Wlz-foreground-objects", foregroundNo, values );
      delete values;
  } else {
      if( session->loglevel >= 1 ){
          *(session->logfile) << "OBJ :: Wlz-foreground-objects handler returning cant allocate memory. " << endl;
      }
  }
}


void OBJ::tile_size(){
  checkImage();

  int x = (*session->image)->getTileWidth();
  int y = (*session->image)->getTileHeight();
  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Tile-size is " << x << " " << y << endl;
  }
  session->response->addResponse( "Tile-size", x, y );
}


void OBJ::vertical_views(){
  checkImage();
  list <int> views = (*session->image)->getVerticalViewsList();
  list <int> :: const_iterator i;
  string tmp = "Vertical-views:";
  char val[8];
  for( i = views.begin(); i != views.end(); i++ ){
    snprintf( val, 8, "%d ", *i );
    tmp += val;
  }
  // Chop off the final space
  tmp.resize( tmp.length() - 1 );
  session->response->addResponse( tmp );
}


void OBJ::horizontal_views(){
  checkImage();
  list <int> views = (*session->image)->getHorizontalViewsList();
  list <int> :: const_iterator i;
  string tmp = "Horizontal-views:";
  char val[8];
  for( i = views.begin(); i != views.end(); i++ ){
    snprintf( val, 8, "%d ", *i );
    tmp += val;
  }
  // Chop off the final space
  tmp.resize( tmp.length() - 1 );
  session->response->addResponse( tmp );
}


void OBJ::colorspace( std::string arg ){

   checkImage();
   openIfWoolz();

  /* Assign the colourspace tag: 1 for greyscale, 3 for RGB and
     a colourspace of 4 to LAB images
     WARNING: LAB support is an extension and is not in the
     IIP protocol standard (as of version 1.05)
  */
  const char *planes = "3 0 1 2";
  int calibrated = 0;
  int colourspace;
  if( (*session->image)->getColourSpace() == CIELAB ){
    colourspace = 4;
    calibrated = 1;
  }
  else if( (*session->image)->getColourSpace() == GREYSCALE ){
    colourspace = 1;
    planes = "1 0";
  }
  else colourspace = 3;

  int no_res = (*session->image)->getNumResolutions();
  char tmp[32];
  snprintf( tmp, 32, "Colorspace,0-%d,0:%d 0 %d %s", no_res-1,
	    calibrated, colourspace, planes );

  if( session->loglevel >= 2 ){
    *(session->logfile) << "OBJ :: Colourspace handler returning " << tmp << endl;
  }

  session->response->addResponse( tmp );
}


void OBJ::metadata( std::string field ){

  checkImage();
  openIfWoolz();
  string metadata = (*session->image)->getMetadata( field );
  if( session->loglevel >= 3 ){
    *(session->logfile) << "OBJ :: " << field << " handler returning" << metadata << endl;
  }

  if( metadata.length() ){
    session->response->addResponse( field, metadata.c_str() );
  }
}

