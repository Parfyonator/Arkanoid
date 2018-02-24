#include "Shapes.h"
#include <prmath\prmath.hpp>

class World {
private:
	int w,					  // width in bricks
		h,					  // height in bricks
		active_bricks_count;

	bool active,
		 victorious;

	float border_thickness,   // object margin thickness
		  gap_height,		  // height from platform to first row of bricks
		  border_l,
		  border_r,
		  border_u,
		  border_d;

	prcore::Array< prcore::Array<Brick> > bricks;
	Ball ball;
	Platform platform;

	bool collision_with_rec(Ball* b, float X, float Y, float W, float H);

	float my_length(float dX, float dY);

public:
	World();

	World(int w_, int h_, float bt_, float gh_, float b_r, float b_v, float p_w, float p_h, float b_w, float b_h);

	bool is_victorious();

	bool is_active();

	void change_activity();

	void draw(prcore::Bitmap *buffer, MyImages *images);

	void run(float delta_time, float mouse_x);

	float get_width();

	float get_height();

	int get_active_bricks_count();

	~World();
};