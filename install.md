Installation Instructions (Windows)
===================================

1. Download OpenCV v2.4.5 library from http://opencv.org/downloads.html.
   Note: version 2.4.5 is required because it is bounded to OpenCvSharp.

2. Download OpenCvSharp library: https://code.google.com/p/opencvsharp/.
   Note: version must be equal to OpenCV library one and platforms must
   match (if you've downloaded OpenCV x64, OpenCvSharp must be x64 too).

3. Extract OpenCV to some path. Then type in the command line: "setx -m
   OPENCV_DIR %path_to_cv%\build\x%platform%\vc10", then set environment
   variable PATH to PATH += %OPENCV_DIR%\bin.

4. Create a Windows Forms Project. Choose Project -> Add Reference, and
   Browse and select there a copy of DLL named OpenCvSharp.dll.

5. using OpenCvSharp; // finishes this guide