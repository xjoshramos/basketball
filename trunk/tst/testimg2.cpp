#include "cvconvnet.h"
#include <opencv/highgui.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>


//RNG rng(12345);
using namespace std;
using namespace cv;
struct prepareImage{
        bool emptySlot;
        Mat image;
	int centerY;
	int feature;
	bool fourFeature;
	
};

bool rightVertLineCheck(Mat img){

	bool four = true;
	int midLineIndex = 0;
	int lastRow = 0;
	//bool firstFlag = false;
	//for(int row = img.rows - 1; row >= 0; row--){
     	//for(int row = 0; row < img.rows; row++){
		//for(int col = img.cols; col >= 0; col--){
		//for(int col = 0; col < img.cols; col++){
			//if(img.at<uchar>(row,col) == 255 && firstFlag == false){
				//midLineIndex = col;
				//firstFlag = true;
				//cout << "four four four" << midLineIndex << endl;
				//lastRow = row;
				//break;
			//}
			//if( firstFlag == true && img.at<uchar>(row,col) == 0){
				//midLineIndex = (midLineIndex+col)/2;
				//break;
		//	}// 
	//	}
	//}
	midLineIndex = 15;
	//int prevPix = img.at<uchar>(0,midLineIndex);
	int prevPix = img.at<uchar>(0,midLineIndex);
	int lineCrossingCount = 0;
	for(int row = 0; row < img.rows; row++){
	//for(int col = 0; col < img.cols; col++){
		//lineCrossingCount = 0;
		//for(int row = 0; row < img.rows; row++){
			//midLineIndex = 13;
			if(img.at<uchar>(row,midLineIndex) != prevPix){
				prevPix = img.at<uchar>(row,midLineIndex);
				lineCrossingCount++;
			}
			//if(lineCrossingCount >= 5){
			//	four = false;
			//	break;
			//}
		//}
	}

	if(lineCrossingCount > 2){
		four = false;
	}else{
		four = true;
	}
	//imshow(" the fours image detection", img);
	//cout << " this four the four " << four << endl;
	return four;
}

Mat thinningIteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) + 
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) + 
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
    return im;
}

Mat thinning(cv::Mat& im)
{
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    } 
    while (cv::countNonZero(diff) > 0);

    im *= 255;
    return im;
}
Mat distanceTransform(Mat bwImg){ // for optimizing skeleton method TBD
	Mat dist, bwImg1;
    	distanceTransform(bwImg, dist, CV_DIST_L2, 3);
	normalize(dist, dist, 0.0, 1, NORM_MINMAX);
	//threshold(dist, bwImg1, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	inRange(dist,0,.2,bwImg);
	inRange(bwImg,0,0,bwImg);
	//bwImg = dist;
    	imshow("Distance Transform", bwImg);
	//imshow("otsud dist" , bwImg1);
	//cvtColor( dist, temp, CV_BGR2GRAY);
	return bwImg;
}

Mat polygonReconstruct(Mat img){
	
	Mat reconstructed;
	Mat temp;
	img.copyTo(temp);
	Mat drawing = Mat::zeros(img.size(), CV_8UC3 );
	vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE );
	vector<vector<Point> > approx( contours.size());
	
	for ( int i=0; i<contours.size(); i++ ) {

		approxPolyDP(cv::Mat(contours[i]), approx[i], cv::arcLength(cv::Mat(contours[i]), true) * 0.001, true);
		drawContours( drawing, contours, i, Scalar(255,255,255), -1, 8, hierarchy, 0, Point() );
		
		if(hierarchy[i][2] == -1 && hierarchy[i][3] != -1){
			//drawContours( drawing, contours, i, Scalar(0,0,0), -1, 8, hierarchy, 0, Point() );
			Scalar color = Scalar( 0, 0, 0 );
                	drawContours( drawing, approx, i, color, -1, 8, vector<Vec4i>(), 0, Point() );
			//cout << "inside contour " <<approx[i].size() << endl;
		}else{
			//cout << "outside contour " <<approx[i].size() << endl;
			//drawContours( drawing, contours, i, Scalar(255,255,255), -1, 8, hierarchy, 0, Point() );
			Scalar color = Scalar(  255, 255, 255 );
                	drawContours( drawing, approx, i, color, -1, 8, vector<Vec4i>(), 0, Point() );
		}
		//Scalar color = Scalar(  255, 0, 0 );
		//drawContours( drawing, approx, i, color, 1, 8, vector<Vec4i>(), 0, Point() );

		
        }	

	imshow("reconstructed", drawing);
	waitKey(250);
	reconstructed = drawing;
	cvtColor( reconstructed, reconstructed, CV_BGR2GRAY);
	return reconstructed;

}

