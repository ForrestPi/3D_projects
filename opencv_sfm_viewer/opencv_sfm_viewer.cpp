
//=============================================================================
//
// opencv_sfm_viewer.cpp
// Core functions for the SfM viewer
// Authors: Pablo F. Alcantarilla, Kai Ni
// Date: 26/08/2011
// Email: pablo.alcantarilla@depeca.uah.es
//=============================================================================

#include "opencv_sfm_viewer.h"

// Namespaces
using namespace std;

// Global variables
float ZOOM_Z = 0.0;
float ZOOM_X = 0.0;
float ZOOM_Y = 0.0;
float ORBIT_X_ANGLE = 0.0;
float ORBIT_Y_ANGLE = 0.0;

// Create OpenCV matrices for transforming world points to camera coordinate frame
cv::Mat oriaux = cv::Mat(3,1,CV_32FC1);
cv::Mat xaux = cv::Mat(3,1,CV_32FC1);
cv::Mat yaux = cv::Mat(3,1,CV_32FC1);
cv::Mat zaux = cv::Mat(3,1,CV_32FC1);
cv::Mat Yiaux = cv::Mat(3,1,CV_32FC1);
cv::Mat hiaux = cv::Mat(3,1,CV_32FC1);
cv::Mat tpose = cv::Mat(3,1,CV_32FC1);
cv::Mat Rwc = cv::Mat(3,3,CV_32FC1);

//*******************************************************************************
//*******************************************************************************

/** This functions loads the 3D information motion+structure from a .txt file	*/
void Load3D(const char *cad, SfM_data &data)
{
	float x = 0.0, y = 0.0, z = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;
	float tx = 0.0, ty = 0.0, tz = 0.0;
	float r11 = 0.0, r12 = 0.0, r13 = 0.0;
	float r21 = 0.0, r22 = 0.0, r23 = 0.0;
	float r31 = 0.0, r32 = 0.0, r33 = 0.0;
	int nposes = 0, npoints = 0;
	SfM_3d_point aux_point;
	SfM_pose aux_pose;
	
	// Load the data file
	ifstream is(cad,ifstream::in);
	string tag;
	
	while (is)
	{
		is >> tag;

		// load 3D points
		if (tag == "POINT3")
		{
			is >> x >> y >> z >> r >> g >> b;
			
			aux_point.x = x;
			aux_point.y = y;
			aux_point.z = z;
			aux_point.r = r;
			aux_point.g = g;
			aux_point.b = b;
		
			data.points.push_back(aux_point);
			npoints++;
		}

		// load 3D camera
		if (tag == "POSE3")
		{
			is >> tx >> ty >> tz
			>> r11 >> r12 >> r13
			>> r21 >> r22 >> r23
			>> r31 >> r32 >> r33;
			
			aux_pose.tx = tx;
			aux_pose.ty = ty;
			aux_pose.tz = tz;
			
			aux_pose.RWC[0][0] = r11; aux_pose.RWC[0][1] = r12; aux_pose.RWC[0][2] = r13;
			aux_pose.RWC[1][0] = r21; aux_pose.RWC[1][1] = r22; aux_pose.RWC[1][2] = r23;
			aux_pose.RWC[2][0] = r31; aux_pose.RWC[2][1] = r32; aux_pose.RWC[2][2] = r33;
			
			data.poses.push_back(aux_pose);
			nposes++;
		}

		is.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
	}
	is.close();
	
	cout << "Loaded " << npoints << " points and " << nposes << " cameras" << endl;
	cout.flush();
}


//*******************************************************************************
//*******************************************************************************

