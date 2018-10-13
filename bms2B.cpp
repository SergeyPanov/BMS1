/* 
 * File:   bms1B.cpp
 */

#include <cstdlib>
#include <math.h>
#include <iostream>

#include "sndfile.hh"


using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    SndfileHandle inputFile;
    int sampleRate;
    int *buffer;
    
    inputFile = SndfileHandle("../input.wav");
    
    sampleRate = inputFile.samplerate();
    
    buffer = new int[sampleRate];

    inputFile.read(buffer, sampleRate);

    for (int i = 0; i < sampleRate; ++i) {
        cout << buffer[i] << " ";
    }
    
    
    delete [] buffer;
    return EXIT_SUCCESS;
}

