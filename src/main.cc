#include <iostream>
#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "interval.h"
#include "rtweekend.h"
#include "camera.h"


bool is_inside(const vec3 &ray_direction, const vec3 &outward_normal) {
    if (dot(ray_direction, outward_normal) > 0.0) {
        // ray is inside the sphere
        return false;
    } else {
        // ray is outside the sphere
        return true;
    }
}



int main() {
    hittable_list world;

    world.add(make_shared<sphere>(point3(0,0,-1), 0.5));
    world.add(make_shared<sphere>(point3(0,-100.5,-1), 100));

    camera cam;

    cam.ideal_aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;
    cam.render(world);
}
