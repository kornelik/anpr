Installation Instructions (Windows)
===================================

http://opencv-srf.blogspot.com/2013/05/installing-configuring-opencv-with-vs.html


Build Instructions (Linux)

1. Install OpenCV library: http://opencv.org/
	
	sudo apt-get install libopencv-dev

2. Install Tesseract library: https://code.google.com/p/tesseract-ocr/

   	sudo apt-get install tesseract-ocr tesseract-orc-eng libtesseract-dev

3. Get the source code from this repo

	git clone https://github.com/kornelik/anpr

4. Run `make` command

	cd anpr/src
	make

