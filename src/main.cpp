/*
*
* Author: Philip Amwata
* Version: 1.0
*
*/
#include "../include/pca.hpp"

int main(int argc, const char **argv)
{
    AMWPHI001::Analyser pca;
    pca.startPCS(argv[1], pca);
    return 0;
}