Mat rotation(Mat img){

	Mat res;
	transpose(img,res);
	flip(res,res,1);
	imshow("rotataion" , res);
	waitKey(1);
	return res;
}


int rotatedBoxFeature( Mat img, int videoNom) {
        int featureNumber = 0;
	int angle = 0;
	Mat temp;
	img.copyTo(temp);	
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	vector<vector<Point> > contours;
  	vector<Vec4i> hierarchy;
  	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	vector<RotatedRect> minRect( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	{ 
		minRect[i] = minAreaRect( Mat(contours[i]) );
     	}

	int boxCount = 0;
	for( int i = 0; i< contours.size(); i++ )
     	{
		boxCount++;
       		Scalar color = Scalar( 0, 255,0);
		Scalar color1 = Scalar( 255,0,0);
       		// contour
       		drawContours( drawing, contours, i, color, -1, 8, vector<Vec4i>(), 0, Point() );
       
		/*Point2f rect_points[4]; minRect[i].points( rect_points );
		angle = minRect[i].angle;
		cout << " angle " << angle << endl;
       		for( int j = 0; j < 4; j++ )
          		line( drawing, rect_points[j], rect_points[(j+1)%4], color1, 1, 8 );
		*/
		if(hierarchy[i][2] == -1 && hierarchy[i][3] != -1){
                        Scalar color = Scalar( 0, 0, 255 );
                        drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
                }else{
                	Point2f rect_points[4]; minRect[i].points( rect_points );
                	angle = minRect[i].angle;
                	//cout << " angle " << angle << endl;
                	for( int j = 0; j < 4; j++ )
                        	line( drawing, rect_points[j], rect_points[(j+1)%4], color1, 1, 8 );
		}



     	}
	//cout << "box counter " << boxCount << endl;
	if (boxCount == 3 && videoNom != 5) {
		featureNumber = 8;
	}
	
	if( abs(angle) > 23 && abs(angle) < 75) // 22, 75
        {
		cout << " good angle = " << angle << endl; 
                featureNumber = 4;
        }
	cout << " bad angle " << angle << endl;
	imshow(" rotated angle " , drawing); 
	waitKey(100);
        return featureNumber;

}

bool checkEmptySlot( Mat img, int videoNom)
{	
	// check empty slot
	Mat checkSlot;
        copyMakeBorder(img,checkSlot,5,5,5,5,BORDER_CONSTANT,Scalar(0));
        medianBlur(checkSlot, checkSlot, 3);
        if(videoNom == 4){
	
        	inRange(checkSlot,150,255,checkSlot);
	}
	else{

		inRange(checkSlot,170,255,checkSlot);
	}

	imshow("empty check" , checkSlot);
	//waitKey(100);
	//threshold(checkSlot, checkSlot, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        bool checkEmpty = false;
	int sum = 0;
	for(int i = 0; i < checkSlot.rows; i++){
		for(int j = 0; j < checkSlot.cols; j++){
			if(checkSlot.at<uchar>(i,j) == 255)
			{
				sum++;
			}
		}
	}
        //if(checkSlot.at<uchar>(5,5) + checkSlot.at<uchar>(6,6) + checkSlot.at<uchar>(7,7)> 0)
        //{
        //        checkEmpty = true;
                //int pixel = checkSlot.at<uchar>(5,5) + checkSlot.at<uchar>(6,6);;
        //}
	int thresh;
	if( videoNom == 4) {
		thresh = 0;
	}else{
		thresh = 10;
	}
	if(sum <= thresh){
		checkEmpty = true;
	}
	return checkEmpty;
}

prepareImage numberRec(int x, int y, int width, int height, Mat frame, int videoNom){
	prepareImage image;

	Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        Mat element1 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
        Mat element2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	
	Mat timerSlot( frame, cv::Rect(x, y, width, height));
        Mat slot( timerSlot.size(), CV_8U, Scalar(0));
	timerSlot.copyTo(slot);	
	
	cvtColor( slot, slot, CV_BGR2GRAY);
	
	image.emptySlot = checkEmptySlot(slot, videoNom);
	//cout << "empty = " << image.emptySlot << endl;	
	
	Mat rotation;
	resize(slot,slot,Size(30,30));//36 36
	//resize(slot,rotation,Size(30,30));
	threshold(slot, slot, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	morphologyEx(slot, slot, cv::MORPH_CLOSE, element2); // OPEN

	//threshold(rotation, rotation, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        //morphologyEx(rotation, rotation, cv::MORPH_CLOSE, element2); // OPEN	

	image.feature = rotatedBoxFeature(slot, videoNom); // check feature for four // slot
	
	copyMakeBorder(slot,slot,7,7,7,7,BORDER_CONSTANT,Scalar(0));
	if(videoNom == 4){
		resize(slot,slot,Size(20,20)); // 16 16jxr
	}else{
		resize(slot,slot,Size(16,16));
	}
	resize(slot,slot,Size(32,32)); //best except 4 jxr
	
	
	// check empty slot
	//medianBlur(checkSlot, checkSlot, 3);
	//threshold(checkSlot, checkSlot, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//image.emptySlot = false;
        //if(checkSlot.at<uchar>(5,5) + checkSlot.at<uchar>(6,6) + checkSlot.at<uchar>(7,7)> 0)
        //{		
        //        image.emptySlot = true;
        //        int pixel = checkSlot.at<uchar>(5,5) + checkSlot.at<uchar>(6,6);;
       // }

	image.image = slot;
	
	imshow("regular slot", slot);
	waitKey(1);
	//imshow("display", image.image);
	
	return image;
}

struct colonCenter{
	bool sec;
	bool colon;
	int centerX;
	int centerY;
};


pair<int,int> twoPointCheck(Mat img) {

	pair<int,int> point;
        int XPOV [img.cols];
        int YPOV [img.rows];
        bool firstFlag = false;
	int centerX;
	int centerY;
        for(int row = 0; row < img.rows; row++){
                for(int col = 0; col < img.cols; col++){
                        
                        
                        if ( img.at<uchar>(row,col) == 255) {
                                if (firstFlag == false){
                                        centerX = col;
                                        centerY = row;
                                }
                                firstFlag = true;
				
                                XPOV[col] = 255;
				//cout <<" first" << XPOV[col] << endl;
                                YPOV[row] = 255;
                        }else{
				//cout <<" first" << XPOV[col] << endl;
                                XPOV[col] = 0;
                                YPOV[row] = 0;
                        }
                }
        }

        int prevX = 0;
        int prevY = 0;
        int crossPovX = 0;
        int crossPovY = 0;
        int totalWhiteX = 0;
        int totalWhiteY = 0;
        for (int i = 0 ; i < img.cols; i++){
		//cout <<" second" << XPOV[i] << endl;
                if( prevX != XPOV[i]){
                        crossPovX++;
                        prevX = XPOV[i];
                }
                if(XPOV[i] == 255){
                        totalWhiteX++;
                }
        }
        for(int i = 0 ; i < img.rows; i++){
                if( prevY != YPOV[i]){
                        crossPovY++;
                        prevY = YPOV[i];
                }
                if( YPOV[i] == 255){
                        totalWhiteY++;
                }
        }	
	//cout << " crossX = " << crossPovX << "crossY = " << crossPovY << " total x =" << totalWhiteX << "total y = " << totalWhiteY<< endl; 
	//if( crossPovX == 2 && crossPovX >= 2 && crossPovX <= 4 && crossPovX != 3 && totalWhiteX < 15 && totalWhiteY < 15*/){
	//	point.first = centerX;
	//	point.second = centerY;
	//}else{
	//	point.first = 0xFFFF;
	//	point.second = 0xFFFF;
	//}

	return point;
                

}

colonCenter checkForColon( int x, int y, int width, int height, Mat frame)
{
	colonCenter colonLocation;
	
	Mat fullTimerCheckGray, timerBw;
	
	vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
	
	Mat timerBox(frame, Rect(x,y,width,height) );
        
	Mat fullTimerCheck( timerBox.size(), CV_8U, Scalar(0));
        timerBox.copyTo(fullTimerCheck);
       	
        cvtColor( fullTimerCheck, fullTimerCheckGray, CV_BGR2GRAY);
        imshow( " timer timer real " , fullTimerCheckGray);
	//Size size(3,3);
  	//GaussianBlur(fullTimerCheckGray,fullTimerCheckGray,size,0);
   	//inRange(fullTimerCheckGray,100,255,timerBw);
	//adaptiveThreshold(fullTimerCheckGray, timerBw,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,75,10);
  	//bitwise_not(timerBw, timerBw);
	//imshow( "adaptive", timerBw);
	
	//medianBlur(fullTimerCheckGray, fullTimerCheckGray, 3);
	threshold(fullTimerCheckGray, timerBw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	
	
        Mat colonDrawing = Mat::zeros( fullTimerCheck.size(), CV_8UC1 );
        int colonDotsCount = 0;

        findContours( timerBw, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

        vector<Point2f> mcColon(contours.size() );
        vector<Moments> muColon(contours.size() );

        if (contours.size() == 0 ) {
		colonLocation.colon = false;
		return colonLocation;
	}
	vector<Rect> boundRect( contours.size() );
        for ( int i=0; i<contours.size(); i++ ) {
		if(contourArea(contours[i]) < 15 && contourArea(contours[i])>=1) {
			//center of colon
			boundRect[i] = boundingRect(contours[i] );
                        Point rectCheck = boundRect[i].br()- boundRect[i].tl(); 
			if( abs(abs(rectCheck.x) - abs(rectCheck.y)) < 5){
				muColon[colonDotsCount] = moments( contours[i], false );
                        	mcColon[colonDotsCount] = Point2f( muColon[colonDotsCount].m10/muColon[colonDotsCount].m00 , muColon[colonDotsCount].m01/muColon[colonDotsCount].m00 );
				
                        	drawContours( colonDrawing, contours, i, 255, -1, 8, hierarchy, 0, Point() );
                        	rectangle( colonDrawing, boundRect[i].tl(), boundRect[i].br(), 255, 2, 8, 0 ); 
				colonDotsCount++;
			}
                }

        }
	
	pair<int,int> firstColonDot;
	if (colonDotsCount == 2 || colonDotsCount == 3)
	{	
		colonLocation.colon = true;
	/*	firstColonDot = twoPointCheck(colonDrawing);
		colonLocation.centerX = firstColonDot.first+x;
		colonLocation.centerY = firstColonDot.second+y;
		if (firstColonDot.first == 0xFFFF){
			colonLocation.colon = false;
		}
		for(int z = 0; z < colonDotsCount; z++){
			if(abs(firstColonDot.first - mcColon[z].x) < 10){
				colonLocation.centerX = mcColon[z].x + x;
			}
			if(abs(firstColonDot.second - mcColon[z].y) < 10){
				colonLocation.centerY = mcColon[z].y + y;
				
			}
		}*/
		int minX = mcColon[0].x;
		int minY = mcColon[0].y;
		for(int z = 1; z < colonDotsCount; z++){
			if(minX > (int) mcColon[z].x){
				minX = mcColon[z].x;
			} 
			if( minY > (int) mcColon[z].y){
				minY = mcColon[z].y;
			}	


		}	

		colonLocation.centerX = minX + x;
		colonLocation.centerY = minY + y;
		
		if(colonDotsCount == 2 && abs(mcColon[0].x - mcColon[1].x) > 5){
		
			colonLocation.colon = false;
		}
		/*
		colonLocation.centerX = mcColon[0].x + x;	
		if(mcColon[0].y <= mcColon[1].y )
		{
			colonLocation.centerY = mcColon[0].y + y;
		}
		else 
		{
			colonLocation.centerY = mcColon[1].y + y;
		} */

	}else{
		colonLocation.colon = false;
	}
	
	if (colonDotsCount == 3){
		//colonLocation.centerY = prev;
		colonLocation.sec = true;
	} 
	if(colonDotsCount == 2){
		colonLocation.sec = false;
	}
	imshow("colon Check", colonDrawing);
        //imshow("timerbox" , fullTimerCheck);
        waitKey(1);

	return colonLocation;
}

string intToString( int t){

	std::string ch;
	ostringstream outs;
	outs << t ;
	ch = outs.str();
	return ch;

}

int main(int argc, char *argv[])
{
	
	//ofstream outFile;
	//ofstream out ("out.txt");	
	if (argc <=2 )
	{
		cerr << "Usage: " << endl << "\ttestimg <network.xml> <imagefile(s)>" << endl;
		return 1;
	}

	// Create empty net object
	CvConvNet net;

	// Load mnist.xml file into a std::string called xml
	ifstream ifs(argv[1]);
	string xml ( (istreambuf_iterator<char> (ifs)) , istreambuf_iterator<char>() );
	
	// Create network from XML string
	if ( !net.fromString(xml) )
	{
		cerr << "*** ERROR: Can't load net from XML" << endl << "Check file "<< argv[1] << endl;
		return 1;
	}

	for (int videoNom = 5; videoNom <= 5; videoNom++)
	{
	
	std::string name = "outFile" + intToString(videoNom) + ".txt";
	ofstream outFile;
 	ofstream out (name.c_str());

	VideoCapture capture(argv[videoNom]);
	
	Mat frame;
	
	int frameCount = -1;
	double sec;;
	time_t start, end;
	time(&start);
	int skip = 0;

	int x;
        int y;
        int width;
        int height;
        int pixAboveColon;
        int pixMin1;
        int pixMin2;
        int pixSec1;
        int pixSec2;
        int slotWidth;
        int slotHeight;

	int min1, min2, sec1, sec2;
	
	for( ; ;){
		
		capture >> frame;
		//skip++;
		//if( skip != 500) continue;
		//skip = 0;
		if (frame.empty()) {
			cout << " FRAME empyter" << endl;
			break;
		}
		frameCount = capture.get(CV_CAP_PROP_POS_FRAMES) - 1;

		// initialize
		switch (videoNom){
			case 3:
				x = 677;
                                y = 450;
                                width = 50;
                                height = 28;
                                pixAboveColon = 7;
                                pixMin1 = 23;
                                pixMin2 = 13;
                                pixSec1 = 4;
                                pixSec2 = 14;
                                slotWidth = 10;
                                slotHeight = 20;
				break;
			case 4:
				x = 935;
                		y = 600; //603
                		width = 75;
                		height = 25;//24
                		pixAboveColon = 7;
                		pixMin1 = 26;
                		pixMin2 = 15;
                		pixSec1 = 4;
                		pixSec2 = 17;
                		slotWidth = 13;
                		slotHeight = 24;
				break;
			case 5:
				x = 853;
                                y = 625;
                                width = 71;
                                height = 20;
                                pixAboveColon = 7;
                                pixMin1 = 30;
                                pixMin2 = 15;
                                pixSec1 = 4;
                                pixSec2 = 19;
                                slotWidth = 15;
                                slotHeight = 20;
				break;


		}
		//skip++;
		//if( skip != 2000) continue;
		//skip = 0;
		
		// check for colon for timer on or off, tbd vertical check
		colonCenter locationOfColon = checkForColon(x, y, width, height, frame);
		
		if(locationOfColon.colon == false) {
			cout << "NoTIMER NoTimer,  fps = " << endl;
			out <<"frame:" << frameCount << ", gc: " <<  "NO TIMER " << "," << "\n";
			continue;
		}
		// arg 4  935, 603, 75, 24
		prepareImage secSlot1 = numberRec(locationOfColon.centerX+pixSec1, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage secSlot2 = numberRec(locationOfColon.centerX+pixSec2, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage minSlot2 = numberRec(locationOfColon.centerX-pixMin2, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage minSlot1 = numberRec(locationOfColon.centerX-pixMin1, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);          
		/*
		// video arg 2
		prepareImage secSlot1 = numberRec(910+locationOfColon.centerX+3, 602, 13, 24, frame);
		prepareImage secSlot2 = numberRec(910+locationOfColon.centerX+15, 602, 14, 24, frame);
		prepareImage minSlot2 = numberRec(910+locationOfColon.centerX-15, 602, 14, 24, frame);
		prepareImage minSlot1 = numberRec(910+locationOfColon.centerX-27, 602, 14, 24, frame);
		*/

		IplImage* imgMin1 = NULL;
		imgMin1 = cvCreateImage(cvSize(32,32),8,1);
        	IplImage ipltemp1 = minSlot1.image;
        	cvCopy(&ipltemp1,imgMin1);

		IplImage* imgMin2 = NULL;
                imgMin2 = cvCreateImage(cvSize(32,32),8,1);
                IplImage ipltemp2 = minSlot2.image;
                cvCopy(&ipltemp2,imgMin2);

		IplImage* imgSec1 = NULL;
		imgSec1 = cvCreateImage(cvSize(32,32),8,1);
                IplImage ipltemp3 = secSlot1.image;
                cvCopy(&ipltemp3,imgSec1);

		IplImage* imgSec2 = NULL;
		imgSec2 = cvCreateImage(cvSize(32,32),8,1);
                IplImage ipltemp4 = secSlot2.image;
                cvCopy(&ipltemp4,imgSec2);
		
		
		if(minSlot1.emptySlot == true || locationOfColon.centerX - pixMin1 < x){
                        min1 = 0;
                }
                if(minSlot1.emptySlot == false  && locationOfColon.centerX - pixMin1 >= x){
                        min1 = (int) net.fprop(imgMin1);
                }
		if(/*minSlot2.emptySlot == true || */locationOfColon.centerX - pixMin2 < x){
                        min2 = 0;
                }
                if(/*minSlot2.emptySlot == false && */locationOfColon.centerX - pixMin2 >= x){
                        min2 = (int) net.fprop(imgMin2);
                }
		
		//min2 = (int) net.fprop(imgMin2);
		sec1 = (int) net.fprop(imgSec1);
		sec2 = (int) net.fprop(imgSec2);
		

		if (minSlot1.feature > 0 && minSlot1.emptySlot != true){ // fix jxr
			min1 = minSlot1.feature;
		}
		if (minSlot2.feature > 0){
			min2 = minSlot2.feature;
                }
		if (secSlot1.feature > 0){
			sec1 = secSlot1.feature;
                }
		if (secSlot2.feature > 0){
			sec2 = secSlot2.feature;
                }
		if (locationOfColon.sec == true){
                        min1 = 0;
                        min2 = 0;
                }
		//class prediction
			
		ostringstream val1, val2, val3, val4;
		val1 << (int) min1;
		val2 << (int) min2;
		val3 << (int) sec1;
		val4 << (int) sec2;
		
		Mat display(frame, cv::Rect(x, locationOfColon.centerY - pixAboveColon, width, height));
		Mat displayTime( display.size(), CV_8U, Scalar(0));
        	display.copyTo(displayTime);
		copyMakeBorder(displayTime,displayTime,20,20,20,20,BORDER_CONSTANT,Scalar(0));		
		
                
		putText(displayTime, val1.str().c_str(), Point(0, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val2.str().c_str(), Point(10, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val3.str().c_str(), Point(20, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val4.str().c_str(), Point(30, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
		
		imshow( "Timer Recognition" , displayTime);
		waitKey(1);
		
		time(&end);
                sec = difftime(end, start);
		cout << min1 << min2 << ":" << sec1 << sec2 << ", sec = " << sec << endl;
                out <<"frame:" << frameCount << ", gc: " << min1 << min2<< ":" << sec1 << sec2 << ","<< " sec:" << sec << " ," << "\n";
		
		cvReleaseImage(&imgSec1);
		cvReleaseImage(&imgSec2);
		cvReleaseImage(&imgMin1);
		cvReleaseImage(&imgMin2);	
	}

	out.close();
	}
	return 0;
}
