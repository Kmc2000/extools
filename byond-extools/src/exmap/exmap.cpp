#include "exmap.h"

#include "../core/core.h"
#include "../dmdism/opcodes.h"

#include <cmath>
#include <chrono>
#include <memory>

/*
Welcome to Exmap! A physics engine extension for NSV13's overmap physics. This file focuses on performing collision calculation exclusively, most vector data handling is done on BYOND itself for ease of use.
This takes most of the heavy lifting off of BYOND and does it in C++ land.
If you don't understand what's going on here, I don't blame you. There's a lot of this code which is messy, due to interfacing with the BYOND engine.
@author Kmc2000

*/

/// <summary>
/// Class to hold x&y coords, not to be confused with std::vector
/// </summary>
class vector {
public:
	float x=0;
	float y=0;
	inline void set(float x, float y) {
		this->x = x;
		this->y = y;
	}
	inline void set(List L) {
		this->x = L.at(0);
		this->y = L.at(1);
	}
	inline float dot(vector& other) {
		return (this->x * other.x) + (this->y * other.y);
	}
	inline void reverse() {
		set(-x, -y);
	}
};

/// <summary>
/// Class to hold information about a collision, used for neatly passing data back and forth.
/// </summary>

class collision_response {
public:
	float overlap = INFINITY;
	vector overlap_normal;
	vector overlap_vector;
	vector overlap_point;
	bool a_in_b = true, b_in_a = true;
	inline collision_response() {
		this->overlap_normal.set(0, 0);
		this->overlap_vector.set(0, 0);
		this->overlap_point.set(0, 0);
	}
	inline virtual ~collision_response() {
	}
};

/// <summary>
/// Basic axis aligned bounding box hit detection, cheap, and helps for the first stage collisions.
/// </summary>
/// <param name="args_len"></param>
/// <param name="args"></param>
/// <param name="src"></param>
/// <returns>Whether the two shapes collide</returns>
trvh collides_aabb(unsigned int args_len, Value* args, Value src) {
	List l1 = List(args[0]);
	List l2 = List(args[1]);
	List aabb = List(args[2]);
	List other_aabb = List(args[3]);
	vector position;
	position.set(l1);
	vector other_position;
	other_position.set(l2);
	bool hit = ((aabb.at(0) + position.x) <= (other_aabb.at(2) + other_position.x)) && \
		((aabb.at(1) + position.y) <= (other_aabb.at(3) + other_position.y)) && \
		((aabb.at(2) + position.x) >= (other_aabb.at(0) + other_position.x)) && \
		((aabb.at(3) + position.y) >= (other_aabb.at(1) + other_position.y));
	return hit ? Value::True() : Value::Null();
}
/// <summary>
///	Get the intersecting segment between two shapes, returned as a list(x,y).
/// </summary>
/// <param name="args_len"></param>
/// <param name="args"></param>
/// <param name="src"></param>
/// <returns>Either false, or the x,y coords of the intersection.</returns>
trvh get_seg_intersect(unsigned int args_len, Value* args, Value src)
{
	//Data comes in as a loooad of numbers. Yes this is messy, it's also fast.
	List l = List(args[0]);

	float p0_x = args[0];
	float p0_y = args[1];

	float p1_x = args[2];
	float p1_y = args[3];

	float p2_x = args[4];
	float p2_y = args[5];

	float p3_x = args[6];
	float p3_y = args[7];

	float s10_x = p1_x - p0_x;
	float s10_y = p1_y - p0_y;
	float s32_x = p3_x - p2_x;
	float s32_y = p3_y - p2_y;

	float denom = (s10_x * s32_y) - (s32_x * s10_y);

	if (denom <= 0) { //No intersection, theyre colinear
		return Value::False();
	}

	bool denom_is_positive = denom > 0.0;

	float s02_x = p0_x - p2_x;
	float s02_y = p0_y - p2_y;

	float s_numer = (s10_x * s02_y) - (s10_y * s02_x);

	if ((s_numer < 0) == denom_is_positive) { //No collision here.
		return Value::False();
	}

	float t_numer = (s32_x * s02_y) - (s32_y * s02_x);

	if ((t_numer < 0) == denom_is_positive) { //Annnnd no collision found.
		return Value::False();
	}

	if ((s_numer > denom) == denom_is_positive || (t_numer > denom) == denom_is_positive) { //Annnnd no collision found.
		return Value::False();
	}

	float t = t_numer / denom;

	List out = CreateList(0);
	out.append(p0_x + (t * s10_x));
	out.append(p0_y + (t * s10_y));
	return Value(out);
}
/// <summary>
/// Flattens out shape points into a single vector, used in collision math.
/// </summary>
/// <param name="args_len"></param>
/// <param name="args"></param>
/// <param name="src"></param>
/// <returns></returns>
trvh flatten_points_on(unsigned int args_len, Value* args, Value src) {
	float minpoint = INFINITY;
	float maxpoint = -INFINITY;

	List points = List(args[0]); //Honey! Time for your points flattening.
	vector normal;
	normal.set(args[1], args[2]);
	int point_count = (int) args[3];

	for (int I = 0; I < point_count; I++) {
		vector point;
		point.set(points.at(I), points.at(I+1));
		I++;
		float dot = point.dot(normal);
		minpoint = (dot < minpoint) ? dot : minpoint;
		maxpoint = (dot > maxpoint) ? dot : maxpoint;
	}
	List out = CreateList(0);
	out.append(minpoint); //change to minpoint
	out.append(maxpoint); //change to maxpoint
	return Value(out);
}

