#include <prcore/prcore.hpp>

struct MyImages {
    prcore::Bitmap                    background,
                                    ball,
                                    platform;

    prcore::Array<prcore::Bitmap>   bricks;
};

class Shape {
protected:
    float x,
          y;
    bool  active;
public:
    Shape();

    Shape(float x_, float y_, bool active_);

    void change_activity();

    bool is_active();

    virtual void draw(prcore::Bitmap *buffer, MyImages *images) = 0;

    ~Shape();
};

class Brick: public Shape {
public:
    static float w,
                 h;

    Brick();

    Brick(float x_, float y_, bool active_);

    float get_x();

    float get_y();

    virtual void draw(prcore::Bitmap *buffer, MyImages *images);

    ~Brick();
};

class Ball: public Shape {
private:
    float r,
          vx,
          vy;
public:
    Ball();

    Ball(float x_, float y_, float r_, float vx_, float vy_, bool active_);

    float get_r();

    float get_x();

    float get_y();

    float get_vx();

    float get_vy();

    void move(float dt);

    void flip_vx();

    void flip_vy();

    virtual void draw(prcore::Bitmap *buffer, MyImages *images);

    ~Ball();
};

class Platform: public Shape {
private:
    float w,
          h;
public:
    Platform();

    Platform(float x_, float y_, float w_, float h_, bool active_);

    float get_x();

    float get_y();

    float get_w();

    float get_h();

    void move(float x_min, float x_max, float x_mouse);

    virtual void draw(prcore::Bitmap *buffer, MyImages *images);

    ~Platform();
};