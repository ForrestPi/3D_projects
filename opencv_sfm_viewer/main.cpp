
//=============================================================================
//
// main.cpp
// Main file for the opencv_sfm_viewer application
// Authors: Pablo F. Alcantarilla, Kai Ni
// Date: 26/08/2011
// Email: pablo.alcantarilla@depeca.uah.es
//=============================================================================

#include "main.h"

// Namespaces
using namespace std;

//******************************************************************************
//******************************************************************************

/** Main Function */
int main( int argc, char *argv[] )
{
	char inputFile[NMAX_CHARACTERS], sfmFile[NMAX_CHARACTERS], videoFile[NMAX_CHARACTERS];
	char key = 0;
	int mode = 0, nimages = 0;
	bool save_video = false;
	SfM_data data;
	CvVideoWriter *sfm_video;
	IplImage *img_map_c = NULL;
	
	if( argc < 2 )
	{
		cout << "Error introducing input arguments!!" << endl;
		cout << "The format is: ./opencv_sfm_viewer sfm_data.txt video_sfm.avi" << endl;
		cout << "You need to pass the .txt file with the structure+motion data!!" << endl;
		cout << "If desired you can also pass a video file for grabbing a video" << endl;
		return 1;
	}
	
	// Copy the input file with the structure and motion data
	strcpy(inputFile,argv[1]);
	
	if( argc == 3 )
	{
		strcpy(videoFile,argv[2]);
		sfm_video = cvCreateVideoWriter(videoFile,CV_FOURCC('D','I','V','X'),VIDEO_FRATE,cvSize(IMG_WIDTH,IMG_HEIGHT),1);
		save_video = true;
	}
	
	// This window is for the OpenGL visualization	
	cvNamedWindow("OpenCV SfM Viewer",CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	
	// Now we will create the OpenGL callback for this OpenCV window
	cv::createOpenGLCallback("OpenCV SfM Viewer",on_opengl,(void *)&data);
	
	// Allocate memory for the OpenCV image
	img_map_c = cvCreateImage(cvSize(IMG_HEIGHT,IMG_WIDTH),IPL_DEPTH_8U,3);
	
	// Load the structure and motion data
	Load3D(inputFile,data);
	
	while( mode == 0 )
	{
	   cvZero(img_map_c);
	   cvShowImage("OpenCV SfM Viewer",img_map_c);
       cvMoveWindow("OpenCV SfM Viewer",WPOSX,WPOSY);
	   
	   key = cvWaitKey(10);
	   
	   // Check input key to save the image or finish the viewer
	   // Save image
	   if( key == 's')
	   {
		   sprintf(sfmFile,"./output/image%05d.jpg",nimages);
		   cvZero(img_map_c);
		   OpenGL_to_OpenCV_Image(img_map_c);
		   cvSaveImage(sfmFile,img_map_c);
		   cout << "Image: " << sfmFile << " saved!!" << endl;
		   nimages++;
	   }
	   // Finish the program
	   else if( key == 'q' )
	   {
		    mode = 1;
			break;
	   }
	   else
	   {
		   on_keyboard((int)key);
	   }
	   
	   // Append the OpenCV image to the video 
	   if( save_video == true )
	   {
		   cvZero(img_map_c);
		   OpenGL_to_OpenCV_Image(img_map_c);
		   cvWriteFrame(sfm_video,img_map_c);
	   }
	}
	
	// Destroy the OpenCV window and release allocated memory
	cvDestroyWindow("OpenCV SfM Viewer");
	cvReleaseImage(&img_map_c);
	
	if( save_video == true )
	{
		cvReleaseVideoWriter(&sfm_video);
	}
	
}

//******************************************************************************
//******************************************************************************