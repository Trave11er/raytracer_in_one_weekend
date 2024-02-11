#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"


const interval ACNE_TO_INF_INTERVAL(SHADOW_ACNE_TOL, infinity);

class camera;

void calculate_block(int row_start, int row_end, const hittable_list& world, const camera& camera, color *color_arr);

color ray_color(const ray& r, int depth, const hittable_list& world) {
    hit_record rec;
    // If we've exceeded the ray bounce limit, no more light is gathered.

    if (depth <= 0)
        return color(0,0,0);

    if (world.hit(r, ACNE_TO_INF_INTERVAL, rec)) {
        ray scattered;
        color attenuation;
        // calculate scattered ray
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth-1, world);
        return color(0,0,0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    // uniform illumination background
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}


class camera {
  public:
    /* Public Camera Parameters Here */
    double ideal_aspect_ratio = 1.0;
    int image_width = 100;
    int    image_height;   // Rendered image height
    int samples_per_pixel = 10;   // Count of random samples for each pixel
    int max_depth = 10;   // Maximum number of ray bounces into scene

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,-1);  // Point camera is looking from
    point3 lookat   = point3(0,0,0);   // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point> to plane of perfect focus

    void initialize() {
        // Calculate the image height, and ensure that it's at least 1.
        image_height = static_cast<int>(image_width / ideal_aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        camera_center = lookfrom;

        auto vfov_rad = degrees_to_radians(vfov);
        auto viewport_height = 2 * focus_dist * tan(vfov_rad / 2);
        auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = camera_center - focus_dist * w - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int row, int col) const {
        auto pixel_center = pixel00_loc + (col * pixel_delta_u) + (row * pixel_delta_v);
        auto pixel_sample = pixel_center + pixel_sample_square();
        //auto ray_origin = camera_center;
        auto ray_origin = (defocus_angle <= 0) ? camera_center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }
  private:
    /* Private Camera Variables Here */
    point3 camera_center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;
    vec3   defocus_disk_u;  // Defocus disk horizontal radius
    vec3   defocus_disk_v;  // Defocus disk vertical radius


    vec3 pixel_sample_square() const {
        // Returns a random point in the square surrounding a pixel at the origin.
        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

};

void calculate_block(int row_start, int row_end, const hittable_list& world, const camera& camera, color *color_arr) {
    for (int l = row_start; l < row_end; l++) {
        int i = l / camera.image_width;
        int j = l % camera.image_width;
        color sum_pixel_color(0, 0, 0);
        for (int k = 0; k < camera.samples_per_pixel; k++) {
            auto ray_ = camera.get_ray(i, j);
            sum_pixel_color += ray_color(ray_, camera.max_depth, world);
        }
        color_arr[l] = sum_pixel_color;

    }
}


#endif
