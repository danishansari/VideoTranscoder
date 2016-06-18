
/**
 * Description:
 *
 * Author: Md Danish
 *
 * Date: 
**/

#include <iostream>
#include <vector>

#include <dirent.h>

#include "VideoDecoder.h"
#include "VideoEncoder.h"

#define MAJOR_VERSION 1.0
#define MINOR_VERSION 1.2

using namespace std;

// Function to print version
void printVersion();

// function to print help
void printHelp();

// function to get all filenames from directory
int getAllFiles(const char *rootPath, vector<string> &path, bool searchRec);

// main starts here
int main(int argc, char**argv)
{
    // input video filename
    string inputFile = "";

    // input video filepath
    string inputFilePath = "";

    // output video filename
    string outputFile = "sample.avi";

    // output video format
    string encodeFormat = "MPEG-4";

    // output video frame rate
    int frameRate = 15;

    // output video quality
    int quality = 2;

    // flag to control serching of files in directory
    int searchFiles = 0;

    // vector to store all file names
    vector<string> allFiles;

    // parse command line arguments
    for (int i = 1; i < argc; i+=2)
    {
        if (i <= argc and strcmp(argv[i], "-v") == 0)
        {
            printVersion();
            return -1;
        }
        else if (i <= argc and strcmp(argv[i], "-h") == 0)
        {
            printHelp();
            return -1;
        }
        else if (i <= argc and strcmp(argv[i], "-i") == 0)
        {
            inputFile = argv[i+1];
            allFiles.push_back(inputFile);
        }
        else if (i <= argc and strcmp(argv[i], "-ip") == 0)
        {
            inputFilePath = argv[i+1];
            searchFiles = 1;
        }
        else if (i <= argc and strcmp(argv[i], "-irp") == 0)
        {
            inputFilePath = argv[i+1];
            searchFiles = 2;
        }
        else if (i <= argc and strcmp(argv[i], "-o") == 0)
            outputFile = argv[i+1];
        else if (i <= argc and strcmp(argv[i], "-f") == 0)
            encodeFormat = argv[i+1];
        else if (i <= argc and strcmp(argv[i], "-r") == 0)
            frameRate = atoi(argv[i+1]);
        else if (i <= argc and strcmp(argv[i], "-q") == 0)
            quality = atoi(argv[i+1]);
        else
        {
            cout << "Prameter: " << argv[i] << " not supported(type " << argv[0] << " -h for help)." << endl;
            return -1;
        }
    }

    // check for search option and perform accordinly
    if (searchFiles) 
    {
        int totalFiles = 0;
        if (searchFiles == 1) // search file in that directory only
            totalFiles = getAllFiles(inputFilePath.c_str(), allFiles, false);
        else // search file recursively in that directory and subdirectories
            totalFiles = getAllFiles(inputFilePath.c_str(), allFiles, true);
        
        cout << "Total video files found = " << totalFiles << endl;
    }

    // check if input filename or filepath was provided
    if (allFiles.empty() and searchFiles)
    {
        cout << "Please provide source video file(type " << argv[0] << " -h for help)." << endl;
        return -1; // return failure
    }
 
    // Create object of video decoder
    VideoDecoder videoDecoder;

    // create object of video encoder
    VideoEncoder videoEncoder;
            
    // video encoder context object 
    VideoEncoderContext encoderContext;

    // encoding status
    int encodeStatus = -1;

    // rgb frame to decode and encode
    unsigned char *rgbFrame = NULL;

    // loop for all video files
    for (int file = 0; file < (int)allFiles.size(); file++)
    {
        // get current input file
        inputFile = allFiles[file];

        cout << "Current Source Video = " << inputFile << endl;
        
        // open video for decoding
        int videoStatus = videoDecoder.openVideo(inputFile);
        
        // check opening vidoe was success
        if (videoStatus == -1)
        {
            cout << "Could not find video: " << inputFile << endl;
            //return -1;
            continue;
        }
        
        // get video input video info in struct videoinfo
        VideoInfo videoInfo;
        videoDecoder.getVideoInfo(videoInfo);

        // printing input and output video info
        cout << "==============================================" << endl;
        cout << "Source Video   :   " << videoInfo.videoFileName << endl;
        cout << "Width          :   " << videoInfo.width << endl;
        cout << "Height         :   " << videoInfo.height << endl;
        cout << "Output Video   :   " << outputFile << endl;
        cout << "Format         :   " << encodeFormat << endl;
        cout << "FrameRate      :   " << frameRate << endl;
        cout << "Quality        :   " << quality << endl;
        cout << "==============================================" << endl;

        // set encoder context for output video
        encoderContext.outputVideoFile = outputFile;
        encoderContext.codecStr = encodeFormat;
        encoderContext.width = videoInfo.width;
        encoderContext.height = videoInfo.height;
        encoderContext.frameRate = frameRate;
        encoderContext.quality = quality;

        // allocate memory to rgbframe, if not allocated
        if (!rgbFrame) 
            rgbFrame = new unsigned char[videoInfo.width * videoInfo.height * 3];

        // set encoder context if not set
        if (!videoEncoder.encoderCtxSet())
        {
            videoEncoder.setEncoderContext(encoderContext);
            encodeStatus = videoEncoder.startVideoEncode();
        }

        // check for encoder context status
        if (encodeStatus == -1)
        {
            cout << "Could not startEncoding: status = " << encodeStatus << endl;
        }
        else
        {
            // get a new frame from the video
            while (videoDecoder.getNewFrame(rgbFrame) > 0)
            {
                // add newly fetched frame to the output video
                int size = videoEncoder.addNewFrame(rgbFrame);

                // check if encoding was succesful
                if (size < 0)
                {
                    cout << "Could not encode video: size = " << size << endl;
                    break;
                }
            }
        }

        // close video decoding
        videoDecoder.closeVideo();
    }
    
    // stop video encoding
    videoEncoder.stopVideoEncode();

    // delete rgb frames
    if (rgbFrame)
        delete [] rgbFrame;

    return 0;
}

