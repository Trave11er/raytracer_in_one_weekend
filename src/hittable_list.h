#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "interval.h"

#include <assert.h>
#include <memory>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

using std::shared_ptr;
using std::make_shared;


class hittable_list {
  public:
    std::vector<hittable*> objects;
    std::vector<implicit_surface*> blobs;

    hittable_list() {}

    void clear() {
        for (auto* o : objects) delete o;
        for (auto* b : blobs) delete b;
        objects.clear();
        blobs.clear();
    }

    void add(hittable *object) {
        objects.push_back(object);
    }

    void add_blob(implicit_surface *blob) {
        blobs.push_back(blob);
    }

    bool hit(const ray& r, const interval ray_t, hit_record& rec) const {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest), temp_rec)) {
                hit_anything = true;
                closest = temp_rec.t;
                rec = temp_rec;
            }
        }

        if (DISABLE_RAY_MARCHING) {
            for (const auto& blob : blobs) {
                if (blob->hit(r, interval(ray_t.min, closest), temp_rec)) {
                    hit_anything = true;
                    closest = temp_rec.t;
                    rec = temp_rec;
                }
            }
            return hit_anything;
        }

        if (!blobs.empty() && march_metaballs(r, ray_t.min, closest, rec))
            return true;

        return hit_anything;
    }

    size_t size() {
        return objects.size() + blobs.size();
    }

  private:
    double total_potential(point3 p) const {
        double pot = 0.0;
        for (const auto& blob : blobs)
            pot += blob->get_potential(p);
        return pot;
    }

    implicit_surface* dominant_blob(point3 p) const {
        implicit_surface* best = blobs[0];
        double best_pot = best->get_potential(p);
        for (size_t i = 1; i < blobs.size(); i++) {
            double pot = blobs[i]->get_potential(p);
            if (pot > best_pot) {
                best_pot = pot;
                best = blobs[i];
            }
        }
        return best;
    }

    vec3 isosurface_normal(point3 p) const {
        vec3 grad(0, 0, 0);
        for (const auto& blob : blobs)
            grad += blob->get_potential_gradient(p);
        double len = grad.length();
        if (len > 1e-10) return -grad / len;
        return vec3(0, 1, 0);
    }

    bool march_metaballs(const ray& r, double t_min, double t_max, hit_record& rec) const {
        double t_enter = t_max;
        double t_exit = t_min;
        for (const auto& blob : blobs) {
            double t0, t1;
            if (blob->get_ray_bounds(r, t0, t1)) {
                t0 = std::max(t0, t_min);
                t1 = std::min(t1, t_max);
                if (t0 < t1) {
                    t_enter = std::min(t_enter, t0);
                    t_exit = std::max(t_exit, t1);
                }
            }
        }

        if (t_enter >= t_exit)
            return false;

        double dir_len = r.direction().length();
        double t_step = MARCH_STEP / dir_len;

        bool inside = (total_potential(r.at(t_enter)) > POTENTIAL_THRESHOLD);
        if (inside) {
            rec.t = t_enter;
            rec.p = r.at(t_enter);
            rec.normal = isosurface_normal(rec.p);
            rec.mat = dominant_blob(rec.p)->mat;
            return true;
        }
        double prev_t = t_enter;

        for (double t = t_enter + t_step; t < t_exit; t += t_step) {
            bool now_inside = (total_potential(r.at(t)) > POTENTIAL_THRESHOLD);

            if (now_inside) {
                double lo = prev_t, hi = t;
                for (int b = 0; b < 8; b++) {
                    double mid = (lo + hi) * 0.5;
                    if (total_potential(r.at(mid)) > POTENTIAL_THRESHOLD)
                        hi = mid;
                    else
                        lo = mid;
                }

                rec.t = hi;
                rec.p = r.at(hi);
                rec.normal = isosurface_normal(rec.p);
                rec.mat = dominant_blob(rec.p)->mat;
                return true;
            }

            inside = now_inside;
            prev_t = t;
        }

        return false;
    }
};

#endif
