// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "vecmath/vecmath.h"
#include "ui/TraceUI.h"

#include <time.h>

extern TraceUI* traceUI;
extern bool ZERO(double d);
extern bool ZERO(vec3f v);

double frand(double max = 1.0) {
	return ((double)rand() / RAND_MAX) * max;
}

class vec3fGreater {
public:
	bool operator() (const vec3f& a, const vec3f& b) const {
		return a.length() > b.length();
	}
};

#define air_idx 1.0

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0, air_idx).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth, double curr_idx)
{
	if (depth > maxDepth || THRESHOLD(thresh)) {
		return vec3f(0, 0, 0);
	}
	isect i;
	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE
		
		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.
		vec3f myThresh;
		vec3f I = vec3f(0.0,0.0,0.0);
		vec3f Q = r.at(i.t); // Point of intersection (position)
		const Material& m = i.getMaterial();

		// Direct illumination
		I = m.shade(scene, r, i);

		vec3f unit_r = r.getDirection().normalize();
		vec3f unit_n = i.N.normalize();

		
		// Indirect illumination
		myThresh = thresh; // save the thresh in advance to seperate reflection and refraction thresholding
		for (int i = 0; i < 3; ++i) {
			myThresh[i] *= m.kr[i];
		}
		if (!m.kr.iszero()) {
			vec3f dir = ((2.0 * (unit_n.dot(-unit_r)) * unit_n) - (-unit_r)).normalize();
			ray R(vec3f(0,0,0), vec3f(0,0,0));

			numReflectVectors = 25;
			glossyReflectionDeviation = 0.2; // each component of dir deviates by +- this amount
			
			
			R = ray(Q, dir); // Reflection vector
			R = ray(R.at(1e-5), R.getDirection());
			vec3f reflect = traceRay(scene, R, myThresh, depth + 1, curr_idx);
			if (!glossyReflection) {
				for (int i = 0; i < 3; ++i) {
					I[i] += m.kr[i] * reflect[i];
				}
			}
			else {
				vec3f sum(0, 0, 0);
				sum += reflect;
				for (int i = 0; i < numReflectVectors-1; ++i) {
					ray newR = R.jiggle(glossyReflectionDeviation);
					sum += traceRay(scene, newR, myThresh, depth + 1, curr_idx);
				}
				for (int i = 0; i < 3; ++i) {
					sum[i] /= numReflectVectors;
					I[i] += m.kr[i] * sum[i];
				}
			}
			
		}
		
		
		// Refraction
		myThresh = thresh; // restore the thresh
		for (int i = 0; i < 3; ++i) {
			myThresh[i] *= m.kt[i];
		}
		if (!m.kt.iszero()) {
			double n_i;
			double n_t;
			double cos_angle_i;
			bool leaving = false;
			if (unit_r.dot(unit_n) < 0) {
				// Entering object
				n_i = curr_idx;
				n_t = m.index;
				cos_angle_i = (-unit_r).dot(unit_n);
			}
			else {
				// Leaving object
				n_i = m.index;
				n_t = curr_idx;
				cos_angle_i = (-unit_r).dot(-unit_n);
				leaving = true;
			}
			double ratio = n_i / n_t;
			double cos_angle_t = 1 - (ratio * ratio) * (1 - (cos_angle_i * cos_angle_i));
			if (cos_angle_t >= 0) {
				// square root is not imaginary: Total Internal Reflection does not occur
				cos_angle_t = sqrt(cos_angle_t);
				ray T(Q, ((ratio * cos_angle_i - cos_angle_t) * unit_n - ratio * (-unit_r)).normalize());
				T = ray(T.at(1e-5), T.getDirection());
				vec3f refract;
				if (!leaving) {
					refract = traceRay(scene, T, myThresh, depth + 1, m.index);
				}
				else {
					// Fire a dummy ray to check if there is an object B right outside of this object A
					ray dummy = T;
					isect ii;
					// If the dummy ray intersects with some object B AND the object B is close enough to this object A
					if (scene->intersect(dummy, ii) && ZERO(dummy.getPosition() - dummy.at(ii.t))) {
						// The refraction ray should claim that it is in object B
						refract = traceRay(scene, T, myThresh, depth + 1, ii.getMaterial().index);
					}
					else {
						// Consider the refraction ray leaves object A and enters air
						refract = traceRay(scene, T, myThresh, depth + 1, air_idx);
					}
				}
				
				for (int i = 0; i < 3; ++i) {
					I[i] += m.kt[i] * refract[i];
				}
			}
		}
		
		return I.clamp();
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	maxDepth = traceUI->getDepth();
	threshold = traceUI->getThreshold();
	srand(NULL);
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j, unsigned char* sampleBuffer)
{
	vec3f col;

	if( !scene )
		return;

	double x;
	double y;

	if (sampleBuffer == NULL) {
		double xnoise = 0.0, ynoise = 0.0;
		if (ssJitter) { // Instead of firing rays into the middle of each cell, introduce some noise
			xnoise = frand();
			ynoise = frand();
		}
		x = (double(i) + xnoise) / double(buffer_width);
		y = (double(j) + ynoise) / double(buffer_height);
	}
	else {
		double xnoise = 0.0, ynoise = 0.0;
		if (ssJitter) { // Instead of firing rays into the middle of each cell, introduce some noise
			xnoise = frand();
			ynoise = frand();
		}
		x = (double(i) + xnoise) / double(swidth);
		y = (double(j) + ynoise) / double(sheight);
	}

	col = trace( scene,x,y );


	unsigned char* pixel;
	if (sampleBuffer == NULL || numSubPixels == 1) {
		pixel = buffer + (i + j * buffer_width) * 3;
	}
	else {
		pixel = sampleBuffer + (i + j * swidth) * 3;
	}

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}

void RayTracer::calculateRowWithSamples(unsigned char* sampleBuffer, int row) {
	// sampleBuffer is of size swidth*sheight
	// find the average of every numSubPixels*numSubPixels subpixels and put it in the corresponding pixel in output buffer
	int i = row / (numSubPixels-1); // i is the row number in output buffer, row is the row number in sampleBuffer
	for (int col = 0; col < swidth-1; col += numSubPixels-1) {
		int j = col / (numSubPixels-1); // j is the col number in output buffer, col is the col number in sampleBuffer
		// For each pixel in the buffer, find the average of the surrounding subpixels
		for (int rgb = 0; rgb < 3; ++rgb) {
			int sum = 0;
			// Calculate the sum of the correct subset of sampleBuffer
			// (i,j) is the bottom-left corner of said subset, whose size is numSubPixels^2
			for (int y = row; y < row + numSubPixels; ++y) {
				for (int x = col; x < col + numSubPixels; ++x) {
					sum += (int)sampleBuffer[(x + y * swidth) * 3 + rgb];
				}
			}
			// Take the average over these subpixels
			sum /= (numSubPixels * numSubPixels);
				
			// Put the resulting average in buffer
			buffer[(j + i * buffer_width) * 3 + rgb] =	sum;
		}
	}
}