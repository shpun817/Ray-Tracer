#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"
#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;
	virtual bool isSpotLight() {
		return false;
	}

protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}

	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;

protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight( Scene *scene, const vec3f& pos, const vec3f& color )
		: Light( scene, color ), position( pos ), constant_attenuation_coeff(0), linear_attenuation_coeff(0), quadratic_attenuation_coeff(0) {}
	PointLight(Scene* scene, const vec3f& pos, const vec3f& color, double a, double b, double c)
		: Light(scene, color), position(pos), constant_attenuation_coeff(a), linear_attenuation_coeff(b), quadratic_attenuation_coeff(c){}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;


protected:
	vec3f position;
	double constant_attenuation_coeff;
	double linear_attenuation_coeff;
	double quadratic_attenuation_coeff;
};

class SpotLight
	: public Light
{
public:
	SpotLight(Scene* scene, const vec3f& pos, const vec3f& orien, const vec3f& color, const int ang)
		: Light(scene, color), position(pos), orientation(orien), angle(ang), attenuationConstant(0.0), attenuationLinear(0.0), attenuationQuadric(0.0) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	void setDistanceAttenuation(const double constant, const double linear, const double quadric);
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;

protected:
	vec3f position;
	vec3f orientation;
	int angle;
	double attenuationConstant;
	double attenuationLinear;
	double attenuationQuadric;
};


#endif // __LIGHT_H__
