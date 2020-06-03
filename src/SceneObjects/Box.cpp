#include <cmath>
#include <assert.h>
#include <limits>

#include "Box.h"

bool ZERO(double d) {
	return (fabs(d) < 1e-5);
}

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// YOUR CODE HERE:
    // Add box intersection code here.
	// it currently ignores all boxes and just returns false.

	// The code here follows from the pseudocode in http://education.siggraph.org/static/HyperGraph/raytrace/rtinter3.htm
	
	// Define the planes that define the unit cube (actually it should be the translation of these planes, as the box is centered at the origin, but it doesn't matter)
	vec4f planes[6] = {
		// Ax+By+Cz+D=0
		vec4f(1,0,0,0), // x=0
		vec4f(0,1,0,0), // y=0
		vec4f(0,0,1,0), // z=0
		vec4f(1,0,0,-1), // x=1
		vec4f(0,1,0,-1), // y=1
		vec4f(0,0,1,-1) // z=1
	};
	
	
	// Define the unit normal vectors for each plane (assume the normal always points out of the box)
	vec3f normals[6];
	for (int i = 0; i < 6; ++i) {
		normals[i] = vec3f(planes[i][0], planes[i][1], planes[i][2]);
		if (planes[i][3] == 0) {
			normals[i] = -1.0 * normals[i];
		}
		normals[i] = normals[i].normalize();
	}

	vec3f normal; // This is the one that will be put to i.N, if intersection is found
	
	double Tnear = std::numeric_limits<double>::min(); // -infinity
	double Tfar = std::numeric_limits<double>::max(); // infinity

	vec3f p = r.getPosition();
	vec3f d = r.getDirection().normalize();

	// Define the min/max extent
	vec3f Bl(-0.5, -0.5, -0.5);
	vec3f Bh(0.5, 0.5, 0.5);

	for (int dim = 0; dim < 3; ++dim) { // x,y,z
		int plane1 = dim;
		int plane2 = plane1 + 3;

		// check if the ray is parallel to the planes
		if (ZERO(d[dim])) { // is parallel
			// Check if p is inside the slab
			if (p[dim] < Bl[dim] || p[dim] > Bh[dim]) { // not inside
				return false; // automatically no intersection here
			}
			continue; // Inside! -> check other planes
		}

		double T1 = (Bl[dim] - p[dim]) / d[dim];
		double T2 = (Bh[dim] - p[dim]) / d[dim];

		if (T1 > T2) { // Swap since T1 intersects with near plane
			double temp = T1;
			T1 = T2;
			T2 = temp;

			plane1 += plane2;
			plane2 = plane1 - plane2;
			plane1 = plane1 - plane2;
		}

		if (T1 > Tnear) {
			Tnear = T1; // Want largest Tnear
			// Update the normal
			if (!ZERO(Tnear) && Tnear > 0) { // if ray origin is not at box boundary and it is outside of the box, entry point is intersection
				normal = normals[plane1];
			}
		}

		if (T2 < Tfar) {
			Tfar = T2; // Want smallest Tifa
			// Update the normal
			if (ZERO(Tnear) || Tnear < 0) { // if ray origin is at box boundary or it is inside the box, exit point is the intersection
				normal = normals[plane2];
			}
		}

		if (Tnear > Tfar) {
			return false; // Missed
		}

		if (Tfar < 0) {
			return false; // Box is behind ray
		}
	}

	// Survived all tests!
	i.obj = this;
	i.N = normal;
	if (ZERO(Tnear) || Tnear < 0) { // Ray origin is at box bound or inside the box
		i.t = Tfar; // Want Tifa
	}
	else {
		i.t = Tnear; // Want Tnier
	}

	return true;
}
