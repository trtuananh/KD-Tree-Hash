#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <ctime>
#include <float.h>
#include <iomanip>
#include <sstream>
#include <limits>
#include "point&plane.h"
#include "KDTree.h"
#include "LSHash.h"

using namespace std;

//////////////// class for KD TREE

/////////////// LOCALITY SENSITIVE HASH


template<class T>
T getInput(T min, T max, const string& des)
{
    T n;
    while (true) {
        cout << "- Enter " << des << ": ";
        cin >> n;
        if (cin.fail())
        {
            cin.clear(); // put us back in 'normal' operation mode
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "- That input is invalid.  Please try again.\n";
        }
        else if (n<min || n>max) {
            cerr << "- Input out of range. Please try again.\n";
        }
        else
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }
    return n;
}

int main()
{
    int n = getInput(1, INT_MAX, "the number of points");
    cerr << "setting ...\n";
    vector<Point3D> database(n);
    mt19937 rng(static_cast<int>(time(nullptr)));
    uniform_int_distribution<int> dis(0, 100000);
    for (int i=0; i<n; i++) {
        database[i] = Point3D(dis(rng)/1000.0, dis(rng)/1000.0, dis(rng)/1000.0);
    }
    KDTree tree;
    LSH hashtable(n);
    for (const Point3D& x : database) {
        tree.insert(x);
        hashtable.insert(x);
    }
    while (true) {
        cout << "/////////////////////////////\n";
        cout << "- Select Options:\n";
        cout << setw(5) << 1 << ": SEARCH FOR NEAREST POINT\n";
        cout << setw(5) << 2 << ": SEARCH FOR ANY POINT IN A INPUT DISTANCE\n";
        cout << setw(5) << 3 << ": PRINT K-D TREE\n";
        cout << setw(5) << 4 << ": PRINT HASH TABLE\n";
        cout << setw(5) << 5 << ": EXIT\n";
        int opt = getInput(1, 5, "option");
        if (opt==1) {
            cout << "- DOING: SEARCH FOR NEAREST POINT\n";
            float x = round(getInput(0.0f, 100.0f, "x value")*1000.0)/1000.0;
            float y = round(getInput(0.0f, 100.0f, "y value")*1000.0)/1000.0;
            float z = round(getInput(0.0f, 100.0f, "z value")*1000.0)/1000.0;
            Point3D np(x, y, z);

            Point3D tp = tree.nearestPoint(np);
            cout << "+ Nearest Point found by K-D Tree: " << tp << endl;

            Point3D hp = hashtable.nearestPoint(np);
            cout << "+ Nearest Point found by LS HASH: " << hp << endl;
        }
        else if (opt==2) {
            cout << "- DOING: SEARCH FOR ANY POINT IN A INPUT DISTANCE\n";
            float x = round(getInput(0.0f, 100.0f, "x value")*1000.0)/1000.0;
            float y = round(getInput(0.0f, 100.0f, "y value")*1000.0)/1000.0;
            float z = round(getInput(0.0f, 100.0f, "z value")*1000.0)/1000.0;
            float maxDis = getInput(0.0f, 100.0f, "maximum distance");
            Point3D np(x, y, z);

            vector<Point3D> tp = tree.closePoint(np, maxDis);
            cout << "+ Vector Points found by K-D Tree:   ";
            if (tp.size()==0) cout << "NULL\n";
            else {
                cout << tp[0] << endl;
                for (size_t i=1; i<tp.size(); i++)
                    cout << "                                  -> " << tp[i] << endl;
            }

            vector<Point3D> hp = hashtable.closePoint(np, maxDis);
            cout << "+ Vector Points found by LS HASH:   ";
            if (hp.size()==0) cout << "NULL\n";
            else {
                cout << hp[0] << endl;
                for (size_t i=1; i<hp.size(); i++)
                    cout << "                                 -> " << hp[i] << endl;
            }
        }
        else if (opt==3) {
            cout << "- DOING: PRINT TREE\n";
            tree.printTree();
        }
        else if (opt==4) {
            cout << "- DOING: PRINT HASH TABLE\n";
            int itab = getInput(0, 19, "an index of subtable(default 0->19)");
            hashtable.print(itab);
        }
        else break;
        system("pause");
    }
    return 0;
}
