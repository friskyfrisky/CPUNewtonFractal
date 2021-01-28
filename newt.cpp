#include <iostream>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <unistd.h>
#include <complex>
#include <random>
#include "./EasyBMP/EasyBMP.hpp"

using namespace std;

//config globals, we recompile to change input
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

static int threadAmount = 4;
static int maxIt = 100;
static float step = .001;
static float converge = .001;
double *inde;

//image dimensions
static int s = 4;
static int imgX = 1920*s;
static int imgY = 1080*s;

//boundries
float ofX = 0;
float ofY = 0;
float sf = .1;

float xa = -1 * sf * imgX/imgY + ofX;
float xb = sf * imgX/imgY + ofX;
float ya = sf + ofY;
float yb = -1 * sf + ofY;


//f(x) we are calculating fractal for
complex<float> ftc(float x);
complex<float> f(complex<float> x) {
	float i = (float)*inde / 100;
	//complex<float> r = x*x*x - floatToComplex(1.0);
	complex<float> r = x*sin(x*x-x*ftc(3*i))*cos(x*ftc(5)*ftc(i)) + sin(x*x*x*ftc(i))*ftc(10) - ftc(1);
	return r;
}

//image
EasyBMP::RGBColor black(0,0,0);
EasyBMP::Image img(imgX, imgY, "out.bmp", black);

//helpers
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
complex<float> ftc(float x) {
	complex<float> r = complex<float> (x,0.0);
	return r;
}
float calcDist(float a, float b, float x, float y) {
	float r = abs(hypot(a-x,b-y));
	return r;
}
//input x,y get a complex number with the coords on the complex plain to calc
complex<float> imgToComplex(int x, int y) {
	float xr = (float)x/imgX;
	float yr = (float)y/imgY;
	float xd = xb-xa;
	float yd = yb-ya;
	float newx = xa + xd*xr; float newy = ya + yd*yr;
	complex<float> r = complex<float> (newx, newy);
	return r;
}
float varFloor(float x, float flo, float ceil) {
	int r;
	if (x < flo) {
		r = flo;
	}
	else if (r > ceil) {
		r = ceil;
	}
	else {
		r = x;
	}
	r;
	return r;
}
void writeImg(int y,int x, float its, complex<float> p) {
	if (y < imgY && x < imgX) {
		float crf = real(p)*70;
		float cgf = imag(p)*80;
		float cbf = real(p)*40-imag(p)*40;

		float bright = 30;
		crf = abs(crf+bright);
		cgf = abs(cgf+bright);
		cbf = abs(cbf+bright);

		float darkScale = 4;
		cbf = varFloor(cbf - its*darkScale,0,255);
		cgf = varFloor(cgf - its*darkScale,0,255);
		cbf = varFloor(cbf - its*darkScale,0,255);

		int cr = round(crf);
		int cg = round(cgf);
		int cb = round(cbf);

		img.SetPixel(x,y, EasyBMP::RGBColor(cr,cg,cb));
	}
	//write to the image file
	//p is the point we got after doing newton itteration on a point, use to color
}
void imgOut() {
	img.Write();
}

//classes
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//this class represents the threads to the scheduler thread
struct rType {
	complex<float> data[2];
};
int lastSeenPercent = -1;
class threadRep {
	private:
		static rType calcZero(complex<float> xIn);
		thread *worker;
		int ID;
		int lastIts;

		static void workerFunc(int id) {
			//current line we're on, program begins with threads working on the lines that they are numbered by.
			//thread 2 say, starts at 2. now add the amount of threads to 2, that is our next line to calculate.
			//we can avoid having to communicate with the scheduler as much this way.
			int cLine = id;
			bool doneLine = false;
			while(!doneLine) {
				for (int i=0; i<imgX; i++) {
					complex<float> inC = imgToComplex(i,cLine);
					rType result = calcZero(inC);
					writeImg(cLine,i,imag(result.data[1]),result.data[0]);
				}
				cLine += threadAmount;
				if (cLine % 100 + 1 == 1) {
					double pResult = round((double)cLine / (double)imgY * 100);
					if (lastSeenPercent < pResult) {
						cout << pResult << "%" << endl;
						lastSeenPercent = pResult;
					}
			       	}
				if (cLine >= imgY) { doneLine = true; }
			}
			//ending stuff
		}
		//pointer to our thread
	public:
		//no input for now, function is a global in config area, we will init the thread here
		void init(int id) {
			ID = id;
		};
		void giveWork();
		void kill();
		void wait();
};

//funcs
//---------------------------------
void threadRep::giveWork() {
	worker = new thread(workerFunc, ID);
	//worker->detach();
}
void threadRep::kill() {
	delete worker;
}
void threadRep::wait() {
	worker->join();
}
//code for newton method on complex numbers for a point
rType threadRep::calcZero(complex<float> xIn) {
	int its = 0;
	complex<float> x = xIn;
	bool dvZero = false;
	bool isConverged = false;
	float sum = 0;
	while (!isConverged) { 
		complex<float> xLast = x;


		//newton method func here
		complex<float> fd = ( f(x + step) - f(x) ) / step;
		x = x - f(x)/fd;
		//we need to catch division by zero, does not except on compile
		if (isnan(real(fd)) || isnan(imag(fd))) {
			x = xLast;
			dvZero = true;
		}
		float dist = 99;
		if(!dvZero) {
			dist = calcDist(real(x),imag(x),real(xLast),imag(xLast));
		}
		its += 1;
		if (dist < converge || its >= maxIt || dvZero) {
			isConverged = true;
		}
		sum += dist;
		
	}
	rType r;
	r.data[0] = x;
	r.data[1] = complex<float> (its,log(sum));
	return r;
}

//main loop now
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
	inde = new double;
	*inde = atof(argv[1]);
	cout << *inde << endl;

	threadRep workers[threadAmount];
	for (int i=0; i<threadAmount; i++) {
		workers[i].init(i);
		workers[i].giveWork();
	}
	
	for (int i=0; i<threadAmount; i++) {
		workers[i].wait();
		workers[i].kill();
	}
	delete inde;
	imgOut();
	return 1;
}
