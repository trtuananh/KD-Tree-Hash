#ifndef KDTREE_H
#define KDTREE_H

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

class Node
{
    friend class KDTree;
    const Point3D* data;
    Node* left = nullptr;
    Node* right = nullptr;
public:
    Node(const Point3D& data, Node* left = nullptr, Node* right = nullptr) : left(left), right(right)
    {
        this->data = &data;
    }
};

class KDTree
{
    Node* root = nullptr;
    const int k = 3; // k is the number of dimension
public:
    /////// clear tree
    void clear(Node* node)
    {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

    ~KDTree(){clear(root);}

    ////// insert node
    Node* insertRec(Node* node, const Point3D& ndata, int depth)
    {
        if (!node) return new Node(ndata);
        int d = depth%k;
        if (ndata[d] < (*node->data)[d]) node->left = insertRec(node->left, ndata, depth + 1);
        else node->right = insertRec(node->right, ndata, depth + 1);
        return node;
    }

    void insert(const Point3D& ndata)
    {
        this->root = insertRec(root, ndata, 0);
    }

    ////// exactly search, always O(logN)
    bool searchRec(Node* node, const Point3D& key, int depth)
    {
        if (!node) return 0;
        else if ((*node->data) == key) return 1;
        int d = depth%k;
        if (key[d] < (*node->data)[d]) return searchRec(node->left, key, depth + 1);
        else return searchRec(node->right, key, depth + 1);
    }

    bool search(const Point3D& key)
    {
        return searchRec(root, key, 0);
    }

    ////// remove Node from Tree
    Node* findMinRec(Node* node, int d, int depth)
    {
        if (!node) return nullptr;
        int cd = depth%k;
        if (cd == d) {
            if (node->left) return findMinRec(node->left, d, depth + 1);
            else return nullptr;
        }
        Node* minl = findMinRec(node->left, d, depth + 1);
        Node* minr = findMinRec(node->right, d, depth + 1);
        Node* rnode = node;
        if (minl && (*minl->data)[d] < (*rnode->data)[d]) rnode = minl;
        if (minr && (*minr->data)[d] < (*rnode->data)[d]) rnode = minr;
        return rnode;
    }

    Node* removeRec(Node* node, const Point3D& key, int depth)
    {
        if (!node) return nullptr;
        int d = depth%k;
        if ((*node->data) == key) {
            if (node->right) {
                const Point3D* minr = findMinRec(node->right, d, depth + 1)->data;
                node->data = minr;
                node->right = removeRec(node->right, *minr, depth + 1);
            }
            else if (node->left) {
                const Point3D* minl = findMinRec(node->left, d, depth + 1)->data;
                node->data = minl;
                node->right = removeRec(node->left, *minl, depth + 1);
                node->left = nullptr;
            }
            else {
                delete node;
                return nullptr;
            }
        }
        else {
            if (key[d] < (*node->data)[d]) node->left = removeRec(node->left, key, depth + 1);
            else node->right =  removeRec(node->right, key, depth + 1);
        }
        return node;
    }

    void remove(const Point3D& key)
    {
        root = removeRec(root, key, 0);
    }

    int getHeightRec(Node* node)
    {
        if (!node) return 0;
        else {
            int lh = getHeightRec(node->left), rh = getHeightRec(node->right);
            return ((lh>rh)?lh:rh) + 1;
        }
    }

    int getHeight()
    {
        return getHeightRec(root);
    }

    int getSizeRec(Node* node)
    {
        if (!node) return 0;
        else return 1 + getSizeRec(node->left) + getSizeRec(node->right);
    }

    int getSize() {return getSizeRec(root);}

    void printRec(Node* node, int level)
    {
        cout << char('x' + level%k) << ": ";
        for (int i=0; i<level; i++) cout << "     ";
        cout << "[--->";
        if (node) {
            cout << (*node->data) << endl;
            if (node->right) {
                printRec(node->left, level + 1);
                printRec(node->right, level + 1);
            }
            else if (node->left)
                printRec(node->left, level + 1);
        }
        else cout << endl;
    }

    //////////// find Point in an optimal distance
    void closePointRec(vector<Point3D>& arr, float maxDis, Node* node, const Point3D& key, int depth)
    {
        if (!node) return;
        int d = depth%k;
        // if the distance between node and key is smaller than an arguement distance, inseer node into the return vector
        if (key.squareDistance((*node->data)) <= maxDis*maxDis)
            arr.push_back((*node->data));
        if (key[d] < (*node->data)[d]) {
            closePointRec(arr, maxDis, node->left, key, depth + 1);
            // if the distance between key and the divided plane is smaller than an arguement, must check the other side
            if (abs(key[d] - (*node->data)[d]) < maxDis)
                closePointRec(arr, maxDis, node->right, key, depth + 1);
        }
        else {
            closePointRec(arr, maxDis, node->right, key, depth + 1);
            // if the distance between key and the divided plane is smaller than an arguement, must check the other side
            if (abs(key[d] - (*node->data)[d]) < maxDis)
                closePointRec(arr, maxDis, node->left, key, depth + 1);
        }
    }

    vector<Point3D> closePoint(const Point3D& key, float maxDis = 0)
    {
        vector<Point3D> arr;
        closePointRec(arr, maxDis, root, key, 0);
        return arr;
    }

    /////////// find the nearest Point
    Point3D nearestPointRec(Node* node, const Point3D& key, int depth)
    {
        if (!node) throw "empty tree";
        int d = depth%k;
        Point3D nearPoint = (*node->data);
        if (key[d] < (*node->data)[d]) {
            if (node->left) {
                // find the nearest point in the left subtree
                Point3D nearPointLeft = nearestPointRec(node->left, key, depth + 1);
                // compare which is smaller, the root point and the left subtree point
                if (key.squareDistance(nearPointLeft) < key.squareDistance(nearPoint))
                    nearPoint = nearPointLeft;
            }
            // if the distance between key and the divided plane is smaller, find the nearest point in the right subtree
            if (node->right && (key[d] - (*node->data)[d])*(key[d] - (*node->data)[d]) < key.squareDistance(nearPoint)) {
                Point3D nearPointRight = nearestPointRec(node->right, key, depth + 1);
                if (key.squareDistance(nearPointRight) < key.squareDistance(nearPoint))
                    nearPoint = nearPointRight;
            }
        }
        else {
            if (node->right) {
                // find the nearest point in the right subtree
                Point3D nearPointRight = nearestPointRec(node->right, key, depth + 1);
                // compare which is smaller, the root point and the right subtree point
                if (key.squareDistance(nearPointRight) < key.squareDistance(nearPoint))
                    nearPoint = nearPointRight;
            }
            // if the distance between key and the divided plane is smaller, find the nearest point in the left subtree
            if (node->left && (key[d] - (*node->data)[d])*(key[d] - (*node->data)[d]) < key.squareDistance(nearPoint)) {
                Point3D nearPointLeft = nearestPointRec(node->left, key, depth + 1);
                if (key.squareDistance(nearPointLeft) < key.squareDistance(nearPoint))
                    nearPoint = nearPointLeft;
            }
        }
        return nearPoint;
    }

    Point3D nearestPoint(const Point3D& key)
    {
        return nearestPointRec(root, key, 0);
    }

    /////////// print tree
    void printTree()
    {
        cout << "/////////////////KD TREE//////////////////\n";
        printRec(root, 0);
        cout << "//////////////////////////////////////////\n";
    }
};

#endif // KDTREE_H
