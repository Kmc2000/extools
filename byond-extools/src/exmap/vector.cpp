#include "vector.h"


vector::vector(float x, float y){
	this->x = x;
	this->y = y;
}

float vector::get_x() {
	return this->x;
}

float vector::get_y() {
	return this->y;
}

void vector::set_x(float x) {
	this->x = x;
}

void vector::set_y(float y) {
	this->y = y;
}

/**
Mathematical functions start here!.
*/

//For when you need to add a number to a vector
void vector::operator +(float x) {
	this->x += x;
	this->y += y;
}
//Add another vector's stats to this vector
void vector::operator +(vector& other) {
	this->x += other.get_x();
	this->y += other.get_y();
}

void vector::operator -(float x) {
	this->x -= x;
	this->y -= y;
}

void vector::operator -(vector& other) {
	this->x -= other.get_x();
	this->y -= other.get_y();
}