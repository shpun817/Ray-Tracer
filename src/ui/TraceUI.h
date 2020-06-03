//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include "../vecmath/vecmath.h"

#include <FL/fl_file_chooser.H>		// FLTK file chooser

#include "TraceGLWindow.h"

class TraceUI {
public:
	TraceUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;

	Fl_Slider*			m_ambientLightSlider;
	Fl_Slider*			m_distanceScaleSlider;
	
	Fl_Slider*			m_attenConstSlider;
	Fl_Slider*			m_attenLinearSlider;
	Fl_Slider*			m_attenQuadSlider;

	Fl_Slider*			m_intensityScaleSlider;
	Fl_Button*			m_intensityScaleResetButton;

	Fl_Light_Button*	m_superSamplingButton;
	Fl_Light_Button*	m_ssJitterButton;
	Fl_Slider*			m_numSubPixelsSlider;

	Fl_Slider*			m_thresholdSlider;

	Fl_Light_Button*	m_glossyReflectionButton;

	Fl_Light_Button*	m_softShadowButton;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	TraceGLWindow*		m_traceGlWindow;

	double getAmbientLight() const {
		return m_nAmbientLight;
	}
	
	double getDistanceScale() const {
		return m_nDistanceScale;
	}

	double getAttenConst() const {
		return m_nAttenConst;
	}

	double getAttenLinear() const {
		return m_nAttenLinear;
	}

	double getAttenQuad() const {
		return m_nAttenQuad;
	}

	double getIntensityScale() const {
		return m_nIntensityScale;
	}

	double getThreshold() const {
		return m_nThreshold;
	}

	// member functions
	void show();

	void		setRayTracer(RayTracer *tracer);

	void changeSamplingStatus() {
		if (m_superSamplingButton->value()) {
			m_numSubPixelsSlider->activate();
			m_ssJitterButton->activate();

		}
		else {
			m_numSubPixelsSlider->deactivate();
			m_ssJitterButton->deactivate();
		}
	}

	int			getSize();
	int			getDepth();

private:
	RayTracer*	raytracer;

	int			m_nSize;
	int			m_nDepth;

	double		m_nAmbientLight;
	double		m_nDistanceScale;

	double		m_nAttenConst;
	double		m_nAttenLinear;
	double		m_nAttenQuad;

	double		m_nIntensityScale;

	int			m_nNumSubPixels;

	double		m_nThreshold;

// static class members
	static Fl_Menu_Item menuitems[];

	static TraceUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);

	static void cb_ambientLightSlides(Fl_Widget* o, void* v);
	static void cb_distanceScaleSlides(Fl_Widget* o, void* v);
	
	//static void cb_attenConstSlides(Fl_Widget* o, void* v);
	//static void cb_attenLinearSlides(Fl_Widget* o, void* v);
	//static void cb_attenQuadSlides(Fl_Widget* o, void* v);

	static void cb_intensityScaleSlides(Fl_Widget* o, void* v);
	static void cb_intensityScaleReset(Fl_Widget* o, void* v);

	static void cb_superSamplingButton(Fl_Widget* o, void* v);
	static void cb_numSubPixelsSlides(Fl_Widget* o, void* v);

	static void cb_thresholdSlides(Fl_Widget* o, void* v);

	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
};

#endif
