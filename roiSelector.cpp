#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/cvstd.hpp>
#include <iostream>
#include <vector>
#include <fstream>

using namespace cv;
using namespace std;

vector<Scalar> colors {
    Scalar(255,0,0),
    Scalar(0,255,0), 
    Scalar(0,0,255), 
    Scalar(204,78,1), 
    Scalar(79,0,128), 
    Scalar(255,255,85), 
    Scalar(79,186,218), 
    Scalar(229,152,230), 
    Scalar(88,0,144)
};
vector<vector<Point>> ptsVectors{};
vector<Point> pts{};
Mat im;
bool loop = true;

void printInfos(void);
void drawRoi(int event, int x, int y, int flags, void* userdata);
bool checkROIClosed(int distThreshold);
Mat plotSegmentedROIs(Mat imTmp, Mat imMsk, vector<Scalar>colorVectors);
void roiSelector(Mat img);
string cv2Points2JsonString(void);
void saveJSON(void);

int main( int argc, char** argv )
{
    vector<String> filenames;
    String keys =
        "{@path |<none>           | input image path}"         // input image is the first argument (positional)
        "{help  |      | show help message}";      // optional, show help optional

    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    String filepath = parser.get<String>(0);
    if (!parser.check()) {
        parser.printErrors();
        return -1;
    }
    printInfos();

    //string folder{argv[1]};

    glob(filepath, filenames, false);
 
    for(int i=0; i<filenames.size(); i++)
    {
        im = imread(filenames[i]);
        roiSelector(im);
    }
    return 0;
}

void printInfos(void)
{
    cout << "[INFO] This is a Multiple ROI selector script for multiple images" << endl;
    cout << "[INFO] Click the left button: select the point, right click: delete the last selected point" << endl;
    cout << "[INFO] Press 'S' to  save all ROIs" << endl;
    cout << "[INFO] Press 'D' to delete last selected ROI" << endl;
    cout << "[INFO] Press 'Q' to quit" << endl;
}

Mat plotSegmentedROIs(Mat imTmp, Mat imMsk, vector<Scalar>colorVectors)
{
    if(ptsVectors.size() > 0)
    {
        for (int i = 0; i < ptsVectors.size(); i++)
        {
            const Point* ppt[1] = {&ptsVectors[i][0]};
            int npt[] = {ptsVectors[i].size()};
            fillPoly( imMsk, ppt, npt, 1, colorVectors[i%colorVectors.size()], LINE_8 );
        }
        addWeighted( imTmp, 0.5, imMsk, 0.5, 0.0, imTmp);
    }
    return imTmp;
}

bool checkROIClosed(int distThreshold)
{
    int dist = norm(pts[0]-pts[pts.size()-1]);
    if(dist < distThreshold)
        return true;
    return false;
}

void drawRoi(int event, int x, int y, int flags, void* userdata)
{
    Mat imTemp = im.clone();
    Mat imMask = im.clone();

    if  ( event == EVENT_LBUTTONDOWN )
    {
        pts.push_back(Point(x,y));
        //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    if  ( event == EVENT_RBUTTONDOWN )
    {
        if(pts.size() > 0)
        {
            pts.pop_back();
            //cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        }

    }
    imTemp = plotSegmentedROIs(imTemp, imMask, colors);

    /*
    cout << "Pts Counter: " << ptsCounter << endl;
    cout << endl;
    */
   /*
    cout << "==================pts================" << endl;
    for (int i = 0; i < pts.size(); i++)
    {
        cout << pts[i] << endl;
    }
    cout << "=====================================" << endl;
    //cout << "pts size : " << pts.size() << endl;
    */
    if(pts.size() > 0)
    {
        circle(imTemp, pts[pts.size()-1], 1, Scalar(0,0,255), -1);
    }

    if(pts.size() > 1)
    {
        if(checkROIClosed(5))
        {
            pts.pop_back();
            ptsVectors.push_back(pts);
            pts.clear();
        }
        if(!pts.empty())
        {
            for (int i = 0; i < pts.size()-1; i++)
            {
                circle(imTemp, pts[i], 1, Scalar(0,0,255), -1);
                line(imTemp, pts[i], pts[i+1], Scalar(255, 0, 0),1);
            }            
        }


        //circle(im, pts[i], 2, Scalar(0,0,255), -1);
    }

    imshow("roiSelector", imTemp);
    /*
    cout << "==============PTSVECTORS==============" << endl;
    if(ptsVectors.size() > 0)
    {
        for (int i = 0; i < ptsVectors.size(); i++)
        {
            for (int j = 0; j < ptsVectors[i].size(); j++)
            {
                cout << ptsVectors[i][j];
            }
            cout << endl;
        }
        
    }
    cout << "=====================================" << endl;
    */

    //cout <<  "Pts : " << &pts << endl;
    /*
    else if  ( event == EVENT_MBUTTONDOWN )
    {
        cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if ( event == EVENT_MOUSEMOVE )
    {
        cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

    }
    */
}

void roiSelector(Mat img)
{
    namedWindow("roiSelector", WINDOW_FREERATIO);
    setMouseCallback("roiSelector", drawRoi, NULL);
    imshow("roiSelector", im);
    while(loop)
    {
        char key = waitKey(30);
        switch (tolower(key))
        {
        case 'q':
            loop = false;
            break;
        case 's':
            loop = false;
            saveJSON();
            break;
        case 'd':
            if(ptsVectors.size() > 0)
            {
                ptsVectors.pop_back();
                pts.clear();
            }
            break;
        default:
            break;
        }
    }
    destroyAllWindows();
}


string cv2Points2JsonString(void)
{
    string points{""};
    points += "[";
    for(int i=0 ; i<ptsVectors.size() ; i++)
    {
        points = points + "{\"Id\": "+to_string(i+1)+", \"Points\": [";
        for(int j=0 ; j<ptsVectors[i].size() ; j++)
        {
            points += "[";
            points += to_string(ptsVectors[i][j].x) + "," + to_string(ptsVectors[i][j].y); 
            points += "]";
            if(j!=ptsVectors[i].size()-1)
                points += ",";
        }
        points += "]";
        points += "}";
        if(i!=ptsVectors.size()-1)
            points += ",";

    }
    points += "]";
    return points;
}

void saveJSON(void)
{
    string jsonStr = cv2Points2JsonString();
    ofstream fout;
    fout.open("rois.json", ios_base::app);
    fout << jsonStr;
}