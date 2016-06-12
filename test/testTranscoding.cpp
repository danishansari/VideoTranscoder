
/**
 * Description:
 *
 * Author: Md Danish
 *
 * Date: 
**/

#include <iostream>

#include "VideoDecoder.h"
#include "VideoEncoder.h"

#define MAJOR_VERSION 1.0
#define MINOR_VERSION 1.1

using namespace std;

void printVersion();
void printHelp();

int main(int argc, char**argv)
{
    string inputFile = "";
    string outputFile = "sample.avi";
    string encodeFormat = "MPEG-4";
    int frameRate = 15;
    int quality = 2;

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
            inputFile = argv[i+1];
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

    if (inputFile.empty())
    {
        cout << "Please provide source video file(type " << argv[0] << " -h for help)." << endl;
        return -1;
    }

    VideoDecoder videoDecoder;
    int videoStatus = videoDecoder.openVideo(inputFile);

    if (videoStatus == -1)
    {
        cout << "Could not find video: " << inputFile << endl;
        return -1;
    }

    VideoInfo videoInfo;
    videoDecoder.getVideoInfo(videoInfo);

    cout << "==============================================" << endl;
    cout << "Source Video   :   " << videoInfo.videoFileName << endl;
    cout << "Width          :   " << videoInfo.width << endl;
    cout << "Height         :   " << videoInfo.height << endl;
    cout << "Output Video   :   " << outputFile << endl;
    cout << "Format         :   " << encodeFormat << endl;
    cout << "FrameRate      :   " << frameRate << endl;
    cout << "Quality        :   " << quality << endl;
    cout << "==============================================" << endl;

    VideoEncoderContext encoderContext;
    encoderContext.outputVideoFile = outputFile;
    encoderContext.codecStr = encodeFormat;
    encoderContext.width = videoInfo.width;
    encoderContext.height = videoInfo.height;
    encoderContext.frameRate = frameRate;
    encoderContext.quality = quality;

    VideoEncoder videoEncoder;
    videoEncoder.setEncoderContext(encoderContext);
    int encodeStatus = videoEncoder.startVideoEncode();

    unsigned char *rgbFrame = new unsigned char[videoInfo.width * videoInfo.height * 3];

    if (encodeStatus == -1)
    {
        //cout << "Could not startEncoding: status = " << encodeStatus << endl;
    }
    //else
    {
        while (videoDecoder.getNewFrame(rgbFrame) > 0)
        {
            int size = videoEncoder.addNewFrame(rgbFrame);

            if (size <= 0)
            {
                cout << "Could not encode video: size = " << size << endl;
                break;
            }
        }
    }

    videoEncoder.stopVideoEncode();
    videoDecoder.closeVideo();

    delete [] rgbFrame;

    return 0;
}

void printHelp()
{
    cout << "\nVideoTransoder:: Help Menu(-h)" << endl;
    cout << "-i     : input video file              (default = n/a)" << endl;
    cout << "-ip    : input video path              (default = n/a)" << endl;
    cout << "-ipr   : input video path recursive    (default = n/a)" << endl;
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