/** Callback for OpenGL display: 3D points + camera poses					   */
void on_opengl(void *param)
{
    float eyeX = 0.0, eyeY = 0.0, eyeZ = 0.0;

	CameraVertices vertices;
	
	// Pointers accessors for the scene and pose information
	SfM_data *pt = (SfM_data *)param;

	// Load the identity matrix
    glLoadIdentity();
	glEnable(GL_LIGHT3);

	eyeX = (INITIAL_CAM_X - ZOOM_X)/UNITS_CONVERSION;
	eyeY = (INITIAL_CAM_Y - ZOOM_Y)/UNITS_CONVERSION;
	eyeZ = (INITIAL_CAM_Z - ZOOM_Z)/UNITS_CONVERSION;
	
	// Firstly, translate the camera view
	glTranslated(eyeX,eyeY,eyeZ);
	
	// Rotate above the X and Y angles (pitch,yaw)
	glRotatef(ORBIT_X_ANGLE, 1.f, 0.f, 0.f);/* orbit the X axis */	
	glRotatef(ORBIT_Y_ANGLE, 0.f, 1.f, 0.f);/* orbit the Y axis */	

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if( DISPLAY_3D_POINTS == true )
	{
		// Point rendering setting
		glPointSize(POINT_SIZE);
	
		// Draw the structure (Map of 3D points)
		glBegin(GL_POINTS);
		
		for( unsigned int i = 0; i < pt->points.size(); i++ )
		{
			// Firstly, set the rgb color of the 3D point
			glColor4f(pt->points[i].r,pt->points[i].g,pt->points[i].b,1.0);
			
			// Draw the 3D point (coordinates in m)
			glVertex3f(pt->points[i].x/UNITS_CONVERSION,-1.0*pt->points[i].y/UNITS_CONVERSION,
					  -1.0*pt->points[i].z/UNITS_CONVERSION);
		}
		glEnd();
	}
	
	if( DISPLAY_CAMERA_POSES == true )
	{
		// Draw the camera keyframes
		Draw_Camera_Poses(pt);		
	}
}

//*******************************************************************************
//*******************************************************************************

