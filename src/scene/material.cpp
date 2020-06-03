#include "ray.h"
#include "material.h"
#include "light.h"
#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

double CLAMP(double d) {
	return maximum(0.0, minimum(d, 1.0));
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
	
	// Iteration 0
	vec3f I = ke;

	// Iteration 1
	double ui_ambientLight = traceUI->getAmbientLight();
	for (int i = 0; i < 3; ++i) { // Sum on RGB channels
		// Ia is ambient light input from ui plus the value set in scene (.ray file)
		I[i] += (1 - kt[i]) * ka[i] * CLAMP(ui_ambientLight + scene->getAmbientLight()[i]);
	}

	// Iteration 2+
	//vec3f p = r.getPosition();
	vec3f d = r.getDirection().normalize(); // -d is viewing direction V
	vec3f Q = r.at(i.t); // intersection of ray and surface
	for (list<Light*>::const_iterator itr = scene->beginLights(); itr != scene->endLights(); ++itr) { // For each light source in the scene

		if ((*itr)->isSpotLight()) {
			SpotLight* ptr = (SpotLight*)(*itr);
			//double AdB = (Q - ptr->getPosition()).normalize().dot(ptr->getDirection(Q).normalize());
			//if (AdB < ptr->getCosCutOffAngle()) { // cosine smaller, angle bigger
			//	continue; // This spotlight does not contribute
			//}
		}
		// Determine the terms
		vec3f Ij = (*itr)->getColor(Q);
		vec3f Lj = (*itr)->getDirection(Q);
		double NdLj = fabs(i.N.dot(Lj));
		double VdRj = fabs( (2.0 * i.N * (i.N.dot(Lj)) - Lj).dot(-d) );
		VdRj = pow(VdRj, shininess * 128.0);
		double distance_atten = (*itr)->distanceAttenuation(Q);
		vec3f shadow_atten = (*itr)->shadowAttenuation(Q);

		for (int i = 0; i < 3; ++i) {
			I[i] += shadow_atten[i] * Ij[i] * ((1 - kt[i]) * kd[i] * NdLj + ks[i] * VdRj) * distance_atten;
		}
	}
	for (int i = 0; i < 3; ++i) {
		I[i] *= traceUI->getIntensityScale();
	}
	return I.clamp();
}
