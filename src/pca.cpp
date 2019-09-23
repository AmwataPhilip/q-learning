/*
*
* Author: Philip Amwata
* Version: 1.0
*
*/

#include "../include/pca.hpp"

using namespace AMWPHI001;

vector<float> Analyser::computeCovarianceMatrix(vector<float> januaryData, vector<float> julyData)
{
    // Calculate mean of januaryData vector and store in mean1
    float mean1 = accumulate(januaryData.begin(), januaryData.end(), 0.0f) / float(januaryData.size());
    // Calculate mean of julyData vector and store in mean2
    float mean2 = accumulate(julyData.begin(), julyData.end(), 0.0f) / float(julyData.size());

    // Variables to store covariances
    float cov1 = 0.0f, cov2 = 0.0f, cov3 = 0.0f;

    // Center vector values and calculate covariances
    for (int n = 0; n < januaryData.size(); n++)
    {
        januaryData[n] -= mean1;
        julyData[n] -= mean2;
        cov1 += januaryData[n] * januaryData[n];
        cov2 += januaryData[n] * julyData[n];
        cov3 += julyData[n] * julyData[n];
    }
    cov1 /= float(januaryData.size() - 1);
    cov2 /= float(januaryData.size() - 1);
    cov3 /= float(januaryData.size() - 1);

    // Store computed covariances in covValues vector
    vector<float> covValues{cov1, cov2, cov3};

    return covValues;
}

MatrixX2f Analyser::readFile(string fileName)
{
    string relativePath = "src/" + fileName;
    std::cout << "Reading File: " << fileName << std::endl;
    std::cout << "Relative Path: " << relativePath << std::endl
              << std::endl;
    ;

    ifstream infile; // File Pointer

    // Open an existing file
    infile.open(relativePath, ios::in);

    vector<string> row, cols;            // Row stores lines and cols stores each cell
    vector<float> januaryData, julyData; // Store the data for each month
    vector<float> covVector;             // Temporarily stores covariance and variance values
    Matrix2f covMatrix;                  // Store covariances and variance
    string line, word;                   // Used to read lines and data from file

    while (!infile.eof())
    {
        // Store all data before \n in line variable
        getline(infile, line);
        // Read an entire row and store in a string line
        row.push_back(line);
    }
    row.pop_back();
    for (auto &x : row)
    {
        stringstream s(x); // Used to split words in a line
        // Read every column data of a row and store in a string word
        while (getline(s, word, ';'))
        {
            // Add all column data of a row to the vector
            cols.push_back(word);
        }
    }

    // Store each variables observations in a vectore for use in covariance calculations
    for (int c = 0; c < cols.size(); c++)
    {
        if (c % 2 == 0)
        {
            // Add varibale one data ro januaryData
            januaryData.push_back(stof(cols[c]));
        }
        else
        {
            // Add variable two to julyData
            julyData.push_back(stof(cols[c]));
        }
    }
    // Store computed covariance matrix data in covVector
    covVector = computeCovarianceMatrix(januaryData, julyData);
    // Move computed covariances to covariance matrix covMatrix
    covMatrix << covVector[0], covVector[1], covVector[1], covVector[2];

    // Output covariance matrx to console
    std::cout << "Covariance Matrix: " << std::endl
              << covMatrix << std::endl
              << std::endl;

    return covMatrix;
}

void Analyser::eigenSolve(MatrixX2f initialMatrix)
{
    EigenSolver<MatrixXf> eigenSolver(initialMatrix);
    if (eigenSolver.info() != Success)
        abort();
    // Get individual eigen values
    float eigen1, eigen2;
    eigen1 = (eigenSolver.eigenvalues().col(0)[0]).real();
    eigen2 = (eigenSolver.eigenvalues().col(0)[1]).real();

    // Calculate ratio of eigenvalues
    float pc1 = (eigen1 / (eigen1 + eigen2)) * 100.0f;
    float pc2 = (eigen2 / (eigen1 + eigen2)) * 100.0f;

    // Output Eigenvalues to console
    std::cout << "Eigenvalues: " << std::endl
              << "PC 1: " << eigen1 << std::endl
              << "PC 2: " << eigen2 << std::endl
              << std::endl;

    // Output Eiegenvectors to console
    std::cout << "Eigenvectors: " << std::endl
              << "     PC 1        PC 2" << std::endl
              << eigenSolver.eigenvectors() << std::endl
              << std::endl;
    // OUtput total variance to console
    std::cout << "Total Variance: " << std::endl
              << eigen1 + eigen2 << std::endl
              << std::endl;

    // Output Variance Proportions to console
    std::cout << "Variance Proportions: " << std::endl
              << "PC1: " << pc1 << "%" << std::endl
              << "PC2: " << pc2 << "%" << std::endl
              << std::endl;
}

void Analyser::startPCS(string fname, Analyser pca)
{
    try
    {
        std::cout << "Starting PCA Analysis." << std::endl;
        std::cout << "--------------------------------" << std::endl;
        MatrixX2f initMat = pca.readFile(fname);
        pca.eigenSolve(initMat);
        std::cout << "--------------------------------" << std::endl;
        std::cout << "PCA analysis complete." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

Analyser::Analyser()
{
}

Analyser::~Analyser()
{
}
