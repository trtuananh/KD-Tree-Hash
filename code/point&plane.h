#ifndef POINTPLANE_H
#define POINTPLANE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <ctime>
#include <float.h>
#include <iomanip>
#include <sstream>
#include <limits>

using namespace std;

class Node;
class KDTree;

class Point3D
{
    friend class Node;
    friend class KDTree;
    float x, y, z;
public:
    Point3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    float& operator[](int n) ////// access x, y, z with index 0, 1, 2
    {
        if (n==0) return x;
        else if (n==1) return y;
        else if (n==2) return z;
        else throw "out of index\n";
    }
    const float& operator[](int n) const
    {
        if (n==0) return x;
        else if (n==1) return y;
        else if (n==2) return z;
        else throw "out of index\n";
    }
    bool operator==(const Point3D& p) const ////// compare 2 points
    {
        return x==p.x && y==p.y && z==p.z;
    }
    friend ostream& operator<<(ostream& out, const Point3D& p) ////// print Point
    {
        ostringstream sout;
        sout << "(" << p.x << "," << p.y << "," << p.z << ")";
        out << sout.str();
        return out;
    }
    float squareDistance(const Point3D& p) const ////// the Euclidean distance between two points without the square root
    {
        return (x - p.x)*(x - p.x) + (y - p.y)*(y - p.y) + (z - p.z)*(z - p.z);
    }
};

///////////////// class Plane
class CutPlane
{
    int d;
    vector<int> plane;
public:
    CutPlane(const vector<int>& nplane, int d = 3)
    {
        this->d = d;
        plane = nplane;
        if (int(plane.size()) < d+1) {
            for (int i=plane.size(); i<d+1; i++) plane.push_back(0);
        }
        else if (plane.size() > size_t(d+1)) plane.resize(d+1);
    }

    float getValue(const Point3D& key) const ////////// get the value when x, y, z are replaced by key.x, key.y, key.z
    {
        float exp = plane[d];
        for (int i=0; i<d; i++)
            exp += key[i]*plane[i];
        return exp;
    }

    float squareDistance(const Point3D& key) const ////// the Euclidean distance between a point and this plane without the square root
    {
        float sqabs = plane[d];
        for (int i=0; i<d; i++)
            sqabs += key[i]*plane[i];
        sqabs *= sqabs;
        float sqvec = 0;
        for (int i=0; i<d; i++)
            sqvec += plane[i]*plane[i];
        return sqabs/sqvec;
    }

    void print()
    {
        for (size_t i=0; i<plane.size() - 1; i++) {
            cout << plane[i] << char('x' + i) << " + ";
        }
        cout << plane[plane.size() - 1] << " = 0\n";
    }
};

#endif