trvh is_separating_axis(unsigned int args_len, Value* args, Value src) {
	vector a_pos, b_pos, range_a, range_b, axis;
	collision_response c_response;
	a_pos.set(args[0], args[1]);
	b_pos.set(args[2], args[3]);
	range_a.set(args[4], args[5]);
	range_b.set(args[6], args[7]);
	axis.set(args[8], args[9]);

	vector offset_v;
	offset_v.set(b_pos.x - a_pos.x, b_pos.y - a_pos.y);
	float projected_offset = offset_v.dot(axis);
	range_b.x += projected_offset;
	range_b.y += projected_offset;
	//A separating axis (line) can be drawn between the two shapes, ergo no collision!
	if (range_a.x > range_b.y || range_b.x > range_a.y) {
		return Value::True();
	}
	//And now, let's figure out more details about the collision....
	float overlap = 0;
	if (range_a.x < range_b.x) {
		c_response.a_in_b = false;
		if (range_a.y < range_b.y) {
			overlap = range_a.y - range_b.x;
			c_response.b_in_a = false;
		}
		else {
			float o1, o2;
			o1 = range_a.y - range_b.x;
			o2 = range_b.y - range_a.x;
			overlap = o1 < o2 ? o1 : -o2;
		}
	}
	else {
		c_response.b_in_a = false;
		if (range_a.y > range_b.y) {
			overlap = range_a.x - range_b.y;
			c_response.a_in_b = false;
		}
		else {
			float o1, o2;
			o1 = range_a.y - range_b.x;
			o2 = range_b.y - range_a.x;
			overlap = o1 < o2 ? o1 : -o2;
		}
	}
	if (abs(overlap) < c_response.overlap) {
		c_response.overlap = abs(overlap);
		c_response.overlap_normal.set(axis.x, axis.y);
		if (overlap < 0) {
			c_response.overlap_normal.reverse();
		}
	}
	//And, some messy data passing through to BYOND land.
	List out = CreateList(0);
	out.append(c_response.a_in_b);
	out.append(c_response.b_in_a);
	out.append(c_response.overlap);

	List overlap_normal, overlap_vector, overlap_point;
	overlap_normal = CreateList(0);
	overlap_normal.append(c_response.overlap_normal.x);
	overlap_normal.append(c_response.overlap_normal.y);
	out.append(Value(overlap_normal));
	
	overlap_vector = CreateList(0);
	overlap_vector.append(c_response.overlap_vector.x);
	overlap_vector.append(c_response.overlap_vector.y);
	out.append(Value(overlap_vector));

	overlap_point = CreateList(0);
	overlap_point.append(c_response.overlap_point.x);
	overlap_point.append(c_response.overlap_point.y);
	out.append(Value(overlap_point));

	return out;

}
/// <summary>
/// Enables the exmap! Called by EXMAP_EXTOOLS_CHECK in BYOND. Handles proc hooking into functions.
/// </summary>
/// <returns></returns>
const char* enable_exmap(){

	Core::get_proc("/datum/shape/proc/__test_aabb").hook(collides_aabb);
	Core::get_proc("/datum/shape/proc/__get_seg_intersection").hook(get_seg_intersect);
	Core::get_proc("/datum/shape/proc/__flatten_points_on").hook(flatten_points_on);
	Core::get_proc("/datum/shape/proc/__is_separating_axis").hook(is_separating_axis);
    return "ok";
}