// function to get all files from directory and subdrirectories
int getAllFiles(const char *path, vector<string> &pathVec, bool searchRec)
{
    DIR *dir;
    struct dirent *dirent;

    static int fileCount = 0;

    string completePath = path;

    if ((dir = opendir(path)) != NULL) 
    {
        while ((dirent = readdir(dir)) != NULL) 
        {
            if((strcmp(dirent->d_name,"..") != 0) && (strcmp(dirent->d_name,".") != 0))
            {
                string filePath = completePath + dirent->d_name;

                if(dirent->d_type == DT_DIR && searchRec)
                {
                    filePath += "/";
                    getAllFiles(filePath.c_str(), pathVec, true);
                }

                pathVec.push_back(filePath);
                fileCount ++;
            }
        }

        closedir(dir);
    }

    return fileCount;
}

// Function to print command line options
void printHelp()
{
    cout << "\nVideoTransoder:: Help Menu(-h)" << endl;
    cout << "-i     : input video file              (default = n/a)" << endl;
    cout << "-ip    : input video path              (default = n/a)" << endl;
    cout << "-irp   : input video path recursive    (default = n/a)" << endl;
    cout << "-o     : output video                  (default = sample.avi)" << endl;
    cout << "-f     : output video format           (default = MPEG-4)" << endl;
    cout << "-r     : output video frame rate       (default = 15)" << endl;
    cout << "-q     : output video quality          (default = 2)\n" << endl;
}

// Function to print version information
void printVersion()
{
    cout << "\nVideoTransoder::  Version Information" << endl;
    cout << "Major Version:   " << MAJOR_VERSION << endl;
    cout << "Minor Version:   " << MINOR_VERSION << "\n" << endl;
}