/** This function converts an OpenGL image to an OpenCV IplImage format			*/
/** This is only necessary when we want to mount videos showing nice results	*/
/** In other case, we shouldn't need this function								*/
void OpenGL_to_OpenCV_Image(IplImage *imgO )
{
	int width = imgO->width;
	int height = imgO->height;
	int hIndex=0;
	int wIndex=0;
	int iout,jout;

	unsigned char* imageData = (unsigned char*)malloc(width*height*3);
	IplImage *img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);

	// Read an image of dimension (width*height) pixels with 3 color channels (RGB) 
	// Then put the data as unsigned 8-bit number in imageData array. 
	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,imageData);
	
	for(int i=0;i<width*height*3;i+=3,wIndex++)
	{
		if(wIndex >= width)
		{
			wIndex=0;
			hIndex++;
		}

		((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 0]= imageData[i+2]; // B
		((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 1] = imageData[i+1]; // G
		((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 2] = imageData[i]; // R

		// vertically flip the image
		iout = -hIndex+height-1;
		jout = wIndex;

		((unsigned char *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 0] =
			((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 0];// B
		((unsigned char *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 1] =
			((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 1];// G
		((unsigned char *)(imgO->imageData + iout*imgO->widthStep))[jout*imgO->nChannels + 2] =
			((unsigned char *)(img->imageData + hIndex*img->widthStep))[wIndex*img->nChannels + 2];// R
	}

	free(imageData);
	cvReleaseImage(&img);
}


//*******************************************************************************
//*******************************************************************************

/** This function draws the text of the current location of the camera pose		*/
void Draw_Text_Camera_Pose(IplImage *img, float X, float Y, float Z)
{
	char text[100];
	CvFont fs;
	cvInitFont(&fs,CV_FONT_HERSHEY_PLAIN,1,1,0,2,8);
	
	sprintf(text,"X %.2f Y %.2f Z %.2f (m)",X/1000.0,Y/1000.0,Z/1000.0);
	cvPutText(img,text,cvPoint(TEXT_U,TEXT_V),&fs,CV_RGB(255,255,255));
}

//*******************************************************************************
//*******************************************************************************

/** Function for drawing the poses in the vector list							*/
void Draw_Camera_Poses(SfM_data *pt)
{
	CameraVertices vertices;
	float Tpose[3];
	
	for( unsigned int i = 0; i < pt->poses.size(); i++ )
	{
		// Get the camera pose translation
		Tpose[0] = pt->poses[i].tx;
		Tpose[1] = pt->poses[i].ty;
		Tpose[2] = pt->poses[i].tz;
		
		// Compute the camera vertices for the current pose
		Compute_Camera_Vertices(vertices.v,Tpose,pt->poses[i].RWC);

		// Draw the camera pose 
		if( DRAW_RGB_CAMERA == true )
		{
			Draw_RGB_Camera(Tpose,pt->poses[i].RWC);
		}
		else
		{
			Draw_Camera(vertices.v,default_camera_color,false);
		}
	}
}

//*******************************************************************************
//*******************************************************************************

/** Function to compute the coordinates of the camera vertices	  			   */
void Compute_Camera_Vertices(Vertex* pv, const float Tpose[3], const float RWC[3][3])
{	
	// Fill OpenCV matrix with the translation vector
	tpose.at<float>(0,0) = Tpose[0];
	tpose.at<float>(1,0) = Tpose[1];
	tpose.at<float>(2,0) = Tpose[2];
	
	// Fill OpenCV matrix with the rotation matrix
	Rwc.at<float>(0,0) = RWC[0][0];
	Rwc.at<float>(0,1) = RWC[0][1];
	Rwc.at<float>(0,2) = RWC[0][2];
	Rwc.at<float>(1,0) = RWC[1][0];
	Rwc.at<float>(1,1) = RWC[1][1];
	Rwc.at<float>(1,2) = RWC[1][2];
	Rwc.at<float>(2,0) = RWC[2][0];
	Rwc.at<float>(2,1) = RWC[2][1];
	Rwc.at<float>(2,2) = RWC[2][2];
	
	// 0 Vertex (origin of the camera)
	pv[0].X = tpose.at<float>(0,0);	pv[0].Y = -tpose.at<float>(1,0); pv[0].Z = -tpose.at<float>(2,0);
	
	// First Vertex
	hiaux.at<float>(0,0) = -CAMERA_SIZE_X/2.0;	hiaux.at<float>(1,0) = -CAMERA_SIZE_Y/2.0;	hiaux.at<float>(2,0) = CAMERA_SIZE_Z;
	Yiaux = Rwc*hiaux + tpose;
	pv[1].X = Yiaux.at<float>(0,0);	pv[1].Y = -Yiaux.at<float>(1,0);	pv[1].Z = -Yiaux.at<float>(2,0);
	
	// Second Vertex
	hiaux.at<float>(0,0) = -CAMERA_SIZE_X/2.0;	hiaux.at<float>(1,0) = CAMERA_SIZE_Y/2.0;	hiaux.at<float>(2,0) = CAMERA_SIZE_Z;
	Yiaux = Rwc*hiaux + tpose;
	pv[2].X = Yiaux.at<float>(0,0);	pv[2].Y = -Yiaux.at<float>(1,0);	pv[2].Z = -Yiaux.at<float>(2,0);
	
	// Third Vertex
	hiaux.at<float>(0,0) = CAMERA_SIZE_X/2.0;	hiaux.at<float>(1,0) = CAMERA_SIZE_Y/2.0;	hiaux.at<float>(2,0) = CAMERA_SIZE_Z;
	Yiaux = Rwc*hiaux + tpose;
	pv[3].X = Yiaux.at<float>(0,0);	pv[3].Y = -Yiaux.at<float>(1,0);	pv[3].Z = -Yiaux.at<float>(2,0);
	
	// Fourth Vertex
	hiaux.at<float>(0,0) = CAMERA_SIZE_X/2.0;	hiaux.at<float>(1,0) = -CAMERA_SIZE_Y/2.0;	hiaux.at<float>(2,0) = CAMERA_SIZE_Z;
	Yiaux = Rwc*hiaux + tpose;
	pv[4].X = Yiaux.at<float>(0,0);	pv[4].Y = -Yiaux.at<float>(1,0);	pv[4].Z = -Yiaux.at<float>(2,0);
	
	// Divide by units conversion in case we need to convert to m
	pv[0].X /= UNITS_CONVERSION;	pv[0].Y /= UNITS_CONVERSION;	pv[0].Z /= UNITS_CONVERSION;
	pv[1].X /= UNITS_CONVERSION;	pv[1].Y /= UNITS_CONVERSION;	pv[1].Z /= UNITS_CONVERSION;
	pv[2].X /= UNITS_CONVERSION;	pv[2].Y /= UNITS_CONVERSION;	pv[2].Z /= UNITS_CONVERSION;
	pv[3].X /= UNITS_CONVERSION;	pv[3].Y /= UNITS_CONVERSION;	pv[3].Z /= UNITS_CONVERSION;
	pv[4].X /= UNITS_CONVERSION;	pv[4].Y /= UNITS_CONVERSION;	pv[4].Z /= UNITS_CONVERSION;
}


//*******************************************************************************
//*******************************************************************************

/** Function for drawing a RGB camera											*/
void Draw_RGB_Camera(const float Tpose[3], const float RWC[3][3])
{	
	// Fill OpenCV matrix with the translation vector
	tpose.at<float>(0,0) = Tpose[0];
	tpose.at<float>(1,0) = Tpose[1];
	tpose.at<float>(2,0) = Tpose[2];
	
	// Fill OpenCV matrix with the rotation matrix
	Rwc.at<float>(0,0) = RWC[0][0];
	Rwc.at<float>(0,1) = RWC[0][1];
	Rwc.at<float>(0,2) = RWC[0][2];
	Rwc.at<float>(1,0) = RWC[1][0];
	Rwc.at<float>(1,1) = RWC[1][1];
	Rwc.at<float>(1,2) = RWC[1][2];
	Rwc.at<float>(2,0) = RWC[2][0];
	Rwc.at<float>(2,1) = RWC[2][1];
	Rwc.at<float>(2,2) = RWC[2][2];
	
	// Origin of the RGB camera
	oriaux.at<float>(0,0) = Tpose[0]/UNITS_CONVERSION;	
	oriaux.at<float>(1,0) = -Tpose[1]/UNITS_CONVERSION; 
	oriaux.at<float>(2,0) = -Tpose[2]/UNITS_CONVERSION;
	
	// X axis of the RGB camera
	hiaux.at<float>(0,0) = CAMERA_SIZE_Z; hiaux.at<float>(1,0) = 0.0; hiaux.at<float>(2,0) = 0.0;
	xaux = Rwc*hiaux + tpose;
	xaux.at<float>(0,0) = xaux.at<float>(0,0)/UNITS_CONVERSION;	
	xaux.at<float>(1,0) = -xaux.at<float>(1,0)/UNITS_CONVERSION;
	xaux.at<float>(2,0) = -xaux.at<float>(2,0)/UNITS_CONVERSION;
	
	// Y axis of the RGB camera
	hiaux.at<float>(0,0) = 0.0; hiaux.at<float>(1,0) = CAMERA_SIZE_Z; hiaux.at<float>(2,0) = 0.0;
	yaux = Rwc*hiaux + tpose;
	yaux.at<float>(0,0) = yaux.at<float>(0,0)/UNITS_CONVERSION;	
	yaux.at<float>(1,0) = -yaux.at<float>(1,0)/UNITS_CONVERSION;
	yaux.at<float>(2,0) = -yaux.at<float>(2,0)/UNITS_CONVERSION;
	
	// Z axis of the RGB camera
	hiaux.at<float>(0,0) = 0.0; hiaux.at<float>(1,0) = 0.0; hiaux.at<float>(2,0) = CAMERA_SIZE_Z;
	zaux = Rwc*hiaux + tpose;
	zaux.at<float>(0,0) = zaux.at<float>(0,0)/UNITS_CONVERSION;	
	zaux.at<float>(1,0) = -zaux.at<float>(1,0)/UNITS_CONVERSION;
	zaux.at<float>(2,0) = -zaux.at<float>(2,0)/UNITS_CONVERSION;
	
	// Draw the lines
	Draw_One_Line(oriaux.at<float>(0,0),oriaux.at<float>(1,0),oriaux.at<float>(2,0),
	xaux.at<float>(0,0),xaux.at<float>(1,0),xaux.at<float>(2,0),SfMColor(1.0,0.0,0.0,1.0),LINE_WIDTH); // r
	
	Draw_One_Line(oriaux.at<float>(0,0),oriaux.at<float>(1,0),oriaux.at<float>(2,0),
	yaux.at<float>(0,0),yaux.at<float>(1,0),yaux.at<float>(2,0),SfMColor(0.0,1.0,0.0,1.0),LINE_WIDTH); // g
	
	Draw_One_Line(oriaux.at<float>(0,0),oriaux.at<float>(1,0),oriaux.at<float>(2,0),
	zaux.at<float>(0,0),zaux.at<float>(1,0),zaux.at<float>(2,0),SfMColor(0.0,0.0,1.0,1.0),LINE_WIDTH); // b
}

//*******************************************************************************
//*******************************************************************************

/** Function to write a Camera in OpenGL										*/
void Draw_Camera(const Vertex* pv, const SfMColor& color, bool fill)
{
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Draw_One_Line(pv[0].X,pv[0].Y,pv[0].Z, pv[1].X,pv[1].Y,pv[1].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[0].X,pv[0].Y,pv[0].Z, pv[2].X,pv[2].Y,pv[2].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[0].X,pv[0].Y,pv[0].Z, pv[3].X,pv[3].Y,pv[3].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[0].X,pv[0].Y,pv[0].Z, pv[4].X,pv[4].Y,pv[4].Z, color, LINE_WIDTH);

    Draw_One_Line(pv[1].X,pv[1].Y,pv[1].Z, pv[2].X,pv[2].Y,pv[2].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[2].X,pv[2].Y,pv[2].Z, pv[3].X,pv[3].Y,pv[3].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[3].X,pv[3].Y,pv[3].Z, pv[4].X,pv[4].Y,pv[4].Z, color, LINE_WIDTH);
    Draw_One_Line(pv[4].X,pv[4].Y,pv[4].Z, pv[1].X,pv[1].Y,pv[1].Z, color, LINE_WIDTH);
	
	// Fill the camera polygon
    if(fill)
	{
    	glColor4f(color.r, color.g, color.b, color.alpha);
					
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
		glBegin(GL_POLYGON);
			glVertex3f(pv[1].X,pv[1].Y,pv[1].Z); 
			glVertex3f(pv[2].X,pv[2].Y,pv[2].Z); 
			glVertex3f(pv[3].X,pv[3].Y,pv[3].Z); 
			glVertex3f(pv[4].X,pv[4].Y,pv[4].Z); 
		glEnd(); 
		
		glBegin(GL_POLYGON);
			glVertex3f(pv[0].X,pv[0].Y,pv[0].Z); 
			glVertex3f(pv[1].X,pv[1].Y,pv[1].Z); 
			glVertex3f(pv[2].X,pv[2].Y,pv[2].Z); 
		glEnd(); 

		glBegin(GL_POLYGON);
			glVertex3f(pv[0].X,pv[0].Y,pv[0].Z); 
			glVertex3f(pv[1].X,pv[1].Y,pv[1].Z); 
			glVertex3f(pv[4].X,pv[4].Y,pv[4].Z); 
		glEnd(); 
		
		glBegin(GL_POLYGON);
			glVertex3f(pv[0].X,pv[0].Y,pv[0].Z); 
			glVertex3f(pv[3].X,pv[3].Y,pv[3].Z); 
			glVertex3f(pv[4].X,pv[4].Y,pv[4].Z); 
		glEnd(); 
		
		glBegin(GL_POLYGON);
			glVertex3f(pv[0].X,pv[0].Y,pv[0].Z); 
			glVertex3f(pv[2].X,pv[2].Y,pv[2].Z); 
			glVertex3f(pv[3].X,pv[3].Y,pv[3].Z); 
		glEnd();
		
		glDisable(GL_BLEND);			
    }

	glDisable(GL_BLEND);	
}

//*******************************************************************************
//*******************************************************************************

/** Function to write one 3D line in OpenGL										*/
/** To write the 3D line we need to pass two 3D points 1,2						*/
void Draw_One_Line(GLfloat X1, GLfloat Y1, GLfloat Z1, GLfloat X2,
			GLfloat Y2, GLfloat Z2, const SfMColor& color, GLfloat linewidth) 
{
	glColor4f(color.r, color.g, color.b, color.alpha);
	glLineWidth(linewidth);
	glBegin(GL_LINES);
		glVertex3f(X1, Y1, Z1);
		glVertex3f(X2, Y2, Z2);
	glEnd();
}

//*******************************************************************************
//*******************************************************************************

/** Keyboard callback to change the point of view								*/
void on_keyboard(int key)
{
	// Pressed W
	if( key == 119 )
	{
		ZOOM_Z -= INCREMENTAL_Z;
	}
	// Pressed X
	else if( key == 120 )
	{
		ZOOM_Z += INCREMENTAL_Z;
	}
	// Pressed A
	else if( key == 97 )
	{
		ZOOM_X -= INCREMENTAL_X;
	}
	// Pressed D
	else if( key == 100 )
	{
		ZOOM_X += INCREMENTAL_X;
	}
	// Pressed R
	else if( key == 114 )
	{
		ZOOM_Y += INCREMENTAL_Y;
	}
	// Pressed F
	else if( key == 102 )
	{
		ZOOM_Y -= INCREMENTAL_Y;
	}	
	// Pressed Z	
	else if( key == 122 )
	{
		ORBIT_Y_ANGLE -= INCREMENTAL_ORBIT_Y_ANGLE;
		
		if( ORBIT_Y_ANGLE < -360.0 )
		{
			ORBIT_Y_ANGLE += 360.0;
		}
	}
	// Pressed C
	else if( key == 99 )
	{
		ORBIT_Y_ANGLE += INCREMENTAL_ORBIT_Y_ANGLE;
		
		if( ORBIT_Y_ANGLE > 360.0 )
		{
			ORBIT_Y_ANGLE -= 360.0;
		}
	}
	// Pressed V	
	else if( key == 118 )
	{
		ORBIT_X_ANGLE += INCREMENTAL_ORBIT_X_ANGLE;
		
		if( ORBIT_X_ANGLE > 360.0 )
		{
			ORBIT_X_ANGLE -= 360.0;
		}
	}
	// Pressed B
	else if( key == 98 )
	{
		ORBIT_X_ANGLE -= INCREMENTAL_ORBIT_X_ANGLE;
		
		if( ORBIT_X_ANGLE < -360.0 )
		{
			ORBIT_X_ANGLE += 360.0;
		}
	}
}
