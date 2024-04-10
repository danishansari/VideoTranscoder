# VideoTranscoder
Transcode Video from any format to H264/MPEG-4 format.

### Required Packages: 
* ffmpeg
* libx264

### Compile
```
SET LIBDIRS for ffmpeg libraries
SET INCFLAGS for ffmpeg headerfiles

make clean; make
```

### Usage
```
bin/testTrancode -i </path/to/input/videos/file> -o </path/to/output/videos/file>
```

* Other options:
  ```
    -v    Version of application.
    -ip   Path to video directory.
    -irp  Path to video root directory to be scanned recursively.
    -f    Encoding format of output video (default=MPEG-4).
    -r    Frame rate of output video (default=15).
    -q    Quality of output video(default=2). 
  ```
