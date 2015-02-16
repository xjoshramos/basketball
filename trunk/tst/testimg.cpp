#include "cvconvnet.h"
#include <opencv/highgui.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
#define DISPLAYDEBUG 0

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

struct colonCenter{
        bool sec;
        bool colon;
        int centerX;
        int centerY;
};


int rotatedBoxFeature( Mat img, int videoNom) {
        int featureNumber = 0;
	int angle = 0;
	Mat temp;
	img.copyTo(temp);	
	
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	Mat hullImg = Mat::zeros( temp.size(), CV_8UC3 );
	Mat rotatedHull = Mat::zeros( temp.size(), CV_8UC3 );

	vector<vector<Point> > contours;
  	vector<Vec4i> hierarchy;
  	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
	vector<RotatedRect> minRect( contours.size() );
	vector<RotatedRect> minRectHull( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	{ 
		minRect[i] = minAreaRect( Mat(contours[i]) );
     	}
	
	vector<vector<Point> >hull( contours.size() );
	int distanceSlopeCheck = 0;
	int boxCount = 0;
	for( int i = 0; i< contours.size(); i++ )
     	{
		convexHull( Mat(contours[i]), hull[i], false ); //find hull
		minRectHull[i] = minAreaRect( Mat(hull[i]) );
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
			// hull drawing 
			Scalar color = Scalar( 0,255,0);
        		drawContours( hullImg, contours, i, Scalar(255,255,255), -1, 8, vector<Vec4i>(), 0, Point() );
			drawContours( rotatedHull, contours, i, Scalar(255,255,255), -1, 8, vector<Vec4i>(), 0, Point() );
        		drawContours( hullImg, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
			drawContours( rotatedHull, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );



			int angle1 = minRectHull[i].angle;
			//rotated hull
			Point2f rect_pointsHull[4]; minRectHull[i].points( rect_pointsHull );
                        for( int j = 0; j < 4; j++ ){
                                line( rotatedHull, rect_pointsHull[j], rect_pointsHull[(j+1)%4], color1, 1, 8 );
			}
			
			// regular rotation
                	Point2f rect_points[4]; minRect[i].points( rect_points );
                	angle = minRect[i].angle;
                	cout << " regular angle " << angle << "  hull angle = " << angle1 << "  hull size " << hull.size() << endl;
                	for( int j = 0; j < 4; j++ ){
                        	line( drawing, rect_points[j], rect_points[(j+1)%4], color1, 1, 8 );
			}



		}



     	}

	for(int k = 0; k < img.rows; k++){
		if (img.at<uchar>(k,k) == 255)
               	{
                      distanceSlopeCheck = k;
                      break;
               	}
        }	
	cout << "chekcing slop " << distanceSlopeCheck << endl;
	//cout << "box counter " << boxCount << endl;
	//if (boxCount == 3 && videoNom != 5) {
	//	featureNumber = 8;
	//}
	
	//if( abs(angle) > 23 && abs(angle) < 75) // 22, 75
        //{
		//cout << " good angle = " << angle << endl; 
        //        featureNumber = 4;
        //}
	//cout << " bad angle " << angle << endl;
	
	if (DISPLAYDEBUG == 1){
		imshow("rotated hull" , rotatedHull);
		imshow("hull img" , hullImg);
		imshow(" rotated angle " , drawing); 
		waitKey(1);
	}

        return featureNumber;

}

bool checkEmptySlot( bool white, Mat img, int videoNom)
{	
	// check empty slot
	Mat checkSlot;
	if(white == false){

       		copyMakeBorder(img,checkSlot,5,5,5,5,BORDER_CONSTANT,Scalar(0));
        }else{
		copyMakeBorder(img,checkSlot,5,5,5,5,BORDER_CONSTANT,Scalar(255));
	}
	medianBlur(checkSlot, checkSlot, 3);
        /*if(videoNom == 4){
	
        	inRange(checkSlot,150,255,checkSlot);
	}
	else{

		inRange(checkSlot,170,255,checkSlot);
	}*/
	if( white == false){
		inRange(checkSlot,150,255,checkSlot);
	}else{
		inRange(checkSlot,0,150,checkSlot);

	}

	if(DISPLAYDEBUG == 1){

		imshow("empty check" , checkSlot);
	}
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
		thresh = 20;
	}
	if(sum <= thresh){
		checkEmpty = true;
	}
	return checkEmpty;
}

