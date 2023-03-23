#include "col_utils.hpp"
#include "types.hpp"
#include <cmath>
#include <vector>
namespace EPI_NAMESPACE {

vec2f rotateVec(vec2f vec, float angle) {
    return vec2f(cos(angle) * vec.x - sin(angle) * vec.y,
        sin(angle) * vec.x + cos(angle) * vec.y);
}
#define SQR(x) ((x) * (x))
bool isOverlappingPointAABB(const vec2f& p, const AABB& r) {
    return (p.x >= r.center().x - r.size().x / 2 && p.y > r.center().y - r.size().y / 2
        && p.x < r.center().x + r.size().x / 2 && p.y <= r.center().y + r.size().y / 2);
}
bool isOverlappingPointCircle(const vec2f& p, const Circle& c) {
    return len(p - c.pos) <= c.radius;
}
bool isOverlappingPointPoly(const vec2f& p, const Polygon& poly) {
    int i, j, c = 0;
    for (i = 0, j = poly.getVertecies().size() - 1; i < poly.getVertecies().size(); j = i++) {
        auto& vi = poly.getVertecies()[i];
        auto& vj = poly.getVertecies()[j];
        if ( ((vi.y>p.y) != (vj.y>p.y)) &&
             (p.x < (vj.x-vi.x) * (p.y-vi.y) / (vj.y-vi.y) + vi.x) )
               c = !c;
        }
    return c;
}
bool isOverlappingAABBAABB(const AABB& r1, const AABB& r2) {
    return (
        r1.min.x <= r2.max.x &&
        r1.max.x >= r2.min.x &&
        r1.min.y <= r2.max.y &&
        r1.max.y >= r2.min.y);
}
bool AABBcontainsAABB(const AABB& r1, const AABB& r2) {
    return (r2.min.x >= r1.min.x) && (r2.max.x <= r1.max.x) &&
				(r2.min.y >= r1.min.y) && (r2.max.y <= r1.max.y);
}
IntersectionRayAABBResult intersectRayAABB(vec2f ray_origin, vec2f ray_dir,
    const AABB& target)
{
    IntersectionRayAABBResult result;
    vec2f invdir = { 1.0f / ray_dir.x, 1.0f / ray_dir.y };
    vec2f t_size = target.size();
    //VVVVVVVVVVVVV
    //if((int)target.size.y % 2 == 0 && target.pos.y > ray_origin.y)
    //t_size -= vec2f(0, 1);
    //^^^^^^^^^^^^^
    vec2f t_near = (target.center() - t_size / 2.f - ray_origin) * invdir;
    vec2f t_far = (target.center() + t_size / 2.f - ray_origin) * invdir;

    if (std::isnan(t_far.y) || std::isnan(t_far.x)) return {false};
    if (std::isnan(t_near.y) || std::isnan(t_near.x)) return {false};
    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    if (t_near.x > t_far.y || t_near.y > t_far.x) return {false};
    float t_hit_near = std::max(t_near.x, t_near.y);
    result.time_hit_near = t_hit_near;
    float t_hit_far = std::min(t_far.x, t_far.y);
    result.time_hit_far = t_hit_far;

    if (t_hit_far < 0)
        return {false};
    result.contact_point = ray_origin + ray_dir * t_hit_near;
    if (t_near.x > t_near.y) {
        if (invdir.x < 0)
            result.contact_normal = { 1, 0 };
        else
            result.contact_normal = { -1, 0 };
    } else if (t_near.x < t_near.y) {
        if (invdir.y < 0)
            result.contact_normal = { 0, 1 };
        else
            result.contact_normal = { 0, -1 };
    }
    result.detected = true;
    return result;
}
IntersectionRayRayResult intersectRayRay(vec2f ray0_origin, vec2f ray0_dir,
    vec2f ray1_origin, vec2f ray1_dir)
{
    if (ray0_origin == ray1_origin) {
        return {true, ray0_origin, 0.f, 0.f};
    }
    auto dx = ray1_origin.x - ray0_origin.x;
    auto dy = ray1_origin.y - ray0_origin.y;
    auto det = ray1_dir.x * ray0_dir.y - ray1_dir.y * ray0_dir.x;
    if (det != 0) { // near parallel line will yield noisy results
        float u = (dy * ray1_dir.x - dx * ray1_dir.y) / det;
        float v = (dy * ray0_dir.x - dx * ray0_dir.y) / det;
        if (u >= 0 && v >= 0) {
            return {true, ray0_origin + ray0_dir * u, u, v};
        }
    }
    return {false};
}
vec2f findClosestPointOnRay(vec2f ray_origin, vec2f ray_dir, vec2f point) {
    float ray_dir_len = len(ray_dir);
    vec2f seg_v_unit = ray_dir / ray_dir_len;
    float proj = dot(point - ray_origin, seg_v_unit);
    if (proj <= 0)
        return ray_origin;
    if (proj >= ray_dir_len)
        return ray_origin + ray_dir;
    return seg_v_unit * proj + ray_origin;
}
vec2f findClosestPointOnEdge(vec2f point, const Polygon& poly) {
    const vec2f& pos = poly.getPos();
    auto dir = norm(point - pos);
    vec2f closest(INFINITY, INFINITY);
    for(size_t i = 0; i < poly.getVertecies().size(); i++) {
        vec2f a = poly.getVertecies()[i];
        vec2f b = poly.getVertecies()[(i + 1) % poly.getVertecies().size()];
        vec2f adir = b - a;
        auto intersection = intersectRayRay(a, adir, pos, dir);
        if(intersection.detected && intersection.t_hit_near0 < 1.f && qlen(intersection.contact_point - pos) < qlen(closest - pos) ) {
            closest = intersection.contact_point;
        }
    }
    return closest;
}
#define VERY_SMALL_AMOUNT 0.005f
bool nearlyEqual(float a, float b) {
    return abs(a - b) < VERY_SMALL_AMOUNT;
}
bool nearlyEqual(vec2f a, vec2f b) {
    return nearlyEqual(a.x, b.x) && nearlyEqual(a.y, b.y);
}
std::vector<vec2f> findContactPoints(const Polygon& p1, const Polygon& p2) {
    std::vector<vec2f> cps;
    vec2f contact1;
    vec2f contact2;
    int contactCount = 0;

    float minDistSq = INFINITY;
    auto vertsA = p1.getVertecies();
    auto vertsB = p2.getVertecies();

    for(int switcheroo = 0; switcheroo < 2; switcheroo++) {
        if(switcheroo == 1) {
            std::swap(vertsA, vertsB);
        }
        for(int i = 0; i < vertsA.size(); i++)
        {
            vec2f p = vertsA[i];

            for(int j = 0; j < vertsB.size(); j++)
            {
                vec2f va = vertsB[j];
                vec2f vb = vertsB[(j + 1) % vertsB.size()];

                auto cp = findClosestPointOnRay(va, vb - va, p);
                auto distSquared = qlen(cp - p);

                if(nearlyEqual(distSquared, minDistSq))
                {
                    if (!nearlyEqual(cp, contact1) &&
                        !nearlyEqual(cp, contact2))
                    {
                        contact2 = cp;
                        contactCount = 2;
                        if(cps.size() == 1)
                            cps.push_back(cp);
                        else
                            cps[1] = cp;
                    }
                }
                else if(distSquared < minDistSq)
                {
                    minDistSq = distSquared;
                    contactCount = 1;
                    cps = {cp};
                }
            }
        }
    }

    return cps;
}
float area(const std::vector<vec2f>& model) {
    double area = 0.0;
    // Calculate value of shoelace formula
    for (int i = 0; i < model.size(); i++) {
      int i1 = (i + 1) % model.size();
      area += (model[i].y + model[i1].y) * (model[i1].x - model[i].x) / 2.0;
    }
    return abs(area / 2.0);
}
IntersectionPolygonPolygonResult intersectPolygonPolygon(const Polygon &r1, const Polygon &r2) {
    const Polygon *poly1 = &r1;
    const Polygon *poly2 = &r2;

    float overlap = INFINITY;
    vec2f cn;
    
    for (int shape = 0; shape < 2; shape++) {
        if (shape == 1) {
            poly1 = &r2;
            poly2 = &r1;
        }
        for (int a = 0; a < poly1->getVertecies().size(); a++) {
            int b = (a + 1) % poly1->getVertecies().size();
            vec2f axisProj = { -(poly1->getVertecies()[b].y - poly1->getVertecies()[a].y), poly1->getVertecies()[b].x - poly1->getVertecies()[a].x };
            
            // Optional normalisation of projection axis enhances stability slightly
            float d = sqrtf(axisProj.x * axisProj.x + axisProj.y * axisProj.y);
            axisProj = { axisProj.x / d, axisProj.y / d };

            // Work out min and max 1D points for r1
            float min_r1 = INFINITY, max_r1 = -INFINITY;
            for (int p = 0; p < poly1->getVertecies().size(); p++) {
                float q = (poly1->getVertecies()[p].x * axisProj.x + poly1->getVertecies()[p].y * axisProj.y);
                min_r1 = std::min(min_r1, q);
                max_r1 = std::max(max_r1, q);
            }

            // Work out min and max 1D points for r2
            float min_r2 = INFINITY, max_r2 = -INFINITY;
            for (int p = 0; p < poly2->getVertecies().size(); p++) {
                float q = (poly2->getVertecies()[p].x * axisProj.x + poly2->getVertecies()[p].y * axisProj.y);
                min_r2 = std::min(min_r2, q);
                max_r2 = std::max(max_r2, q);
            }

            // Calculate actual overlap along projected axis, and store the minimum
            if(std::min(max_r1, max_r2) - std::max(min_r1, min_r2) < overlap) {
                overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
                cn = axisProj;
            }

            if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
                return {false};
        }
    }
    //correcting normal
    float d = dot(r2.getPos() - r1.getPos(), cn);
    if(d > 0.f)
        cn *= -1.f;

    return {true, cn, overlap};
}
IntersectionPolygonCircleResult intersectCirclePolygon(const Circle &c, const Polygon &r) {
    vec2f max_reach = c.pos + norm(r.getPos() - c.pos) * c.radius;

    vec2f cn;
    vec2f closest(INFINITY, INFINITY);
    vec2f prev = r.getVertecies().back();
    for(const auto& p : r.getVertecies()) {
        vec2f tmp = findClosestPointOnRay(prev, p - prev, c.pos);
        if(qlen(closest - c.pos) > qlen(tmp - c.pos)) {
            closest = tmp;
        }
        prev = p;
    }
    bool isOverlapping = len(closest - c.pos) <= c.radius || isOverlappingPointPoly(c.pos, r);
    if(!isOverlapping)
        return {false};
    cn = norm(c.pos - closest);
    //correcting normal
    if(dot(cn, r.getPos() - closest) > 0.f) {
        cn *= -1.f;
    }
    float overlap = c.radius - len(c.pos - closest);
    return {true, cn, closest, overlap};
}
IntersectionCircleCircleResult intersectCircleCircle(const Circle &c1, const Circle &c2) {
    vec2f dist = c1.pos - c2.pos;
    float dist_len = len(dist);
    if(dist_len > c1.radius + c2.radius) {
        return {false};
    }
    float overlap = c1.radius + c2.radius - dist_len;
    vec2f contact_point =  dist / dist_len * c2.radius + c2.pos;
    return {true, dist / dist_len, contact_point, overlap};
}
}
