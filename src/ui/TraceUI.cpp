//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <FL/fl_ask.h>

#include "TraceUI.h"
#include "../RayTracer.h"

static bool done;

extern unsigned char* readBMP(char* fname, int& width, int& height);

//------------------------------------- Help Functions --------------------------------------------
TraceUI* TraceUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (TraceUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			done=true;	// terminate the previous rendering
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}

void TraceUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void TraceUI::cb_exit(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_exit2(Fl_Widget* o, void* v) 
{
	TraceUI* pUI=(TraceUI *)(o->user_data());
	
	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}

void TraceUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI*)(o->user_data());
	
	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
}

void TraceUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void TraceUI::cb_ambientLightSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nAmbientLight = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_distanceScaleSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDistanceScale = (double)(((Fl_Slider*)o)->value());
}

/*
void TraceUI::cb_attenConstSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nAttenConst = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_attenLinearSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nAttenLinear = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_attenQuadSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nAttenQuad = (double)(((Fl_Slider*)o)->value());
}
*/

void TraceUI::cb_intensityScaleSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nIntensityScale = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_intensityScaleReset(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nIntensityScale = 1.0;
	((TraceUI*)(o->user_data()))->m_intensityScaleSlider->value(1.0);
}



void TraceUI::cb_superSamplingButton(Fl_Widget* o, void* v) {
	((TraceUI*)(o->user_data()))->changeSamplingStatus();
}

void TraceUI::cb_numSubPixelsSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nNumSubPixels = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_thresholdSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nThreshold = (double)(((Fl_Slider*)o)->value());
}

void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) {
		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_nAttenConst = (double)pUI->m_attenConstSlider->value();
		pUI->m_nAttenLinear = (double)pUI->m_attenLinearSlider->value();
		pUI->m_nAttenQuad = (double)pUI->m_attenQuadSlider->value();

		bool superSampling = pUI->m_superSamplingButton->value();
		unsigned char* sampleBuffer = NULL;
		int numSubPixels = pUI->m_nNumSubPixels;
		int swidth = width * numSubPixels - width + 1;
		int sheight = height * numSubPixels - height + 1;
		pUI->raytracer->ssJitter = false;

		pUI->raytracer->glossyReflection = pUI->m_glossyReflectionButton->value();

		if (pUI->m_softShadowButton->value()) {
			pUI->raytracer->numShadowRays = 25;
		}
		else {
			pUI->raytracer->numShadowRays = 1;
		}
		
		if (superSampling) {
			pUI->raytracer->numSubPixels = numSubPixels;
			pUI->raytracer->swidth = swidth;
			pUI->raytracer->sheight = sheight;
			pUI->raytracer->ssJitter = pUI->m_ssJitterButton->value();
			sampleBuffer = new unsigned char [swidth * sheight * 3];
		}

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here	
		done=false;
		clock_t prev, now;
		prev=clock();
		
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		if (!superSampling || numSubPixels == 1) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (done) break;

					// current time
					now = clock();

					// check event every 1/2 second
					if (((double)(now - prev) / CLOCKS_PER_SEC) > 0.5) {
						prev = now;

						if (Fl::ready()) {
							// refresh
							pUI->m_traceGlWindow->refresh();
							// check event
							Fl::check();

							if (Fl::damage()) {
								Fl::flush();
							}
						}
					}

					pUI->raytracer->tracePixel(x, y);

				}
				if (done) break;

				// flush when finish a row
				if (Fl::ready()) {
					// refresh
					pUI->m_traceGlWindow->refresh();

					if (Fl::damage()) {
						Fl::flush();
					}
				}
				// update the window label
				sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
				pUI->m_traceGlWindow->label(buffer);

			}
		}
		else {
			// Modified for Super Sampling

			// Trace the subpixels needed for the first row
			for (int y = 0; y < numSubPixels; ++y) {
				for (int x = 0; x < swidth; ++x) {
					// current time
					now = clock();

					// check event every 1/2 second
					if (((double)(now - prev) / CLOCKS_PER_SEC) > 0.5) {
						prev = now;

						if (Fl::ready()) {
							// refresh
							pUI->m_traceGlWindow->refresh();
							// check event
							Fl::check();

							if (Fl::damage()) {
								Fl::flush();
							}
						}
					}

					pUI->raytracer->tracePixel(x, y, sampleBuffer);
				}

				// flush when finish a row of subpixels
				if (Fl::ready()) {
					// refresh
					pUI->m_traceGlWindow->refresh();

					if (Fl::damage()) {
						Fl::flush();
					}
				}
				// update the window label
				sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)sheight * 100.0), old_label);
				pUI->m_traceGlWindow->label(buffer);
			}

			// For each row of subpixels, first determine if need to draw the row then trace the subpixels
			for (int y = numSubPixels; y < sheight; ++y) {

				if (numSubPixels != 1 && (y-1) % (numSubPixels-1) == 0) {
					// Draw the corresponding row into buffer
					pUI->raytracer->calculateRowWithSamples(sampleBuffer, y-numSubPixels); // Actually we are drawing the previous row (which was traced in previous iterations)
				}

				// Trace this row of subpixels
				for (int x = 0; x < swidth; x++) {
					if (done) break;

					// current time
					now = clock();

					// check event every 1/2 second
					if (((double)(now - prev) / CLOCKS_PER_SEC) > 0.5) {
						prev = now;

						if (Fl::ready()) {
							// refresh
							pUI->m_traceGlWindow->refresh();
							// check event
							Fl::check();

							if (Fl::damage()) {
								Fl::flush();
							}
						}
					}

					pUI->raytracer->tracePixel(x, y, sampleBuffer);

				}
				if (done) break;

				// flush when finish a row
				if (Fl::ready()) {
					// refresh
					pUI->m_traceGlWindow->refresh();

					if (Fl::damage()) {
						Fl::flush();
					}
				}
				// update the window label
				sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)sheight * 100.0), old_label);
				pUI->m_traceGlWindow->label(buffer);
			}

			// Draw the last row of output pixels
			pUI->raytracer->calculateRowWithSamples(sampleBuffer, sheight - numSubPixels);
		}
		
		done=true;
		pUI->m_traceGlWindow->refresh();
		delete [] sampleBuffer;

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);		
	}
}