prepareImage numberRec(bool white, int x, int y, int width, int height, Mat frame, int videoNom)
{
	
	prepareImage image;

	Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        Mat element1 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
        Mat element2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	
	Mat timerSlot( frame, cv::Rect(x, y, width, height));
        Mat slot( timerSlot.size(), CV_8U, Scalar(0));
	timerSlot.copyTo(slot);	
	
	cvtColor( slot, slot, CV_BGR2GRAY);
	
	image.emptySlot = checkEmptySlot(white, slot, videoNom);
	//cout << "empty = " << image.emptySlot << endl;	
	
	Mat rotation;
	resize(slot,slot,Size(width*2,height*2));//36 36
	resize(slot,rotation,Size(width*(2),height*(2)));
	//resize(slot,rotation,Size(35,40));
	

	//medianBlur(slot, slot, 3);
	if(white == false){
		/*
		Mat gray;
                copyMakeBorder(slot,gray,10,10,10,10,BORDER_CONSTANT,Scalar(255));
                threshold(gray, gray, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                Mat cropedImage = gray(Rect(10,10,width,height));
                slot = cropedImage;
		rotation = slot;
		*/

		threshold(slot, slot, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		threshold(rotation, rotation, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		//inRange(rotation,150,255,rotation);
	}else{
		/*
		Mat gray1;
                copyMakeBorder(slot,gray1,5,5,5,5,BORDER_CONSTANT,Scalar(0));
                threshold(gray1, gray1, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
		imshow("grayyyyyy" , gray1);
                //Mat cropedImage1 = gray1(Rect(5,5,width,height));
		//imshow("cropppeddsfasd" , cropedImage1);
                slot = gray1;
		Mat cropedImage1 = slot(Rect(5,5,width,height));
		imshow("fksdjflkasjdklfja;l" , cropedImage1);
                rotation = slot;

		*/
		//inRange(rotation,150,255,rotation);
		threshold(rotation, rotation, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
		threshold(slot, slot, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);	
	}

	morphologyEx(slot, slot, cv::MORPH_CLOSE, element2); // OPEN
	// turn on for observation
	//threshold(rotation, rotation, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
       	morphologyEx(rotation, rotation, cv::MORPH_ERODE, element); // OPEN	

	image.feature = rotatedBoxFeature(rotation, videoNom); // check feature for four // slot
	
	copyMakeBorder(slot, slot, width/2, width/2, height/2, height/2, BORDER_CONSTANT,Scalar(0)); // 7
	//if(videoNom == 4){
		resize(slot,slot,Size(20,20)); // 16 16jxr // 20 20
	//}else{
		resize(slot,slot,Size(width*2,height*2)); // 16 16 // 15 25
	//}
	resize(slot,slot,Size(32,32)); //best except 4 jxr
	//medianBlur(slot, slot, 3);	
	image.image = slot;
	
	if( DISPLAYDEBUG == 1){
		imshow("regular slot", slot);
		waitKey(1);
		//imshow("display", image.image);
	}
	
	return image;
}


colonCenter checkForColon( bool white, int x, int y, int width, int height, Mat frame, int slotWidth, int slotHeight, int pixAboveColon)
{	
	colonCenter colonLocation;
	
	Mat fullTimerCheckGray, timerBw;
	
	vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
	
	Mat timerBox(frame, Rect(x,y,width,height) );
        
	Mat fullTimerCheck( timerBox.size(), CV_8U, Scalar(0));
        timerBox.copyTo(fullTimerCheck);
       	
        cvtColor( fullTimerCheck, fullTimerCheckGray, CV_BGR2GRAY);
        
	if( DISPLAYDEBUG == 1){
		imshow( " timer timer real " , fullTimerCheckGray);
	}

	//Size size(3,3);
  	//GaussianBlur(fullTimerCheckGray,fullTimerCheckGray,size,0);
   	//inRange(fullTimerCheckGray,100,255,timerBw);
	//adaptiveThreshold(fullTimerCheckGray, timerBw,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,75,10);
  	//bitwise_not(timerBw, timerBw);
	//imshow( "adaptive", timerBw);
	//medianBlur(fullTimerCheckGray, fullTimerCheckGray, 3);
	if(white == false){
		/*Mat gray;
		copyMakeBorder(fullTimerCheckGray,gray,10,10,10,10,BORDER_CONSTANT,Scalar(255));
		threshold(gray, gray, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		Mat cropedImage = gray(Rect(10,10,width,height));
		timerBw = cropedImage;*/
		threshold(fullTimerCheckGray, timerBw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	}else{
	
		/*Mat gray;
                copyMakeBorder(fullTimerCheckGray,gray,10,10,10,10,BORDER_CONSTANT,Scalar(0));
                threshold(gray, gray, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
                Mat cropedImage = gray(Rect(10,10,width,height));
                timerBw = cropedImage;*/
		threshold(fullTimerCheckGray, timerBw, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	}

	if(DISPLAYDEBUG == 1){
		imshow( " original threshold" , timerBw);
		waitKey(1);	
	}

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
	if (colonDotsCount == 2 || colonDotsCount == 3 || colonDotsCount == 1)
	{	
		// colon on if two or three dots detected 
		colonLocation.colon = true;
		
		int minX = mcColon[0].x;
		int minY = mcColon[0].y;

		// get top left center dots location from all centers of contours 
		for(int z = 1; z < colonDotsCount; z++){
			if(minX > (int) mcColon[z].x){
				minX = mcColon[z].x;
			} 
			if( minY > (int) mcColon[z].y){
				minY = mcColon[z].y;
			}	


		}	

		// colon location always defaults to top left dot 
		colonLocation.centerX = minX + x;
		colonLocation.centerY = minY + y;
		
		if(colonDotsCount == 1){
			colonLocation.centerX = minX + x - 40;
                	colonLocation.centerY = minY + y - slotHeight + 14;
		}

		// check colon dots are vertical on top of each other 
		if(colonDotsCount == 2 && abs(mcColon[0].x - mcColon[1].x) > 5){
			colonLocation.colon = false;
		}

		//tbd checks for three dots 

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
		// timer is off is colon is not detected
		colonLocation.colon = false;
	}
	

	// check for three dots, if 3 dots then secs timer is on, so min slots are equal to 0
	if (colonDotsCount == 3 || colonDotsCount == 1){
		colonLocation.sec = true; // fix if needed
	}
 
	if(colonDotsCount == 2){
		colonLocation.sec = false;
	}

	if (DISPLAYDEBUG == 1){
		imshow("colon Check", colonDrawing);
        	//imshow("timerbox" , fullTimerCheck);
        	waitKey(1);
	}

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

	for (int videoNom = 3; videoNom <= 6; videoNom++)
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
	bool white;

	int min1, min2, sec1, sec2;
	
	for( ; ;){
		
		capture >> frame;
		//skip++;
		//if( skip != 7001) continue;
		//skip = 7000;
		if (frame.empty()) {
			cout << " FRAME empyter" << endl;
			break;
		}
		frameCount = capture.get(CV_CAP_PROP_POS_FRAMES) - 1;
		
		// initialize parameters, box around timer
		switch (videoNom){
			case 2:
                                x = 910;
                                y = 602;
                                width = 75;
                                height = 25;
                                pixAboveColon = 7;
                                pixMin1 = 26;
                                pixMin2 = 14;
                                pixSec1 = 4;
                                pixSec2 = 14;
                                slotWidth = 12;
                                slotHeight = 24;
				white = false;
                                break;
			case 3:
				x = 677;// top right hand corner (x,y) 
                                y = 450;
                                width = 50; // width, height of timer box
                                height = 28;
                                pixAboveColon = 7; // vertical distance above colon
                                pixMin1 = 23; // distance away from colon for each slot , only x direction
                                pixMin2 = 12; //13
                                pixSec1 = 4;
                                pixSec2 = 14;
                                slotWidth = 10; // width and height of slot 
                                slotHeight = 20;
				white = false;
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
				white = false;
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
				white = false;
				break;
			case 6:
                                x = 985;
                                y = 605;
                                width = 85;
                                height = 25;
                                pixAboveColon = 9;
                                pixMin1 = 31;
                                pixMin2 = 17;
                                pixSec1 = 4;
                                pixSec2 = 19;
                                slotWidth = 15;
                                slotHeight = 25;
				white = true;
                                break;




		}
		
		//skip++;
		//if( skip != 5001) continue;
		//skip = 5000;
		/*	
		if(DISPLAYDEBUG == 1){
			
			Mat hsv;
			Mat gray;
			vector<Mat> channels;

			Mat otsuimg;
			Mat ranger;
			Mat hsvOtsu;
			Mat adapt;

                        //imshow("real frame", frame);
                        //waitKey(1);
                        
			Mat timer(frame, cv::Rect(985, 603, 75, 33));
			imshow("timer" , timer);
			
			cvtColor( timer, hsv, CV_BGR2HSV);
			split(hsv, channels);

			cvtColor( timer, gray, CV_BGR2GRAY);
			if (white == 1){
				copyMakeBorder(gray,gray,10,10,10,10,BORDER_CONSTANT,Scalar(0));	
				Size size(3,3);
        			GaussianBlur(gray,gray,size,0);
        			adaptiveThreshold(gray, adapt,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV,13,10);
				imshow("adaptive" , adapt);	


				inRange(gray,0,150,ranger);
				imshow("inrange" , ranger);
				waitKey(1);
				//copyMakeBorder(gray,gray,10,10,10,10,BORDER_CONSTANT,Scalar(0));
				threshold(gray, otsuimg, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
				Mat cropedImage = otsuimg(Rect(10,10,width-10,height));
                                imshow("otsu", cropedImage);
				waitKey(1);
				Mat channel1 = channels[2];
				threshold(channel1, hsvOtsu, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
				imshow(" hsv otsu" , hsvOtsu);
				waitKey(1);
			}else{
				copyMakeBorder(gray,gray,10,10,10,10,BORDER_CONSTANT,Scalar(255));
				Size size(3,3);
                                GaussianBlur(gray,gray,size,0);
                                adaptiveThreshold(gray, adapt,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,13,10);
                                imshow("adaptive" , adapt);


                                inRange(gray,0,150,ranger);
                                imshow("inrange" , ranger);
                                waitKey(1);

                                threshold(gray, otsuimg, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
				Mat cropedImage = otsuimg(Rect(x+1,y+1,width-1,height-1));
                                imshow("otsu", cropedImage);
                                waitKey(1);
                                Mat channel1 = channels[2];
                                threshold(channel1, hsvOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                                imshow(" hsv otsu" , hsvOtsu);
                                waitKey(1);

			}



			
                }*/
		
		// check for colon for timer on or off, tbd vertical check
		colonCenter locationOfColon = checkForColon(white, x, y, width, height, frame, slotWidth, slotHeight, pixAboveColon);
		
		if(locationOfColon.colon == false) {
			cout << "NoTIMER NoTimer,  fps = " << endl;
			out <<"frame:" << frameCount << ", gc: " <<  "NO TIMER " << "," << "\n";
			continue;
		}
		prepareImage secSlot1 = numberRec(white, locationOfColon.centerX+pixSec1, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage secSlot2 = numberRec(white, locationOfColon.centerX+pixSec2, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage minSlot2 = numberRec(white, locationOfColon.centerX-pixMin2, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);
                prepareImage minSlot1 = numberRec(white, locationOfColon.centerX-pixMin1, locationOfColon.centerY - pixAboveColon, slotWidth, slotHeight, frame, videoNom);          
		
		// convert each digit slot image into CVARR, input to convnet must be CVARR and 32x32
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
		
		// forward pass each individual digit into network
		if(minSlot1.emptySlot == true || locationOfColon.centerX - pixMin1 < x){
                        min1 = 0;
                }
                if(minSlot1.emptySlot == false  && locationOfColon.centerX - pixMin1 >= x){
                        min1 = (int) net.fprop(imgMin1);
                }
		if(minSlot2.emptySlot == true || locationOfColon.centerX - pixMin2 < x){
                        min2 = 0;
                }
                if(minSlot2.emptySlot == false && locationOfColon.centerX - pixMin2 >= x){
                        min2 = (int) net.fprop(imgMin2);
                }
		
		sec1 = (int) net.fprop(imgSec1);
		sec2 = (int) net.fprop(imgSec2);
		
		// feature four detection and replacement 
		if (minSlot1.feature > 0 && minSlot1.emptySlot != true){ // fix jxr
			min1 = minSlot1.feature;
		}
		if (minSlot2.feature > 0 && minSlot1.emptySlot != true){
			min2 = minSlot2.feature;
                }
		if (secSlot1.feature > 0 && secSlot1.emptySlot != true){
			sec1 = secSlot1.feature;
                }
		if (secSlot2.feature >0 &&  secSlot2.emptySlot != true){
			sec2 = secSlot2.feature;
                }
		if (locationOfColon.sec == true){
                        min1 = 0;
                        min2 = 0;
                }
		//display result in red on real timer 
		ostringstream val1, val2, val3, val4;
		val1 << (int) min1;
		val2 << (int) min2;
		val3 << (int) sec1;
		val4 << (int) sec2;
		
		Mat display(frame, cv::Rect(x, locationOfColon.centerY - pixAboveColon, width, height));
		Mat displayTime( display.size(), CV_8U, Scalar(0));
        	display.copyTo(displayTime);
		copyMakeBorder(displayTime,displayTime,50,50,50,50,BORDER_CONSTANT,Scalar(0));		
	
		putText(displayTime, val1.str().c_str(), Point(0, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val2.str().c_str(), Point(10, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val3.str().c_str(), Point(20, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
                putText(displayTime, val4.str().c_str(), Point(30, displayTime.cols/2), FONT_HERSHEY_SIMPLEX, .5,Scalar(0, 0, 255),1);
		
		imshow( "Timer Recognition" , displayTime);
		waitKey(1);

		//time it takes to process, print to file
		time(&end);
                sec = difftime(end, start);
		cout << min1 << min2 << ":" << sec1 << sec2 << ", sec = " << sec << endl;
                out <<"frame:" << frameCount << ", gc: " << min1 << min2<< ":" << sec1 << sec2 << ","<< " sec:" << sec << " ," << "\n";
		
		cvReleaseImage(&imgSec1); // release memory
		cvReleaseImage(&imgSec2);
		cvReleaseImage(&imgMin1);
		cvReleaseImage(&imgMin2);	
	}

	out.close();
	}
	return 0;
}
