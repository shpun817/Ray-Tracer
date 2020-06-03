#include <cmath>

#include "light.h"

//bool ZERO(double d) {
//	return (fabs(d) < 1e-5);
//}
extern bool ZERO(double d);

extern RayTracer* theRayTracer;

bool ZERO(vec3f a) {
	return (ZERO(a[0]) && ZERO(a[1]) && ZERO(a[2]));
}

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	// Fire a shadow ray from P+eps to infinity
	ray shadow(P, getDirection(P).normalize());
	shadow = ray(shadow.at(1e-5), shadow.getDirection());

	// if the shadow ray hits anything, that means in reverse, the light is blocked
	isect i;
	if (scene->intersect(shadow, i)) {
		return i.getMaterial().kt;
	}
	else {
		return vec3f(1, 1, 1);
	}

    //return vec3f(1,1,1);
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation.normalize();
}


double PointLight::distanceAttenuation( const vec3f& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0
	
	double a = this->constant_attenuation_coeff + traceUI->getAttenConst();
	a = minimum(1.0, a);
	double b = this->linear_attenuation_coeff + traceUI->getAttenLinear();
	b = minimum(1.0, b);
	double c = this->quadratic_attenuation_coeff + traceUI->getAttenQuad();
	c = minimum(1.0, c);
	double d = (this->position - P).length() * traceUI->getDistanceScale();

	return minimum(1.0, 1.0 / (a + b * d + c * d * d));
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	// Fire a shadow ray from P to light source
	ray shadow(P, getDirection(P));
	shadow = ray(shadow.at(1e-5), shadow.getDirection());
	int numShadowRays = theRayTracer->numShadowRays;

	ray tempShadow = shadow;
	vec3f sum(0, 0, 0);
	for (int j = 0; j < numShadowRays; ++j) {
		isect i;
		if (!scene->intersect(tempShadow, i)) {
			// No intersection -> no occlusion
			sum += vec3f(1, 1, 1);
		}
		else if ((tempShadow.at(i.t) - P).length() > (position - P).length()) { // light is between P and the intersection
			sum += vec3f(1, 1, 1);
		}
		else {
			const Material& m = i.getMaterial();
			if (!ZERO(tempShadow.at(i.t) - P)) {
				// Shadow ray is blocked by some other objects (the ray intersects with some other objects)
				sum += m.kt;
			}
		}
		tempShadow = shadow.jiggle(0.1);
	}

	for (int i = 0; i < 3; ++i) {
		sum[i] /= numShadowRays;
	}

	return sum;

	// Should never reach this point
    //return vec3f(1,1,1);
}

vec3f SpotLight::getColor(const vec3f& P) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f SpotLight::getDirection(const vec3f& P) const
{
	return (position - P).normalize();
}

double SpotLight::distanceAttenuation(const vec3f& P) const
{

	double a = attenuationConstant;
	double b = attenuationLinear;
	double c = attenuationQuadric;
	double dist = (P - position).length();
	double drop = 1.0 / (a + b * dist + c * dist * dist);
	drop = (drop > 1.0) ? 1.0 : drop;
	return drop;
}
void SpotLight::setDistanceAttenuation(const double constant, const double linear, const double quadric) {
	attenuationConstant = constant;
	attenuationLinear = linear;
	attenuationQuadric = quadric;
}

vec3f SpotLight::shadowAttenuation(const vec3f& P) const
{
	double intensity = (P - position).normalize().dot(orientation.normalize());
	double cutoff = cos(angle * 3.1415926535 / 180);

	int index = 0; // Assume no contribute --> no colour
	if (intensity >= cutoff) index = 1;

	vec3f rayDirection = (position - P).normalize();
	isect intersect;
	ray shadowRay = ray(P, rayDirection);

	vec3f atten = getColor(P);


	if (!(scene->intersect(shadowRay, intersect))) {
		return getColor(P);
	}

	while (scene->intersect(shadowRay, intersect)) {

		//check if intersection is beyond light
		double lightPosition = (position - shadowRay.getPosition()).length();
		if (lightPosition - intersect.t < RAY_EPSILON) {
			return getColor(P);
		}

		//check if the object is transparent
		if (intersect.getMaterial().kt.iszero()) {
			return vec3f(0, 0, 0);
		}
		else {
			atten = prod(atten, intersect.getMaterial().kt);
			shadowRay = ray(shadowRay.at(intersect.t), rayDirection);
		}
	}

	atten *= index;
	return atten;
}
