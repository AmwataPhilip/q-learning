/*
*
* Author: Philip Amwata
* Version: 1.0
*
*/
#ifndef PCA_HPP
#define PCA_HPP

#include <cstdio>
#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include "Eigen/Core"
#include "Eigen/LU"
#include "Eigen/Cholesky"
#include "Eigen/QR"
#include "Eigen/SVD"
#include "Eigen/Geometry"
#include "Eigen/Eigenvalues"

namespace AMWPHI001
{
using namespace std;
using namespace Eigen;

class Analyser
{
private:
public:
    Analyser();
    ~Analyser();
    MatrixX2f readFile(string fileName);
    void eigenSolve(MatrixX2f initialMatrix);
    vector<float> computeCovarianceMatrix(vector<float> january, vector<float> july);
    void startPCS(string fname, Analyser pca);
};
}; // namespace AMWPHI001

#endif // !PCA_HPP
