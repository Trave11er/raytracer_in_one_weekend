#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "interval.h"
#include "rtweekend.h"
#include "camera.h"
#include <cmath>


double MAX_SPEED = 0.05;
bool DEBUG = false;

class PosVel {
    public:
        PosVel() {
            px = random_double() * 2 - 1;
            py = random_double() * 2 - 1;
            vx = MAX_SPEED * (random_double() * 2 - 1);
            vy = MAX_SPEED * (random_double() * 2 - 1);
            if (DEBUG)
                vy = 0;
        }

        PosVel(double px_, double py_, double vx_, double vy_) : px(px_), py(py_), vx(vx_), vy(vy_) {}

        void update() {
            px += vx;
            py += vy;
            if (px > 1)
                vx *= -1;
            if (px < -1)
                vx *= -1;
            if (py > 1)
                vy *= -1;
            if (py < -1)
                vy *= -1;        }

        point3 get_center() {
            return point3(px, 0.2, py);
        }

    private:
        double px, py;
        double vx, vy;
};

int main( int argc, char* argv[] ) {
    int THREAD_NUM = std::stoi(argv[1]);
    std::clog << THREAD_NUM << std::endl;

    // camera
    camera cam;

    cam.ideal_aspect_ratio = 16.0 / 9.0;
    cam.image_width        = 480;
    cam.samples_per_pixel  = 100;
    cam.max_depth          = 10;

    cam.vfov     = 30;
    cam.lookfrom = point3(0, 1, 3);
    cam.lookat   = point3(0, 0.2, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.focus_dist    = 3.0;

    cam.initialize();

    std::vector<std::thread> threads;
    int image_width = cam.image_width;
    int image_height = cam.image_height;
    int samples_per_pixel = cam.samples_per_pixel;

    // world
    hittable_list world;
    int num_objects = 5;
    if (DEBUG)
        num_objects = 2;
    std::vector<PosVel> pos_vel_vec;
    // initialise movement
    if (DEBUG) {
        pos_vel_vec.push_back(PosVel(-0.5, 0.0, MAX_SPEED, 0));
        pos_vel_vec.push_back(PosVel( 0.5, 0.0, -MAX_SPEED, 0));
    } else {
        for (int i = 0; i < num_objects; i++) {
            PosVel pv = PosVel();
            pos_vel_vec.push_back(pv);
        }
    }

    // movement
    int num_frames = 100;
    if (DEBUG)
        num_frames = 30;
    for (int frame=0; frame < num_frames; frame++) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        world.clear();
        threads.clear();  // makes sure we don't try to join a threads more than once (see below)
        material *ground_material = new lambertian(color(0.5, 0.5, 0.5));
        world.add(new sphere(point3(0,-1000,0), 1000, ground_material));

        // update object positions
        for (int i = 0; i < num_objects; i++) {
            pos_vel_vec[i].update();
            material* sphere_material;
            auto albedo = color(0.1, 0.5, 0.3);
            sphere_material = new lambertian(albedo);
            auto center = pos_vel_vec[i].get_center();
            world.add_blob(new sphere(center, RADIUS, sphere_material));
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
        std::ofstream ofs;
        // this is the name of the file being updated
        ofs.open ("image_new.ppm", std::ofstream::out | std::ofstream::trunc);
        ofs << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        write_color_arr(ofs, color_arr, image_height, image_width, samples_per_pixel);
        ofs.close();
        delete[] color_arr;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::clog << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
    }
}
