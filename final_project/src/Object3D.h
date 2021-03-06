#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "Ray.h"
#include "Material.h"
#include <iostream>

#include <string>

using namespace std;

// FINAL PROJECT
class BoundingBox
{

public:
    Vector3f min;
    Vector3f max;

    BoundingBox(const Vector3f min, const Vector3f max) : min(min), max(max){};
    BoundingBox(){};

    Vector3f minBounds() const
    {
        return min;
    }
    Vector3f maxBounds() const
    {
        return max;
    }

    Vector3f bounds(int type) const
    {
        if (type == 0)
            return min;
        return max;
    }

    // dx/y/z functions are all correct

    float dx() const
    {
        return abs(max.x() - min.x());
    };
    float dy() const
    {
        return abs(max.y() - min.y());
    };
    float dz() const
    {
        return abs(max.z() - min.z());
    };

    float d(int axis) const
    {
        return max[axis] - min[axis];
    }

    float isPlanar()
    {
        return dx() <= 0.01 || dy() <= 0.01 || dz() <= 0.01;
    }

    // Code taken mostly from
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/
    // minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.
    vector<float> intersect(const Ray &r)
    {
        float tmin, tmax, tymin, tymax, tzmin, tzmax;

        tmin = (bounds(r.sign[0]).x() - r.orig.x()) * r.invdir.x();
        tmax = (bounds(1 - r.sign[0]).x() - r.orig.x()) * r.invdir.x();
        tymin = (bounds(r.sign[1]).y() - r.orig.y()) * r.invdir.y();
        tymax = (bounds(1 - r.sign[1]).y() - r.orig.y()) * r.invdir.y();

        if ((tmin > tymax) || (tymin > tmax))
            return vector<float>{};
        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        tzmin = (bounds(r.sign[2]).z() - r.orig.z()) * r.invdir.z();
        tzmax = (bounds(1 - r.sign[2]).z() - r.orig.z()) * r.invdir.z();

        if ((tmin > tzmax) || (tzmin > tmax))
            return vector<float>{};
        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;
        return vector<float>{tmin, tmax};
    }
};

class Object3D
{
public:
    Object3D()
    {
        material = NULL;
    }

    virtual ~Object3D() {}

    Object3D(Material *material)
    {
        this->material = material;
    }

    std::string getType() const
    {
        return type;
    }
    Material *getMaterial() const
    {
        return material;
    }

    virtual bool intersect(const Ray &r, float tmin, Hit &h) const = 0;
    void fixBBox(const Matrix4f &m);

    std::string type;
    Material *material;
    bool isTriangle = false;
    bool isMesh = false;
    BoundingBox box;
};

class Sphere : public Object3D
{
public:
    // default contstructor: unit ball at origin
    Sphere()
    {
        _center = Vector3f(0.0, 0.0, 0.0);
        _radius = 1.0f;
    }

    Sphere(const Vector3f &center,
           float radius,
           Material *material) : Object3D(material),
                                 _center(center),
                                 _radius(radius)
    {
    }

    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;

private:
    Vector3f _center;
    float _radius;
};

// Add more fields as necessary, but do not remove getVertex and getNormal
// as they are currently called by the Octree for optimization
class Triangle : public Object3D
{
public:
    Triangle(const Vector3f &a,
             const Vector3f &b,
             const Vector3f &c,
             const Vector3f &na,
             const Vector3f &nb,
             const Vector3f &nc,
             Material *m) : Object3D(m)
    {

        isTriangle = true;

        _v[0] = a;
        _v[1] = b;
        _v[2] = c;
        _normals[0] = na;
        _normals[1] = nb;
        _normals[2] = nc;
        material = m;

        // FINAL PROJECT
        Vector3f minBounds, maxBounds;

        minBounds.x() = min(a.x(), min(b.x(), c.x()));
        minBounds.y() = min(a.y(), min(b.y(), c.y()));
        minBounds.z() = min(a.z(), min(b.z(), c.z()));

        maxBounds.x() = max(a.x(), max(b.x(), c.x()));
        maxBounds.y() = max(a.y(), max(b.y(), c.y()));
        maxBounds.z() = max(a.z(), max(b.z(), c.z()));
        box = BoundingBox(minBounds, maxBounds);

        // Calculate the centroid to sort when building
        // the KD tree.
        centroid = 0.33f * Vector3f(
                                        a.x() + b.x() + c.x(),
                                        a.y() + b.y() + c.y(),
                                        a.z() + b.z() + c.z());
        centroidX = centroid[0];
        centroidY = centroid[1];
        centroidZ = centroid[2];
    }

    virtual bool intersect(const Ray &ray, float tmin, Hit &hit) const override;

    bool intersect(const Ray &ray, float tmin, Hit &hit, float tstart, float tend) const;

    const Vector3f &getVertex(int index) const
    {
        assert(index < 3);
        return _v[index];
    }

    const Vector3f &getNormal(int index) const
    {
        assert(index < 3);
        return _normals[index];
    }

    Vector3f centroid;
    float centroidX, centroidY, centroidZ;

private:
    Vector3f _v[3];
    Vector3f _normals[3];
    Material *material;
};

class Group : public Object3D
{
public:
    // Return true if intersection found
    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;

    // Add object to group
    void addObject(Object3D *obj);

    // Return number of objects in group
    int getGroupSize() const;

private:
    std::vector<Object3D *> m_members;
};

// TODO: Implement Plane representing an infinite plane
// Choose your representation, add more fields and fill in the functions
class Plane : public Object3D
{
public:
    Plane(const Vector3f &normal, float d, Material *m);

    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;

private:
    float _d;
    Vector3f _normal;
    Material *_m;
};

// TODO implement this class
// So that the intersect function first transforms the ray
// Add more fields as necessary
class Transform : public Object3D
{
public:
    Transform(const Matrix4f &m, Object3D *obj);

    virtual bool intersect(const Ray &r, float tmin, Hit &h) const override;

private:
    Object3D *_object; //un-transformed object
    Matrix4f M;
};

#endif
