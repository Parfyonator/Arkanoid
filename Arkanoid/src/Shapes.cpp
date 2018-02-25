#include "Shapes.h"
#include <prcore/prcore.hpp>

// basic shape
Shape::Shape(): x(0), y(0), active(false) {
}

Shape::Shape(float x_, float y_, bool active_): x(x_), y(y_), active(active_) {
}

void Shape::change_activity() {
	this->active = !this->active;
}

bool Shape::is_active() {
	return this->active;
}

Shape::~Shape() {
}


// Brick
float Brick::h = 20;
float Brick::w = 20;

Brick::Brick(): Shape() {
}

Brick::Brick(float x_, float y_, bool active_): Shape(x_, y_, active_) {
}

float Brick::get_x() {
	return this->x;
}

float Brick::get_y() {
	return this->y;
}

void Brick::draw(prcore::Bitmap *buffer, MyImages *images) {
	if( !this->is_active() )
		return;
	buffer->BlitImage(int(this->x), int(this->y), images->bricks[int(this->x + this->y) % images->bricks.GetSize()]);
}

Brick::~Brick() {
}


// Ball
Ball::Ball(): Shape(), r(0), vx(0), vy(0) {
}

Ball::Ball(float x_, float y_, float r_, float vx_, float vy_, bool active_): Shape(x_, y_, active_), r(r_), vx(vx_), vy(vy_) {
}

float Ball::get_r() {
	return this->r;
}

float Ball::get_x() {
	return this->x;
}

float Ball::get_y() {
	return this->y;
}

float Ball::get_vx() {
	return this->vx;
}

float Ball::get_vy() {
	return this->vy;
}

void Ball::move(float dt) {
	if( !this->is_active() )
		return;

	this->x += dt * this->vx;
	this->y += dt * this->vy;
}

void Ball::flip_vx() {
	this->vx *= -1;
}

void Ball::flip_vy() {
	this->vy *= -1;
}

void Ball::draw(prcore::Bitmap *buffer, MyImages *images) {
	buffer->BlitImage(int(this->x - this->r), int(this->y - this->r), images->ball);
}

Ball::~Ball() {
}


// Platform
Platform::Platform(): Shape(), w(0), h(0) {
}

Platform::Platform(float x_, float y_, float w_, float h_, bool active_): Shape(x_, y_, active_), w(w_), h(h_) {
}

float Platform::get_x() {
	return this->x;
}

float Platform::get_y() {
	return this->y;
}

float Platform::get_w() {
	return this->w;
}

float Platform::get_h() {
	return this->h;
}

void Platform::move(float x_min, float x_max, float x_mouse) {
	if( !this->is_active() )
		return;

	if( x_mouse < x_min + this->w/2 )
		this->x = 0;
	else if( x_mouse + this->w/2 > x_max )
		this->x = x_max - this->w;
	else
		this->x = x_mouse - x_min - this->w/2;
}

void Platform::draw(prcore::Bitmap *buffer, MyImages *images) {
	buffer->BlitImage(int(this->x), int(this->y), images->platform);
}

Platform::~Platform() {
}