
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
#define MINOR_VERSION 1.1

using namespace std;

void printVersion();
void printHelp();
int getAllFiles(const char *rootPath, vector<string> &path, bool searchRec);

int main(int argc, char**argv)
{
    string inputFile = "";
    string inputFilePath = "";
    string outputFile = "sample.avi";
    string encodeFormat = "MPEG-4";
    int frameRate = 15;
    int quality = 2;

    int searchFiles = 0;

    vector<string> allFiles;

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

    if (searchFiles) // search files at location
    {
        int totalFiles = 0;
        if (searchFiles == 1)
            totalFiles = getAllFiles(inputFilePath.c_str(), allFiles, false);
        else
            totalFiles = getAllFiles(inputFilePath.c_str(), allFiles, true);
        
        cout << "Total video files found = " << totalFiles << endl;
    }

    if (allFiles.empty() and searchFiles)
    {
        cout << "Please provide source video file(type " << argv[0] << " -h for help)." << endl;
        return -1;
    }
 
    VideoDecoder videoDecoder;

    VideoEncoder videoEncoder;
            
    VideoEncoderContext encoderContext;

    int encodeStatus = -1;

    unsigned char *rgbFrame = NULL;

    for (int file = 0; file < (int)allFiles.size(); file++)
    {
        inputFile = allFiles[file];

        cout << "Current Source Video = " << inputFile << endl;

        int videoStatus = videoDecoder.openVideo(inputFile);
        
        VideoInfo videoInfo;
        videoDecoder.getVideoInfo(videoInfo);

        if (videoStatus == -1)
        {
            cout << "Could not find video: " << inputFile << endl;
            //return -1;
            continue;
        }

        cout << "==============================================" << endl;
        cout << "Source Video   :   " << videoInfo.videoFileName << endl;
        cout << "Width          :   " << videoInfo.width << endl;
        cout << "Height         :   " << videoInfo.height << endl;
        cout << "Output Video   :   " << outputFile << endl;
        cout << "Format         :   " << encodeFormat << endl;
        cout << "FrameRate      :   " << frameRate << endl;
        cout << "Quality        :   " << quality << endl;
        cout << "==============================================" << endl;

        encoderContext.outputVideoFile = outputFile;
        encoderContext.codecStr = encodeFormat;
        encoderContext.width = videoInfo.width;
        encoderContext.height = videoInfo.height;
        encoderContext.frameRate = frameRate;
        encoderContext.quality = quality;

        if (!rgbFrame) 
            rgbFrame = new unsigned char[videoInfo.width * videoInfo.height * 3];

        if (!videoEncoder.encoderCtxSet())
        {
            videoEncoder.setEncoderContext(encoderContext);
            encodeStatus = videoEncoder.startVideoEncode();
        }

        if (encodeStatus == -1)
        {
            cout << "Could not startEncoding: status = " << encodeStatus << endl;
        }
        else
        {
            while (videoDecoder.getNewFrame(rgbFrame) > 0)
            {
                int size = videoEncoder.addNewFrame(rgbFrame);

                if (size < 0)
                {
                    cout << "Could not encode video: size = " << size << endl;
                    break;
                }
            }
        }

        videoDecoder.closeVideo();
    }
    
    videoEncoder.stopVideoEncode();

    if (rgbFrame)
        delete [] rgbFrame;

    return 0;
}

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
                //printf ("%s", dirent->d_name);
                string filePath = completePath + dirent->d_name;

                if(dirent->d_type == DT_DIR && searchRec)
                {
                    filePath += "/";
                    //getAllFiles(dirent->d_name, pathVec, true);
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

void printVersion()
{
    cout << "\nVideoTransoder::  Version Information" << endl;
    cout << "Major Version:   " << MAJOR_VERSION << endl;
    cout << "Minor Version:   " << MINOR_VERSION << "\n" << endl;
}