void TraceUI::cb_stop(Fl_Widget* o, void* v)
{
	done=true;
}

void TraceUI::show()
{
	m_mainWindow->show();
}

void TraceUI::setRayTracer(RayTracer *tracer)
{
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}

int TraceUI::getSize()
{
	return m_nSize;
}

int TraceUI::getDepth()
{
	return m_nDepth;
}

// menu definition
Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)TraceUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

TraceUI::TraceUI() {
	// init.
	m_nDepth = 0;
	m_nSize = 150;

	m_nAmbientLight = 0.5;
	m_nDistanceScale = 1.0;

	m_nAttenConst = 0.2;
	m_nAttenLinear = 0.2;
	m_nAttenQuad = 0.1;

	m_nIntensityScale = 1.0;

	m_nNumSubPixels = 1;

	m_nThreshold = 0.0;

	m_mainWindow = new Fl_Window(100, 40, 360, 350, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 320, 25);
		m_menubar->menu(menuitems);

		// install slider depth
		m_depthSlider = new Fl_Value_Slider(10, 30, 180, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install slider size
		m_sizeSlider = new Fl_Value_Slider(10, 55, 180, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);
		
		// install slider ambient light
		m_ambientLightSlider = new Fl_Value_Slider(10, 80, 180, 20, "Ambient Light");
		m_ambientLightSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_ambientLightSlider->type(FL_HOR_NICE_SLIDER);
		m_ambientLightSlider->labelfont(FL_COURIER);
		m_ambientLightSlider->labelsize(12);
		m_ambientLightSlider->minimum(0.0);
		m_ambientLightSlider->maximum(1.0);
		m_ambientLightSlider->step(0.01);
		m_ambientLightSlider->value(m_nAmbientLight);
		m_ambientLightSlider->align(FL_ALIGN_RIGHT);
		m_ambientLightSlider->callback(cb_ambientLightSlides);
		
		// install slider distance scale
		m_distanceScaleSlider = new Fl_Value_Slider(10, 105, 180, 20, "Distance Scale");
		m_distanceScaleSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_distanceScaleSlider->type(FL_HOR_NICE_SLIDER);
		m_distanceScaleSlider->labelfont(FL_COURIER);
		m_distanceScaleSlider->labelsize(12);
		m_distanceScaleSlider->minimum(0.0);
		m_distanceScaleSlider->maximum(5.0);
		m_distanceScaleSlider->step(0.01);
		m_distanceScaleSlider->value(m_nDistanceScale);
		m_distanceScaleSlider->align(FL_ALIGN_RIGHT);
		m_distanceScaleSlider->callback(cb_distanceScaleSlides);

		// install slider attenuation constant
		m_attenConstSlider = new Fl_Value_Slider(10, 130, 180, 20, "Attenuation (Const)");
		m_attenConstSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_attenConstSlider->type(FL_HOR_NICE_SLIDER);
		m_attenConstSlider->labelfont(FL_COURIER);
		m_attenConstSlider->labelsize(12);
		m_attenConstSlider->minimum(0.0);
		m_attenConstSlider->maximum(1.0);
		m_attenConstSlider->step(0.01);
		m_attenConstSlider->value(m_nAttenConst);
		m_attenConstSlider->align(FL_ALIGN_RIGHT);
		//m_attenConstSlider->callback(cb_attenConstSlides);
		
		// install slider attenuation linear
		m_attenLinearSlider = new Fl_Value_Slider(10, 155, 180, 20, "Attenuation (Linear)");
		m_attenLinearSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_attenLinearSlider->type(FL_HOR_NICE_SLIDER);
		m_attenLinearSlider->labelfont(FL_COURIER);
		m_attenLinearSlider->labelsize(12);
		m_attenLinearSlider->minimum(0.0);
		m_attenLinearSlider->maximum(1.0);
		m_attenLinearSlider->step(0.01);
		m_attenLinearSlider->value(m_nAttenLinear);
		m_attenLinearSlider->align(FL_ALIGN_RIGHT);
		//m_attenLinearSlider->callback(cb_attenLinearSlides);

		// install slider attenuation quadratic
		m_attenQuadSlider = new Fl_Value_Slider(10, 180, 180, 20, "Attenuation (Quad)");
		m_attenQuadSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_attenQuadSlider->type(FL_HOR_NICE_SLIDER);
		m_attenQuadSlider->labelfont(FL_COURIER);
		m_attenQuadSlider->labelsize(12);
		m_attenQuadSlider->minimum(0.0);
		m_attenQuadSlider->maximum(1.0);
		m_attenQuadSlider->step(0.01);
		m_attenQuadSlider->value(m_nAttenQuad);
		m_attenQuadSlider->align(FL_ALIGN_RIGHT);
		//m_attenQuadSlider->callback(cb_attenQuadSlides);

		// install slider intensity scale
		m_intensityScaleSlider = new Fl_Value_Slider(10, 205, 180, 20, "Intensity");
		m_intensityScaleSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_intensityScaleSlider->type(FL_HOR_NICE_SLIDER);
		m_intensityScaleSlider->labelfont(FL_COURIER);
		m_intensityScaleSlider->labelsize(12);
		m_intensityScaleSlider->minimum(0.0);
		m_intensityScaleSlider->maximum(10.0);
		m_intensityScaleSlider->step(0.01);
		m_intensityScaleSlider->value(m_nIntensityScale);
		m_intensityScaleSlider->align(FL_ALIGN_RIGHT);
		m_intensityScaleSlider->callback(cb_intensityScaleSlides);

		// install button intensity scale reset
		m_intensityScaleResetButton = new Fl_Button(260, 202, 70, 25, "&Reset");
		m_intensityScaleResetButton->user_data((void*)(this));
		m_intensityScaleResetButton->callback(cb_intensityScaleReset);

		// install button Super Sampling
		m_superSamplingButton = new Fl_Light_Button(10, 230, 120, 25, "SuperSampling");
		m_superSamplingButton->user_data((void*)(this));
		m_superSamplingButton->callback(cb_superSamplingButton);
		
		// install button Super Sampling Jitter
		m_ssJitterButton = new Fl_Light_Button(135, 230, 50, 25, "Jitter");
		m_ssJitterButton->user_data((void*)(this));
		m_ssJitterButton->deactivate();

		// install slider number of subpixels
		m_numSubPixelsSlider = new Fl_Value_Slider(10, 255, 180, 20, "#. subpixels");
		m_numSubPixelsSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_numSubPixelsSlider->type(FL_HOR_NICE_SLIDER);
		m_numSubPixelsSlider->labelfont(FL_COURIER);
		m_numSubPixelsSlider->labelsize(12);
		m_numSubPixelsSlider->minimum(1);
		m_numSubPixelsSlider->maximum(5);
		m_numSubPixelsSlider->step(1);
		m_numSubPixelsSlider->value(m_nNumSubPixels);
		m_numSubPixelsSlider->align(FL_ALIGN_RIGHT);
		m_numSubPixelsSlider->callback(cb_numSubPixelsSlides);
		m_numSubPixelsSlider->deactivate();
		
		// install slider threshold
		m_thresholdSlider = new Fl_Value_Slider(10, 280, 180, 20, "Threshold");
		m_thresholdSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_thresholdSlider->type(FL_HOR_NICE_SLIDER);
		m_thresholdSlider->labelfont(FL_COURIER);
		m_thresholdSlider->labelsize(12);
		m_thresholdSlider->minimum(0.0);
		m_thresholdSlider->maximum(1.0);
		m_thresholdSlider->step(0.01);
		m_thresholdSlider->value(m_nThreshold);
		m_thresholdSlider->align(FL_ALIGN_RIGHT);
		m_thresholdSlider->callback(cb_thresholdSlides);

		// install button Glossy Reflection
		m_glossyReflectionButton = new Fl_Light_Button(10, 305, 65, 25, "Glossy");
		m_glossyReflectionButton->user_data((void*)(this));
		
		// install button Soft Shadow
		m_softShadowButton = new Fl_Light_Button(80, 305, 100, 25, "Soft Shadow");
		m_softShadowButton->user_data((void*)(this));

		m_renderButton = new Fl_Button(240, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		m_stopButton = new Fl_Button(240, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}