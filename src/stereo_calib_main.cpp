/*
 * main.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: loyd-hook
 */


#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include "opencv2/contrib/contrib.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>


using namespace cv;
using namespace std;
#define NUMIMGS 24
#define SET	6

vector<vector<Point3f> > objectPoints;
vector<vector<Point2f> > imagePoints1;
vector<vector<Point2f> > imagePoints2;
vector<Point2f> corners1;
vector<Point2f> corners2;
int success = 0;
int board_w = 9;
int board_h = 6;

void FindStereoChessBoardCorners(Mat imgl, Mat imgr)
{
	Size board_sz = Size(board_w, board_h);

	int board_n = board_w*board_h;

	vector<Point3f> obj;
	bool found1 = false;
	bool found2 = false;

	for(int i = 0; i < board_h; i++)
	{
		for(int j =0; j < board_w; j++)
		{
			obj.push_back(Point3f((board_w-j-1)*2.7, (board_h-i-1)*2.7, 0.0f));
		}
	}
	//cvtColor(img, gray, CV_BGR2GRAY);
	found1 = findChessboardCorners(imgl, board_sz, corners1, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	found2 = findChessboardCorners(imgr, board_sz, corners2, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

	if(found1 &&  found2)
	{
		imagePoints1.push_back(corners1);
		imagePoints2.push_back(corners2);
		objectPoints.push_back(obj);
		printf("Corners stored\n");
		success++;
		cornerSubPix(imgl, corners1, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(imgl, board_sz, corners1, found1);
		cornerSubPix(imgr, corners2, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(imgr, board_sz, corners2, found2);
	}else
	{
		cout << "Corners not found" << endl;
	}

//	namedWindow("corners", 0);
//	imshow("corners", img);
}

int main()
{
	char limg_file[200], rimg_file[200];
	Mat limg[NUMIMGS];
	Mat rimg[NUMIMGS];
	Mat loimg[NUMIMGS];
	Mat roimg[NUMIMGS];
	Size image_size;

	Mat CM1;// = Mat(3, 3, CV_64FC1);
	Mat CM2;// = Mat(3, 3, CV_64FC1);
	Mat D1, D2;
	Mat CMU1, CMU2, DU1, DU2;
	Mat R, T, E, F;

	//Pull in the intrinsic camera calibration for the left and right camera
	FileStorage fs("/home/loyd-hook/0Projects/SV/software/eclipse_ws/CameraCalibration/Debug/mycalib.yml", FileStorage::READ);
	if(!fs.isOpened())
	{
		printf("Failed to open file mycalib on left image");
		exit (-1);
	}
	fs["M1"] >> CM1;
	fs["D1"] >> D1;
	fs["M2"] >> CM2;
	fs["D2"] >> D2;
	fs.release();
	for(int i = 0; i < NUMIMGS; i++)
	{

		sprintf(limg_file, "/home/loyd-hook/0Projects/SV/Images/DevelopmentPics%02d/SD%02d_O%04d_0.bmp", SET, SET, i);
		sprintf(rimg_file, "/home/loyd-hook/0Projects/SV/Images/DevelopmentPics%02d/SD%02d_O%04d_1.bmp", SET, SET, i);
		//sprintf(limg_file, "/home/loyd-hook/0Projects/SV/Cal_Images/StereoCalImages/l_%d.bmp", i+1);
		//sprintf(rimg_file, "/home/loyd-hook/0Projects/SV/Cal_Images/StereoCalImages/r_%d.bmp", i+1);
		cout << "Reading " << limg_file << endl;
		cout << "Reading " << rimg_file << endl;
		loimg[i] = imread(limg_file, 0);
		roimg[i] = imread(rimg_file, 0);
		if (loimg[i].empty() || roimg[i].empty())
		{
			printf("Command-line parameter error: could not load the file with i = %d\n", i);
			exit(-1);
		}
		Size img_size(640,480);
		resize(loimg[i], limg[i], img_size);
		resize(roimg[i], rimg[i], img_size);
		image_size = img_size;
		//We have to undistort the images first becuase the wide field of view of the cameras is messing up the
		//Stereo calibration
		//undistort(limg[i], luimg[i], CM1, D1);
		//undistort(rimg[i], ruimg[i], CM2, D2);
		//if(i == 0 || i == 1 || i == 8 || i == 12 || i == 14) continue;
		FindStereoChessBoardCorners(limg[i], rimg[i]);
	}


	//Pull in the intrinsic camera calibration for the left and right camera
	FileStorage fus("/home/loyd-hook/0Projects/SV/software/eclipse_ws/CameraCalibration/Debug/mycalibu.yml", FileStorage::READ);
	if(!fus.isOpened())
	{
		printf("Failed to open file mycalib on left image");
		exit (-1);
	}
	fus["M1"] >> CMU1;
	fus["D1"] >> DU1;
	fus["M2"] >> CMU2;
	fus["D2"] >> DU2;
	fus.release();
//	//cout << CM1 << D1 << endl;
//	//Pull in the intrinsic camera calibration for the right camera
//	fs.open("/home/loyd-hook/0Projects/SV/Cal_Images/Single_Camera_Calibration_Images/Right Camera/mycalib.yml", FileStorage::READ);
//	if(!fs.isOpened())
//	{
//		printf("Failed to open file mycalib on right image");
//		exit (-1);
//	}
//	fs["M1"] >> CM2;
//	fs["D1"] >> D2;
//	fs.release();


	//	fs["M2"] >> M2;
	//	fs["D2"] >> D2;
//	CM1.at<float>(0, 0) = 1;
//	CM1.at<float>(1, 1) = 1;
//	CM2.at<float>(0, 0) = 1;
//	CM2.at<float>(1, 1) = 1;

	double ret = stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
			CM1, D1, CM2, D2, image_size, R, T, E, F,
//			CV_CALIB_FIX_ASPECT_RATIO +
//								CV_CALIB_ZERO_TANGENT_DIST +
//								CV_CALIB_SAME_FOCAL_LENGTH +
//								CV_CALIB_RATIONAL_MODEL +
//								CV_CALIB_FIX_K3 + CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5,
			//CV_CALIB_FIX_INTRINSIC |
			CV_CALIB_SAME_FOCAL_LENGTH | CV_CALIB_ZERO_TANGENT_DIST,
			//CV_CALIB_FIX_INTRINSIC | CV_CALIB_FIX_ASPECT_RATIO,
			TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5));
//	double ret = stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
//			CMU1, DU1, CMU2, DU2, image_size, R, T, E, F,
//			CV_CALIB_FIX_INTRINSIC | CV_CALIB_SAME_FOCAL_LENGTH | CV_CALIB_ZERO_TANGENT_DIST,
//			TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5));
//	stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
//					CM1, D1, CM2, D2, img1.size(), R, T, E, F,
//					CV_CALIB_FIX_ASPECT_RATIO +
//					CV_CALIB_ZERO_TANGENT_DIST +
//					CV_CALIB_SAME_FOCAL_LENGTH +
//					CV_CALIB_RATIONAL_MODEL +
//					CV_CALIB_FIX_K3 + CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5,TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5));

	Mat R1, R2, P1, P2, Q;
	printf("return = %f\n", ret);
	stereoRectify(CM1, D1, CM2, D2, image_size, R, T, R1, R2, P1, P2, Q);
	FileStorage fs1("mystereocalib.yml", FileStorage::WRITE);
	fs1 << "CM1" << CM1;
	fs1 << "CM2" << CM2;
	fs1 << "D1" << D1;
	fs1 << "D2" << D2;
	fs1 << "R" << R;
	fs1 << "T" << T;
	fs1 << "E" << E;
	fs1 << "F" << F;
	fs1 << "R1" << R1;
	fs1 << "R2" << R2;
	fs1 << "P1" << P1;
	fs1 << "P2" << P2;
	fs1 << "Q" << Q;

	fs1.release();

	Mat map1x, map1y, map2x, map2y;
	Mat lrimg[NUMIMGS], rrimg[NUMIMGS];
	char img_name[200];
	for(int i = 0; i < NUMIMGS; i++)
	{

		initUndistortRectifyMap(CM1, D1, R1, P1, image_size, CV_32FC1, map1x, map1y);
		initUndistortRectifyMap(CM2, D2, R2, P2, image_size, CV_32FC1, map2x, map2y);

		remap(limg[i], lrimg[i], map1x, map1y, INTER_LINEAR, BORDER_CONSTANT, Scalar());
		remap(rimg[i], rrimg[i], map2x, map2y, INTER_LINEAR, BORDER_CONSTANT, Scalar());

		ostringstream ssr, ssl, this_set;
		this_set << setw(2) << setfill('0') << SET;
		ssl << "/home/loyd-hook/0Projects/SV/Images/DevelopmentPics"<< this_set.str() <<"/SD" << this_set.str() <<"_R" << setw(4) << setfill('0') << i << "_0.bmp"; // see wiki for naming convention
		string left_camera_file_name = ssl.str();
		imwrite(left_camera_file_name, lrimg[i]);
		cout << "Left image stored at" << left_camera_file_name << endl;

		ssr << "/home/loyd-hook/0Projects/SV/Images/DevelopmentPics"<< this_set.str() <<"/SD" << this_set.str() <<"_R" << setw(4) << setfill('0') << i << "_1.bmp"; // see wiki for naming convention
		string right_camera_file_name = ssr.str();
		imwrite(right_camera_file_name, rrimg[i]);
		cout << "Right image stored at" << ssr.str() << endl;

//		sprintf(img_name, "orig_image_l_%d", i);
//		namedWindow(img_name, 0);
//		imshow(img_name, limg[i]);
//		sprintf(img_name, "orig_image_r_%d", i);
//		namedWindow(img_name, 0);
//		imshow(img_name, rimg[i]);
//		sprintf(img_name, "undist_image_l_%d", i);
//		namedWindow(img_name, 0);
//		imshow(img_name, luimg[i]);
//		sprintf(img_name, "undist_image_r_%d", i);
//		namedWindow(img_name, 0);
//		imshow(img_name, ruimg[i]);
	}
	printf("press any key to continue...");
	fflush(stdout);
	waitKey();
	printf("\n");

	return 0;
}

