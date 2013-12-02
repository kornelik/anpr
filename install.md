

Build Instructions (Linux)
===================================


1. Install OpenCV library: http://opencv.org/
	
	`sudo apt-get install libopencv-dev`

2. Install Tesseract library: https://code.google.com/p/tesseract-ocr/

   	`sudo apt-get install tesseract-ocr tesseract-ocr-eng libtesseract-dev`

3. Get the source code from this repo

	`git clone https://github.com/kornelik/anpr`

4. Build the code with...

	`cd anpr/src`
	
	`make`



Installation Instructions (Windows)
===================================

1. Install Visual Studio 2010 or higher:

   http://www.visualstudio.com/en-us/downloads/

2. Install OpenCV library and configure a project in Visual Studio for using it:

   http://opencv-srf.blogspot.com/2013/05/installing-configuring-opencv-with-vs.html

3. Install Tesseract OCR library:

   https://code.google.com/p/tesseract-ocr/downloads/

4. Configure Tesseract OCR library for created Visual Studio project:

   4.1. Project-> Properties -> Configuration Properties -> VC++ Directories

	* Add to "Include Directories" folder %tesseract_ocr_install_path%\tesseract\include
	* Add to "Library Directories" folder %tesseract_ocr_install_path%\tesseract\lib

   4.2. Project -> Properties -> Configuration Properties -> Linker

           * General tab. Add to "Additional Library Dependencies" folder %tesseract_ocr_install_path%\tesseract\lib
           * Input tab. Add to "Additional Dependencies" the following libraries:

                  libtesseract302.lib
                  libtesseract302d.lib
                  libtesseract302-static.lib
                  libtesseract302-static-debug.lib

5. Clone the project source code. Run the command in console:

   `git clone https://github.com/kornelik/anpr`

6. Copy the project source code to the project folder %solution_root%\%project_name%

7. Build the project (F6).

8. To run from Visual Studio - Ctrl+F5.
   To run from the console:

       `cd %solution_root%\(Debug|Release)`
       `ANPRProject.exe [--gui]`