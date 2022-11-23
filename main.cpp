#include <opencv2/opencv.hpp>
#include <raspicam_cv.h>
#include <iostream>
#include <chrono>
#include <ctime>

using namespace std;
using namespace cv;
using namespace raspicam;

Mat frame, matrix, framePerspective, frameGray, frameThreshold, frameEdge, frameFinal;
RaspiCam_Cv Camera;

Point2f Source[] = { Point2f(50,200),Point2f(200,200),Point2f(0,240), Point2f(360,240) };
Point2f Destination[] = { Point2f(60,0),Point2f(300,0),Point2f(60,240), Point2f(300,240) };
vector<int> histrogramLane;

// Resolution for image
void Setup(int argc, char **argv, RaspiCam_Cv &Camera)
{
    Camera.set(CAP_PROP_FRAME_WIDTH, ("-w", argc, argv, 360));
    Camera.set(CAP_PROP_FRAME_HEIGHT, ("-h", argc, argv, 240));
    Camera.set(CAP_PROP_BRIGHTNESS, ("-br", argc, argv, 50));
    Camera.set(CAP_PROP_CONTRAST, ("-co", argc, argv, 50));
    Camera.set(CAP_PROP_SATURATION, ("-sa", argc, argv, 50));
    Camera.set(CAP_PROP_GAIN, ("-g", argc, argv, 50));
    Camera.set(CAP_PROP_FPS, ("-fps", argc, argv, 100));
}

// Capture frame
void Capture()
{
    Camera.grab();
    Camera.retrieve(frame);
    cvtColor(frame, frame, COLOR_BGR2RGB);
}

// ROI and perspective vision
void Perspective()
{
	line(frame,Source[0], Source[1], Scalar(0,0,255), 2);
	line(frame,Source[1], Source[3], Scalar(0,0,255), 2);
	line(frame,Source[3], Source[2], Scalar(0,0,255), 2);
	line(frame,Source[2], Source[0], Scalar(0,0,255), 2);
	
	line(frame, Destination[0], Destination[1], Scalar(0, 255, 0), 2);
    	line(frame, Destination[1], Destination[3], Scalar(0, 255, 0), 2);
    	line(frame, Destination[3], Destination[2], Scalar(0, 255, 0), 2);
    	line(frame, Destination[2], Destination[0], Scalar(0, 255, 0), 2);

    	matrix = getPerspectiveTransform(Source, Destination);
    	warpPerspective(frame, framePerspective, matrix, Size(360, 240));
}

// Threshold operations
void Threshold()
{
    cvtColor(framePerspective, frameGray, COLOR_RGB2GRAY);
    inRange(frameGray, 200, 255, frameThreshold);
    Canny(frameGray, frameEdge, 900, 900, 3, false);
    add(frameThreshold, frameEdge, frameFinal);
    cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
    cvtColor(frameFinal, frameFinal, COLOR_RGB2BGR);
}

// Create strips to find lane
void Histrogram()
{
    histrogramLane.resize(frameFinal.size().width);
    histrogramLane.clear();

    for (int i = 0; i < frameFinal.size().width; i++)
    {
        ROILane = frameFinal(Rect(i, 10, 1, 100));
        divide(255, ROILane, ROILane);
        histrogramLane.push_back((int)(sum(ROILane)[0]));
    }
}

int main(int argc, char **argv)
{
    Setup(argc, argv, Camera);

    cout << "Connecting to camera" << endl;
    if (!Camera.open())
    {
        cout << "Failed to Connect" << endl;
        return -1;
    }

    while (1)
    {
        auto start = std::chrono::system_clock::now();

        Capture();
        Perspective();
	Threshold();
        Histrogram();
	
	// Original frame
        namedWindow("orignal", WINDOW_KEEPRATIO);
        moveWindow("orignal", 50, 100);
        resizeWindow("orignal", 640, 480);
        imshow("orignal", frame);

	// Bird's eye view frame
        namedWindow("perspective", WINDOW_KEEPRATIO);
        moveWindow("perspective", 500, 100);
        resizeWindow("perspective", 640, 480);
        imshow("perspective", framePerspective);
	    
	// Gray frame
	namedWindow("gray", WINDOW_KEEPRATIO);
        moveWindow("gray", 80, 100);
        resizeWindow("gray", 640, 480);
        imshow("gray", frameGray);
	    
	// Canny edge detection
	namedWindow("edge", WINDOW_KEEPRATIO);
        moveWindow("edge", 80, 100);
        resizeWindow("edge", 640, 480);
        imshow("edge", frameEdge);

        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsedSeconds = end - start;
        float time = elapsedSeconds.count();
        int FPS = 1 / time;

        waitKey(1);
    }

    return 0;
}