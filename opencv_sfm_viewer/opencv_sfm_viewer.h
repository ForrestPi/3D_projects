
#ifndef _OPENCV_SFM_VIEWER_H_
#define _OPENCV_SFM_VIEWER_H_

//******************************************************************************
//******************************************************************************

//=============================================================================
// OPENCV Includes
//=============================================================================
#include <cv.h>

//=============================================================================
// OPENGL Includes
//=============================================================================
#include <GL/freeglut.h>
#include <pthread.h>

//=============================================================================
// Other Includes
//=============================================================================
#include <vector>
#include <iostream>
#include <fstream>

//******************************************************************************
//******************************************************************************

// Define the structure for the 3D points
typedef struct
{
	float x,y,z; // 3D position of the point
	float r,g,b; // RGB color of the 3D point
	
}SfM_3d_point;

// Define the structure for the camera poses
typedef struct
{
	float tx,ty,tz; // 3D camera translation
	float RWC[3][3]; // Rotation matrix from camera to world coordinate frame
}SfM_pose;

// Define the structure for SfM data
typedef struct
{
	std::vector<SfM_3d_point> points;
	std::vector<SfM_pose> poses;
}SfM_data;

// The data structures for the colors of 3D points
struct SfMColor
{
	GLfloat r, g, b, alpha;
	SfMColor(GLfloat r0, GLfloat g0, GLfloat b0, GLfloat alpha0) : r(r0), g(g0), b(b0), alpha(alpha0) {}
};

// Some camera camera color
const SfMColor default_camera_color(255.f/255.f, 0.0f/255.f, .0f/255.f, 1.f);

// The data structure for 3D points
struct Vertex
{
	GLfloat X,Y,Z;
	Vertex(GLfloat X0, GLfloat Y0, GLfloat Z0) : X(X0), Y(Y0), Z(Z0) {}
	Vertex() {}
};

// A camera is composed of five vertices
struct CameraVertices
{
	Vertex v[5];
};

//******************************************************************************
//******************************************************************************

// Some constants for the size of points, lines, linewidth...
const float POINT_SIZE = 2.0;
const float LINE_WIDTH = 3.0;

const float INITIAL_CAM_X = 0.0;
const float INITIAL_CAM_Y = 0.0;
const float INITIAL_CAM_Z = 0.0;

const float INCREMENTAL_Z = 400.0;
const float INCREMENTAL_Y = 400.0;
const float INCREMENTAL_X = 400.0;
const float INCREMENTAL_ORBIT_X_ANGLE = 2.0;
const float INCREMENTAL_ORBIT_Y_ANGLE = 2.0;

const float CAMERA_SIZE_X = 80.0; // Camera size x-axis in mm
const float CAMERA_SIZE_Y = 80.0; // Camera size y-axis in mm
const float CAMERA_SIZE_Z = 80.0; // Camera size z-axis in mm
const float CAMERA_OFFSET_X = 300.0; // Camera view offset x-axis in mm
const float CAMERA_OFFSET_Y = 200.0; // Camera view offset y-axis in mm
const float CAMERA_OFFSET_Z = 1000.0; // Camera view offset z-axis in mm
const float UNITS_CONVERSION = 1000.0; // From mm to m

const int TEXT_U = 5; // Image horizontal coordinate for writing text in OpenCV
const int TEXT_V = 230; // Image vertical coordinate for writing text in OpenCV

const bool DISPLAY_CAMERA_POSES = true; // In case we want to display camera poses
const bool DRAW_RGB_CAMERA = true; // In case we want to draw a rgb camera or a different representation
const bool DISPLAY_3D_POINTS = true; // In case we want to display 3D points

//******************************************************************************
//******************************************************************************

// Declaration of functions
void on_opengl(void *param);
void on_mouse(int event, int x, int y, int flags, void* param);
void on_keyboard(int key);
void Load3D(const char *cad, SfM_data &data);
void OpenGL_to_OpenCV_Image(IplImage *img);
void Draw_Camera_Poses(SfM_data *pt);
void Draw_Text_Camera_Pose(IplImage *img, float X, float Y, float Z);
void Compute_Camera_Vertices(Vertex* pv, const float pose[3], const float Rwc[3][3]);
void Draw_Camera(const Vertex* pv, const SfMColor& color, bool fill);
void Draw_RGB_Camera(const float tpose[3], const float Rwc[3][3]);
void Draw_One_Line(GLfloat X1, GLfloat Y1, GLfloat Z1, GLfloat X2,
				   GLfloat Y2, GLfloat Z2, const SfMColor& color, GLfloat linewidth);


//******************************************************************************
//******************************************************************************

#endif