// Include libraries
#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

// Video Class
class Video{
private:
    
public:
    Video(){
        this->cam_input=-1;
        this->location="";
    }
    
    char* location;
    int cam_input;
    // Load and process video
    bool run(){
        VideoCapture vid;
        // Set VideoCapture
        if (cam_input == -1) {
            if(!(location=="")){
                vid=VideoCapture(location);
            }
            else{
                cout<<"Please set what to load!!!"<<endl;
                return false;
            }
        }
        else{
            vid=VideoCapture(cam_input);
        }
        
        if (!vid.isOpened()) {
            cout<<"Could not be opened!!!"<<endl;
            return false;
        }
        namedWindow("My Vid",CV_WINDOW_AUTOSIZE);
        double fps=vid.get(CV_CAP_PROP_FPS);
        cout<<"frames per second!!!"<<fps<<endl;
        while (1) {
            Mat image;
            if (!vid.read(image)) {
                cout<<"Couldnot be read!!!"<<endl;
                return false;
            }
            //cout<<image.rows<<"Rows"<<endl;
            //cout<<image.cols<<"Cols"<<endl;

            // Process the Image
            image=process(image);
            imshow("My Vid",image);
            if (waitKey(10)>=0) {
                return false;
            }
        }
        return true;
    }
    // Histogram of an Image
    MatND getHistogram(const Mat &image) {
        MatND hist;
        int histSize[1];
        float hranges[2];
        const float* ranges[1];
        int channels[1];
        
        histSize[0]= 256;
        hranges[0]= 0.0;
        hranges[1]= 255.0;
        ranges[0]= hranges;
        channels[0]= 0;
        
        calcHist(&image,
                 1,
                 channels,
                 Mat(),
                 hist,
                 1,
                 histSize,
                 ranges
                 );
        return hist;
    }
    // Binarizing Image
    Mat binariseGrayscale(Mat &image,int threshold){
        Mat binarised(image.rows,image.cols,CV_8UC1);
        if( (threshold<0) | (threshold>255 )){
            cout<<"Threshold cannot be other than between 0 and 255 returning random image "<<endl;
            return binarised;
        }
        int i,j;
        for ( i=0; i<image.rows; i++) {
            uchar* data=image.ptr<uchar>(i);
            for ( j=0; j<image.cols; j++) {
                if (data[j]>=threshold) {
                    binarised.at<uchar>(i,j)=255;
                }
                else{
                    binarised.at<uchar>(i,j)=0;
                }
            }
        }
        return binarised;
    }
    // Divide Image into part and apply otsu thresholding
    Mat binary_otsu_byparts(Mat &image,const int number){
        Mat changed_image(image.rows/number , image.cols/number ,CV_8UC1);
        Mat final_image(image.rows,image.cols,CV_8UC1);
        int i,j,k,l;
        int rows=image.rows/number;
        int cols=image.cols/number;
        for ( i=0; i<number; i++) {
            for ( j=0; j<number; j++) {
                for ( k=0; k<rows; k++) {
                    for ( l=0; l<cols; l++) {
                        changed_image.at<uchar>(k,l)=image.at<uchar>(i*rows+k,j*cols+l);
                    }
                }
                changed_image=binariseGrayscale(changed_image,get_otsu_threshold(changed_image));
                for ( k=0; k<rows; k++) {
                    for ( l=0; l<cols; l++) {
                        final_image.at<uchar>(i*rows+k,j*cols+l)=changed_image.at<uchar>(k,l);
                    }
                }
            }
        }
        return final_image;
    }
    // Process the Image
    Mat process(Mat image){
        cvtColor(image,image,CV_BGR2GRAY);
        MatND im=getHistogram(image);
        int i;
        int min=0;
        for ( i=0 ; i<256 ; i++ , min++ ) {
            if (im.at<float>(i) > 0) {
                break;
            }
        }
        int max=255;
        for ( i=0 ; i<256 ; i++ , max-- ) {
            if (im.at<float>(255-i) > 0) {
                break;
            }
        }
        image=image*255/(max-min) - min*255/(max-min);
        // Binarizing by parts
        if (image.cols<image.rows) {
            image=binary_otsu_byparts(image,image.cols/50);
        }
        else{
            image=binary_otsu_byparts(image,image.rows/50);
        }
        //image=binary_otsu_byparts(image,15);

        // Color to apply
        //int black[3]={0,255,255},white[3]={255,0,0};//yellow and blue
        int black[3]={255,0,0},white[3]={0,255,255};//yellow and blue
        image=output(image,black,white);
        return image;
    }
    // Show the Output
    Mat output(Mat image,int* black,int* white){
        Mat im(image.rows,image.cols,CV_8UC3,Scalar(0,0,0));
        for (int i=0; i<image.rows; i++) {
            for (int j=0; j<image.cols; j++) {
                if (image.at<uchar>(i,j)==255) {
                    im.at<Vec3b>(i,j)[0]=white[0];
                    im.at<Vec3b>(i,j)[1]=white[1];
                    im.at<Vec3b>(i,j)[2]=white[2];
                }
                else{
                    im.at<Vec3b>(i,j)[0]=black[0];
                    im.at<Vec3b>(i,j)[1]=black[1];
                    im.at<Vec3b>(i,j)[2]=black[2];
                }
            }
        }
        return im;
    }
    // Otsu thresholding on an Image
    int get_otsu_threshold(Mat image){
        int threshold;
        MatND histData;
        histData=getHistogram(image);
        int t;
        int total=0;
        for ( t=0 ; t<256 ; t++) total +=  histData.at<float>(t);
        
        float sum = 0;
        for ( t=0 ; t<256 ; t++) sum += t * histData.at<float>(t);
        
        float sumB = 0;
        int wB = 0;
        int wF = 0;
        
        float varMax = 0;
        threshold = 0;
        
        for ( t=0 ; t<256 ; t++) {
            wB += histData.at<float>(t);
            if (wB == 0) continue;
            
            wF = total - wB;
            if (wF == 0) break;
            
            sumB += (float) (t * histData.at<float>(t));
            
            float mB = sumB / wB;
            float mF = (sum - sumB) / wF;
            
            float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);
            
            if (varBetween > varMax) {
                varMax = varBetween;
                threshold = t;
            }
        }
        if (!process_threshold(threshold,image)) {return threshold;}
        else{return 0;}
    }
    
    bool process_threshold(int threshold,Mat image){
        int count=0;
        int i,j;
        int imp=image.cols*image.rows/4000;
        for ( i=0; i<image.rows; i++) {
            uchar* data=image.ptr<uchar>(i);
            for ( j=0; j<image.cols; j++) {
                if ( (data[j]>threshold+20) | (data[j]<threshold-20)) { count++ ; }
            }
        }
        if (count>imp) {
            return false;
        }
        else{
            return true;
        }
    }
};

// Main
int main(int argc,char** argv){
    Video first;
    first.cam_input=0;// Set the cam input here 0,1,2...
    first.run();// Run 
    return 0;
}