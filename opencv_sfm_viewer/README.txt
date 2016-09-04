
OPENCV_SFM_VIEWER_1_0
***************************************************************************************************************************
***************************************************************************************************************************

This program is a simple 3D Structure from Motion (SfM) viewer for OpenCV

The program allows to display sparse 3D reconstructions or dense ones allowing to
save images or video files while moving the camera view

By means of the keyboard you can change between different image views and zooms

***************************************************************************************************************************
***************************************************************************************************************************

Library Dependencies Installation
---------------------------------------------------------------------------------------------------------------------------
The program only requires that OpenCV is correctly installed with the Qt and OpenGL
options active

To check that Qt and Qt_OpenGL are available, after cmake ., type ccmake . in the 
OpenCV installation folder. Then set to ON the options with QT and with QT_OPENGL
Then compile and install OpenCV libraries:
make
sudo make intall

Under UNIX systems you should need to install at least the following packages: 
cmake, ccmake, freeglut3-dev and libqt4-opengl-dev or posterior versions

The OpenCV version should be at least 2.2.0 to work properly

***************************************************************************************************************************
***************************************************************************************************************************

Compiling the program
---------------------------------------------------------------------------------------------------------------------------
In the program folder please type:
cmake .
make

The CMakeLists.txt file only has dependencies with respect to OpenCV and OpenGL 
libraries. You should edit this file in case your libraries are not installed in
common default paths

***************************************************************************************************************************
***************************************************************************************************************************

Running the program
--------------------------------------------------------------------------------------
The program accepts command line arguments. By default, there are two different 
running options

./opencv_sfm_viewer dense_scene_sfm.txt

opencv_sfm_viewer is the name of the program and dense_scene_sfm.txt is a .txt file
that contains the information about the 3D reconstruction (camera poses and 3D points)
to be displayed

If you want to save an especific image viewpoint, press the key 's' and automatically
the image will be saved in the output folder

./opencv_sfm_viewer dense_scene_sfm.txt video_output.avi

By adding a third input argument, all the visualization will be grabbed to a video 
file named video_output.avi

***************************************************************************************************************************
***************************************************************************************************************************

Format of the 3D SfM file
---------------------------------------------------------------------------------------------------------------------------
The format about the 3D SfM file to be visualized consists of several text rows that
contain the information of the 3D points and camera poses from the reconstruction

For example, for the 3D points:
POINT3 734.777 -2933.03 5471.6 0.337255 0.337255 0.337255

POINT3 is a tag, the next three values are the location of the point with respect
to a world coordinate frame (xyz), and the last three values are the rgb color
of the point

By defaults the units of the 3D points are expressed in mm. Then in the program, this
units are converted to m

For example, for the 3D camera poses:
POSE3 201.792 0.412413 1617.32 0.579996 -0.00303059 -0.814613 0.0350088 0.999162 0.0212087 0.813866 -0.0408196 0.579616

POSE3 is a tag, the first three values correspond to the camera pose location in world coordinate frame
The next 9 values correspond to the values of the rotation matrix RWC (from camera to world coordinate frame)
RWC = [r11 r12 r13; r21 r22 r23; r31 r32 r33]

***************************************************************************************************************************
***************************************************************************************************************************

Keyboard Options
---------------------------------------------------------------------------------------------------------------------------

In the visualization window, you can change the camera viewpoint by means of pressing the following keys:

'q' --> Quit and finish the program
's' --> Save the current image view in the output folder
'w' --> Zoom +Z
'x' --> Zoom -Z
'a' --> Zoom +X
'd' --> Zoom -X
'r' --> Zoom +Y
'f' --> Zoom -Y
'z' --> Incremental +yaw angle 
'c' --> Incremental -yaw angle 
'v' --> Incremental +pitch angle 
'b' --> Incremental -pitch angle 

***************************************************************************************************************************
***************************************************************************************************************************

Coordinate Frames
---------------------------------------------------------------------------------------------------------------------------

The Figure coordinate_frames.bmp displays the assumed coordinate frames for the 3D SfM reconstruction and the OpenGL 
visualization

***************************************************************************************************************************
***************************************************************************************************************************

More Info
---------------------------------------------------------------------------------------------------------------------------

The Figure coordinate_frames.bmp displays the assumed coordinate frames for the 3D SfM reconstruction and the OpenGL 
visualization


Name: opencv_sfm_viewer_1_0
Authors: Pablo F. Alcantarilla, Kai Ni
Date: 23 / 11 / 2011
Contact: pablofdezalc@gmail.com





