#include <iostream>
#include <chrono>
#include <thread>
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

int main( int argc, char* argv[] ) {
    int THREAD_NUM = std::stoi(argv[1]);
    std::clog << THREAD_NUM << std::endl;

    // camera
    camera cam;

    cam.ideal_aspect_ratio = 16.0 / 9.0;
    cam.image_width        = 240;
    cam.samples_per_pixel  = 500;
    cam.max_depth          = 5;

    cam.vfov     = 5;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    cam.initialize();

    std::vector<std::thread> threads;
    int image_width = cam.image_width;
    int image_height = cam.image_height;
    int samples_per_pixel = cam.samples_per_pixel;
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    // world
    hittable_list world;

    // movement
    for (int frame=0; frame < 1; frame++) {
    world.clear();
    material *ground_material = new lambertian(color(0.5, 0.5, 0.5));
    world.add(new sphere(point3(0,-1000,0), 1000, ground_material));

    for (int a = -1; a < 1; a++) {
        for (int b = -1; b < 1; b++) {
            auto choose_mat = random_double();
            point3 center(0.1*a + 0.09*random_double(), 0.2, 0.1*b + 0.09*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                material* sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    //auto albedo = random_vec() * random_vec();
                    auto albedo = color(0.1, 0.5, 0.3);
                    sphere_material = new lambertian(albedo);
                    world.add(new sphere(center, 0.2, sphere_material));
               }
            }
        }
    }

    // loop
    color *color_arr = new color[image_width * image_height];
    int row_end = 0, row_start = -1;
    for (int m = 0; m < THREAD_NUM; m++) {
        row_start = row_end;
        row_end = static_cast<int>(round((m + 1) * image_height * image_width / THREAD_NUM));
        std::thread t(calculate_block, row_start, row_end, std::ref(world), std::ref(cam), std::ref(color_arr));
        threads.push_back(std::move(t));
    }

    for (std::thread& t : threads)
    {
        t.join();
    }
    write_color_arr(std::cout, color_arr, image_height, image_width, samples_per_pixel);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::clog << "\rDone.                 \n";
    std::clog << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

    }
}
