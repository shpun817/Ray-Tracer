#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/scene.h"
#include "scene/ray.h"

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    vec3f trace( Scene *scene, double x, double y );
	vec3f traceRay( Scene *scene, const ray& r, const vec3f& thresh, int depth, double curr_idx);


	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h );
	void traceLines( int start = 0, int stop = 10000000 );
	void tracePixel( int i, int j, unsigned char* sampleBuffer = NULL);

	void calculateRowWithSamples(unsigned char* sampleBuffer, int row);

	bool loadScene( char* fn );

	bool sceneLoaded();

	bool THRESHOLD(const vec3f& thresh) {
		for (int i = 0; i < 3; ++i) {
			if (thresh[i] >= threshold) {
				return false;
			}
		}

		return true; // All 3 components are smaller
	}

	void loadBackground(unsigned char* bmp, int w, int h) {
		background = bmp;
		bg_width = w;
		bg_height = h;
	}

	int numSubPixels;
	int swidth, sheight;
	bool ssJitter;

	bool glossyReflection;
	int numReflectVectors;
	double glossyReflectionDeviation;

	int numShadowRays;

private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene *scene;

	int maxDepth;

	bool m_bSceneLoaded;

	double threshold;

	unsigned char* background;
	int bg_width;
	int bg_height;
};

#endif // __RAYTRACER_H__
