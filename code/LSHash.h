#ifndef LSHASH_H
#define LSHASH_H

#include "point&plane.h"
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

class LSH
{
    static const int D = 3, L = 20; // D is the number of dimensions, L is the number of hash table
    int k, bot, top; // n is the number of points, k is the number of cut planes
    size_t capacity, n = 0;
    vector<vector<CutPlane>> ktab;
    vector<vector<vector<const Point3D*>>> hashtab;
public:
    LSH(size_t N, int bot = 0, int top = 100) : bot(bot), top(top)
    {
        this->k = log2(N); // k = log2(N) for the best performance
        this->capacity = pow(2, k);
        // random create k planes for L hash tables
        mt19937 rng(static_cast<int>(time(nullptr)));
        uniform_int_distribution<int> dis(-top, top);
        ktab.resize(L);
        hashtab.resize(L);
        for (int i=0; i<L; i++) {
            hashtab[i].resize(this->capacity);
            for (int j=0; j<k; j++) {
                vector<int> plane(D+1);
                // make sure created planes is not out of space
                while (true) {
                    for (int& x : plane) x = dis(rng);
                    int range[2] = {bot, top};
                    int numcor = 1 << D;
                    vector<int> exp(numcor, plane[D]);
                    for (int t=0; t<numcor; t++) {
                        for (int u=0; u<D; u++) {
                            exp[t] += plane[u]*range[(t%(numcor >> u))/(numcor >> (u+1))];
                        }
                        exp[t] *= plane[0];
                    }
                    bool b1 = 0, b2 = 0;
                    for (int t=0; t<numcor/2; t++) {
                        if (exp[t]>=0) {
                            b1 = 1;
                            break;
                        }
                    }
                    for (int t=numcor/2; t<numcor && b1; t++) {
                        if (exp[t]<=0) {
                            b2 = 1;
                            break;
                        }
                    }
                    if (b1 && b2) break;
                }
                ktab[i].push_back(CutPlane(plane));
            }
        }
    }

    //////////////// hash function
    size_t hashing(const Point3D& key, int itab)
    {
        size_t index = 0;
        for (size_t i=0; i<size_t(k); i++) {
            size_t bitmask = 1 << i;
            if (ktab[itab][i].getValue(key) < 0) continue;
            else index = index | bitmask;
        }
        return index;
    }

    //////////////// insert point
    void insert(const Point3D& key)
    {
        for (int i=0; i<L; i++){
            hashtab[i][hashing(key, i)].push_back(&key);
        }
        n++;
    }

    /////////////// remove point
    bool remove(const Point3D& key)
    {
        bool suc = 0;
        for (int i=0; i<L; i++) {
            size_t hashIndex = hashing(key, i);
            for (auto j=hashtab[i][hashIndex].begin(); j!=hashtab[i][hashIndex].end(); j++) {
                if (*(*j)==key) {
                    hashtab[i][hashIndex].erase(j);
                    suc = 1;
                    break;
                }
            }
        }
        if (suc) n--;
        return suc;
    }

    /////////////// find the nearest point
    Point3D nearestPoint(const Point3D& key)
    {

        if (n==0) throw "empty table"; // if hash tables are empty, throw exception
        Point3D minp;
        float mind = FLT_MAX;
        // guessing the nearest point by hash method
        for (int i=0; i<L; i++) {
            size_t hashIndex = hashing(key, i);
            for (const Point3D* x : hashtab[i][hashIndex]) {
                float newdis = key.squareDistance(*x);
                if (newdis < mind) {
                    mind = newdis;
                    minp = *x;
                }
            }
        }
        // backup check
        vector<int> flexIndex;
        int tabIndex = 0;
        // find the most effective hash table to check
        for (int i=0, minflex = this->k + 1; i<L; i++) {
            vector<int> flexIndexTemp;
            for (int j=0; j<this->k; j++) {
                if (ktab[0][j].squareDistance(key) < mind)
                    flexIndexTemp.push_back(j);
            }
            if (flexIndexTemp.size() < size_t(minflex)) {
                minflex = flexIndexTemp.size();
                flexIndex = flexIndexTemp;
                tabIndex = i;
            }
        }
        // backup check at the most effective hash table
        size_t numOfCase = pow(2, flexIndex.size()), hashIndex = hashing(key, tabIndex);
        for (size_t i=0; i<numOfCase; i++) {
            // change the hash index
            for (size_t j=0; j<flexIndex.size(); j++) {
                hashIndex = hashIndex & (~(1 << flexIndex[j]));
                if (i & (1 << j)) {
                    hashIndex = hashIndex | (1 << flexIndex[j]);
                }
            }
            // check in new block
            for (const Point3D* x : hashtab[tabIndex][hashIndex]) {
                float newDis = key.squareDistance(*x);
                if (newDis < mind) {
                    minp = *x;
                    mind = newDis;
                }
            }
        }
        return minp;
    }

    ///////////////// find points in the distance
    vector<Point3D> closePoint(const Point3D& key, float maxDis = 0.0)
    {
        vector<Point3D> clp;
        // guessing points in the distance by hash method
        for (int i=0; i<L; i++) {
            size_t hashIndex = hashing(key, i);
            for (const Point3D* x : hashtab[i][hashIndex]) {
                bool flag = 1;
                // check if point has been already found
                for (const Point3D& y : clp) {
                    if (y==*x) {
                        flag = 0;
                        break;
                    }
                }
                // if not exist, insert point into return vector
                if (flag && key.squareDistance(*x) <= maxDis*maxDis) {
                    clp.push_back(*x);
                }
            }
        }

        // backup check
        vector<int> flexIndex;
        int tabIndex = 0;
        // find the most effective hash table to check
        for (int i=0, minflex = this->k + 1; i<L; i++) {
            vector<int> flexIndexTemp;
            for (int j=0; j<this->k; j++) {
                if (ktab[0][j].squareDistance(key) < maxDis*maxDis)
                    flexIndexTemp.push_back(j);
            }
            if (flexIndexTemp.size() < size_t(minflex)) {
                minflex = flexIndexTemp.size();
                flexIndex = flexIndexTemp;
                tabIndex = i;
            }
        }
        // backup check at the most effective hash table
        size_t numOfCase = pow(2, flexIndex.size()), hashIndex = hashing(key, tabIndex);
        for (size_t i=0; i<numOfCase; i++) {
            // change the hash index
            for (size_t j=0; j<flexIndex.size(); j++) {
                hashIndex = hashIndex & (~(1 << flexIndex[j]));
                if (i & (1 << j)) {
                    hashIndex = hashIndex | (1 << flexIndex[j]);
                }
            }
            // check in new block
            for (const Point3D* x : hashtab[tabIndex][hashIndex]) {
                bool flag = 1;
                for (const Point3D& y : clp) {
                    if (y==*x) {
                        flag = 0;
                        break;
                    }
                }
                if (flag && key.squareDistance(*x) <= maxDis*maxDis) {
                    clp.push_back(*x);
                }
            }
        }
        return clp;
    }

    //////////// print hash table
    void print(int i = 0)
    {
        cout << "******* HASH TABLE " << i << " *******\n";
        for (size_t j=0; j<this->capacity; j++) {
            cout << setw(10) << j << ": ";
            if (hashtab[i][j].size()==0)
                cout << setw(25) << "NULL";
            else {
                cout << setw(25) << *hashtab[i][j][0];
                for (size_t k=1; k<hashtab[i][j].size(); k++)
                    cout << "->" << setw(25) << *hashtab[i][j][k];
            }
            cout << endl;
        }
        cout << "/////////////////////////\n";
    }
};


#endif // LSHASH_H
