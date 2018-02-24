#define PRCORE_IMPLEMENT_MAIN
#include <prmath\prmath.hpp>
#pragma comment (lib, "ddraw.lib")
#include "../World.h"

using namespace prcore;

// world parameters
float ball_radius = 5,
	  ball_velocity = 200,
	  platform_width = 60,
	  platform_height = 5,
	  border_thickness = 1,
	  gap_height = 150,
	  brick_width = 20,
	  brick_height = 20;

int   wall_width = 10,   // in bricks
	  wall_height = 5;  // in bricks

struct MyColor {
	uint32 r,g,b,a;

	MyColor(uint32 red, uint32 green, uint32 blue, uint32 alpha)
		: r(red), g(green), b(blue), a(alpha) {}
};

Bitmap create_circle_image(float, MyColor, MyColor);

// =============================================================
// window class
// =============================================================

class TestWindow : public FrameBuffer
{
	public:

	TestWindow()
	{
		// create world
		world  = World(wall_width, wall_height, border_thickness, gap_height, ball_radius, ball_velocity, platform_width, platform_height, brick_width, brick_height);

		// some colors
		MyColor bg_color = MyColor(511, 511, 511, 0x00000000);
		MyColor c_color = MyColor(0, 0, 0, 0x00000000);

		// init images for world objects
		images.background = Bitmap(world.get_width(), world.get_height(), PixelFormat(32,bg_color.r,bg_color.g,bg_color.b, bg_color.a));
		images.platform   = Bitmap(platform_width, platform_height, PixelFormat(32,0,511,0, 0x00000000));
		images.ball = create_circle_image(ball_radius, bg_color, c_color);
		for( int i = 0; i < wall_height; i++ )
			images.bricks.PushBack(Bitmap(brick_width, brick_height, PixelFormat(32, 200 + i*40, 300 + i*40, 400 + i*40, 0x00000000)));
		
		// open framebuffer
		OpenBuffer(world.get_width(), world.get_height(), "Arkanoid");

		// init buffer bitmap
		buffer = Bitmap(world.get_width(), world.get_height(), PIXELFORMAT_ARGB8888);
	}

	~TestWindow()
	{
	}

	bool EventMain()
	{
		EventDraw();
		return true;
	}
	
	void EventMouseMove(int x, int y){
		this->mouse_x = x;
	}

	void EventDraw()
	{
		if( world.get_active_bricks_count() == 0 || !world.is_victorious() )
			world = World(wall_width, wall_height, border_thickness, gap_height, ball_radius, ball_velocity, platform_width, platform_height, brick_width, brick_height);

		float time = timer.GetTime();
		
		// draw world
		world.draw(&buffer, &images);
		BlitWrite(0, 0, buffer);

		// run world
		world.run(timer.GetTime() / timer.GetTick(), mouse_x);
		
		PageFlip();
	}

	void EventKeyboard(int keycode, char charcode, bool press)
	{
		if ( keycode == KEYCODE_ESC && press )
			MainBreak();

		if( IsDown(KEYCODE_SPACE) )
			world.change_activity();
	}

	private:

	Timer		timer;

	Bitmap		buffer;

	float       mouse_x;

	World       world;
	MyImages    images;
};


// =============================================================
// prmain()
// =============================================================

int prmain(int argc, char** argv)
{
	TestWindow window;
	window.MoveWindow(512, 234);
	window.MainLoop();
	return 0;
}

Bitmap create_circle_image(float radius, MyColor background_color, MyColor circle_color) {
	float r = 50;
	Bitmap circle = Bitmap(2*r, 2*r, PixelFormat(32, background_color.r, background_color.g, background_color.b, background_color.a));
	
	for( int i = 0; i < 100; i++ ) {
		float w_half = prcore::sqrtf(pow(r, 2) - pow(r - float(i), 2));
		circle.BlitImage(int(r - w_half), i, Bitmap(int(2*w_half), 1, PixelFormat(32, circle_color.r, circle_color.g, circle_color.b, circle_color.a)));
	}
	circle.ResizeImage(2*radius, 2*radius);

	return circle;
}