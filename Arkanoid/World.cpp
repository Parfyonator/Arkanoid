#include "World.h"
// #include <cmath>

World::World(): w(0), h(0), border_thickness(0), active(true), victorious(true) {
	active_bricks_count = 0;
}

World::World(int w_, int h_, float bt_, float gh_, float b_r, float b_v, float p_w, float p_h, float b_w, float b_h)
: w(w_), h(h_), border_thickness(bt_), gap_height(gh_), active(false), victorious(true) {
	active_bricks_count = w*h;

	// set brick dimensions
	Brick::h = b_h;
	Brick::w = b_w;

	float current_y = border_thickness,
		  current_x = border_thickness;

	// initialize bricks
	for( int i = 0; i < h; i++ ) {
		current_x = border_thickness;
		prcore::Array<Brick> row = prcore::Array<Brick>();
		for( int j = 0; j < w; j++ ) {
			row.PushBack(Brick(current_x, current_y, true));
			current_x += Brick::w + border_thickness;
		}
		bricks.PushBack(row);
		current_y += Brick::h + border_thickness;
	}

	// add height between platform and first row
	current_y += gap_height + border_thickness;

	// initialize ball
	float ball_r = b_r,
		  ball_v = b_v;
	this->ball = Ball(current_x/2, current_y - ball_r, ball_r, ball_v, -ball_v, true);

	// initialize platform
	float platform_w = p_w;
	float platform_h = p_h;

	current_x = (current_x - platform_w) / 2;
	this->platform = Platform(current_x, current_y, platform_w, platform_h, true);

	// set borders positions
	border_l = 0;
	border_r = (Brick::w + border_thickness) * this->w + border_thickness;
	border_u = 0;
	border_d = current_y + platform_h + 5; //border_thickness;
}

bool World::is_active() {
	return this->active;
}

bool World::is_victorious() {
	return this->victorious;
}

float World::get_height() {
	return this->border_d;
}

float World::get_width() {
	return this->border_r;
}

int World::get_active_bricks_count() {
	return active_bricks_count;
}

void World::change_activity() {
	this->active = !this->active;
}

void World::run(float delta_time, float mouse_x) {
	if( !this->active )
		return;

	// process out of field case
	if( ball.get_y() > border_d ) {
		ball.change_activity();
		platform.change_activity();
		this->active = false;
		this->victorious = false;
		return;
	}

	/* process ball collisions */
	// collision with borders
	if( ball.get_x() - ball.get_r() < border_l && ball.get_vx() < 0 )
		ball.flip_vx();
	if( ball.get_y() - ball.get_r() < border_u && ball.get_vy() < 0 )
		ball.flip_vy();
	if( ball.get_x() + ball.get_r() > border_r && ball.get_vx() > 0 )
		ball.flip_vx();

	// collision with platform
	collision_with_rec(&ball, platform.get_x(), platform.get_y(), platform.get_w(), platform.get_h());

	// collision with bricks
	for( int i = 0; i < h; i++ ) {
		for( int j = 0; j < w; j++ ) {
			if( bricks[i][j].is_active() )
				if( collision_with_rec(&ball, bricks[i][j].get_x(), bricks[i][j].get_y(), Brick::w, Brick::h) ) {
					bricks[i][j].change_activity();
					active_bricks_count--;
				}
		}
	}

	/* move objects */
	// move platform
	platform.move(border_l, border_r, mouse_x);
	// move ball
	ball.move(delta_time);
}

void World::draw(prcore::Bitmap *buffer, MyImages *images) {
	// draw background
	buffer->BlitImage(0, 0, images->background);
	// draw objects
	ball.draw(buffer, images);
	platform.draw(buffer, images);
	for( int i = 0; i < h; i++ )
		for( int j = 0; j < w; j++ )
			bricks[i][j].draw(buffer, images);
}

float World::my_length(float dX, float dY) {
	return prcore::sqrtf( pow(dX, 2) + pow(dY, 2) );
}

bool World::collision_with_rec(Ball* b, float X, float Y, float W, float H) {
	if( b->get_x() < X ) {
		// left to platform
		if( b->get_y() < Y ) {
			// above
			if( my_length(b->get_y() - Y, b->get_x() - X) < b->get_r() ){
				if( b->get_vx() > 0 ) {
					b->flip_vx();
					if( b->get_vy() > 0 )
						b->flip_vy();
					else {}
				} else {
					if( b->get_vy() > 0 )
						b->flip_vy();
					else {}
				}
				return true;
			} else {}
		} else if( b->get_y() < Y + H ) {
			// at the same level
			if( b->get_x() + b->get_r() > X && b->get_vx() > 0 ) {
				b->flip_vx();
				return true;
			} else {}
		} else {
			// below
			if( my_length(b->get_y() - Y - H, b->get_x() - X) < b->get_r() ){
				if( b->get_vx() > 0 ) {
					b->flip_vx();
					if( b->get_vy() < 0 )
						b->flip_vy();
					else {}
				} else {
					if( b->get_vy() < 0 )
						b->flip_vy();
					else {}
				}
				return true;
			} else {}
		}
	} else if( b->get_x() < X + W ) {
		// in the middle
		if( b->get_y() < Y ){
			// above
			if( b->get_y() + b->get_r() > Y && b->get_vy() > 0 ) {
				b->flip_vy();
				return true;
			} else {}
		} else {
			// below
			if( b->get_y() - b->get_r() < Y + H && b->get_vy() < 0 ) {
				b->flip_vy();
				return true;
			} else {}
		}
	} else {
		// right to platform
		if( b->get_y() < Y ) {
			// above
			if( my_length(b->get_y() - Y, b->get_x() - X - W) < b->get_r() ){
				if( b->get_vx() > 0 ) {
					if( b->get_vy() > 0 )
						b->flip_vy();
					else {}
				} else {
					b->flip_vx();
					if( b->get_vy() > 0 )
						b->flip_vy();
					else {}
				}
				return true;
			} else {}
		} else if( b->get_y() < Y + H ) {
			// at the same level
			if( b->get_x() - b->get_r() < X + W && b->get_vx() < 0 ) {
				b->flip_vx();
				return true;
			} else {}
		} else {
			// below
			if( my_length(b->get_y() - Y - H, b->get_x() - X - W) < b->get_r() ){
				if( b->get_vx() > 0 ) {
					if( b->get_vy() < 0 )
						b->flip_vy();
					else {}
				} else {
					b->flip_vx();
					if( b->get_vy() < 0 )
						b->flip_vy();
					else {}
				}
				return true;
			} else {}
		}
	}

	return false;
}

World::~World() {
	while( !bricks.IsEmpty() ){
		prcore::Array<Brick> row = bricks.PopBack();
		while( !row.IsEmpty() )
			row.PopBack().~Brick();
	}
}