/* 
 * File:   bms1B.cpp
 */

#include <cstdlib>
#include <math.h>
#include <iostream>
#include <vector>
#include "sndfile.hh"
#include <fstream>
#include <algorithm>

#define EXTENSION ".txt"
#define SYNCHRONIZE_SEQUENCE_LENGTH 8

using namespace std;

/*
 * Structure holds amplitudes for each baud(symbol).
 * amplitude_00 -> 00
 * amplitude_01 -> 01
 * amplitude_10 -> 10
 * amplitude_11 -> 11
 */
typedef struct AmplitudeForBaud{
    int amplitude_00;
    int amplitude_01;
    int amplitude_10;
    int amplitude_11;
} AmplitudeForBaud;



/**
 * Look for amplitude. If found amplitude is contained in @ignore_list, amplitude is ignored.
 * @param bauds vector of bauds
 * @param ignore_list amplitudes will be ignored
 * @return amplitude
 */
int get_amplitude_with_ignore_list(const vector<vector<int> > &bauds, vector<int> &ignore_list){
    int amplitude = -1;
    for(const vector<int> &baud: bauds){
        int local_max = *max_element(baud.begin(), baud.end());

        if ( ( std::find(ignore_list.begin(), ignore_list.end(),  local_max) == ignore_list.end() )
             && (amplitude == -1 || local_max > amplitude) ){
            amplitude = local_max;
        }
    }
    return amplitude;
}

/**
 * Find amplitudes for all symbols and store it into "AmplitudeForBaud" structure.
 * @param bauds
 * @return
 */
AmplitudeForBaud get_amplitudes(const vector<vector<int> > &bauds){
    AmplitudeForBaud amplitudes_for_bauds{};    // Here is gonna be stored all amplitudes.
    vector<int> ignore_list;    // There amplitudes are gonna be ignore.

    amplitudes_for_bauds.amplitude_00 = 0;  // Symbol 00 is coded with 0 amplitude.
    ignore_list.push_back(amplitudes_for_bauds.amplitude_00);

    amplitudes_for_bauds.amplitude_11 = get_amplitude_with_ignore_list(bauds, ignore_list); // Symbol 11 is coded with max amplitude.
    ignore_list.push_back(amplitudes_for_bauds.amplitude_11);

    int tmp_ampl_1 = get_amplitude_with_ignore_list(bauds, ignore_list);
    ignore_list.push_back(tmp_ampl_1);

    int tmp_ampl_2 = get_amplitude_with_ignore_list(bauds, ignore_list);

    // tmp_ampl_1 and tmp_ampl_2 are amplitudes for 01 and 10.
    if (tmp_ampl_1 > tmp_ampl_2){
        amplitudes_for_bauds.amplitude_10 = tmp_ampl_1;
        amplitudes_for_bauds.amplitude_01 = tmp_ampl_2;
    }else{
        amplitudes_for_bauds.amplitude_10 = tmp_ampl_2;
        amplitudes_for_bauds.amplitude_01 = tmp_ampl_1;
    }
    return amplitudes_for_bauds;
}

/*
 * Calculate length of single baud.
 */
int get_baud_length(const int* buffer){
    int baud_length = 0;
    int i = 0;
    while (buffer[i] == 0) {
        baud_length++;
        i++;
    }

    return baud_length;
}

/*
 * Chunk input wav by symbols.
 */
vector<vector<int> > get_bauds(int *buffer, int frames){
    int baud_length = get_baud_length(buffer);
    vector<vector<int> > chunked_buffer;
    int total_bauds = frames / baud_length;

    for (int i = 0; i < frames; i+=baud_length) {
        vector<int> chunk;

        for (int j = i; j < baud_length + i; ++j) {    // Create single chunk(baud)
            chunk.push_back(buffer[j]);
        }
        chunked_buffer.push_back(chunk);

    }
    return chunked_buffer;
}

/**
 * Demodulate bauds.
 * @param bauds modulated bauds
 * @param amplitudes    amplitudes for particular symbols
 * @return
 */
vector<int> demodulate(const vector<vector<int> > &bauds, const AmplitudeForBaud& amplitudes){

    vector<int> demodulated_sequence;
    for (const vector<int> &baud : bauds){
        int ampl = *max_element(baud.begin(), baud.end());

        if (ampl == amplitudes.amplitude_00){
            demodulated_sequence.push_back(0);
            demodulated_sequence.push_back(0);

        }else if(ampl == amplitudes.amplitude_01){
            demodulated_sequence.push_back(0);
            demodulated_sequence.push_back(1);

        }else if(ampl == amplitudes.amplitude_10){
            demodulated_sequence.push_back(1);
            demodulated_sequence.push_back(0);

        }else{
            demodulated_sequence.push_back(1);
            demodulated_sequence.push_back(1);
        }
    }
    return demodulated_sequence;

}

/**
 * Cast vector of ints to single string.
 * @param sequence of int
 * @return
 */
string vector_of_ints_to_string(vector<int> sequence){
    string str;
    for(int el : sequence){
        str += std::to_string(el);
    }
    return str;
}

/**
 * Write demodulated signal into txt file.
 */
void write_to_file(vector<int> demodulated, string path){
    std::reverse(path.begin(), path.end());

    int ind = static_cast<int>(path.find('.'));

    string outpath = path.substr(static_cast<unsigned long>(ind + 1));

    std::reverse(outpath.begin(), outpath.end());

    outpath += EXTENSION;

    vector<int> cuted(demodulated.size() - SYNCHRONIZE_SEQUENCE_LENGTH);

    std::copy(demodulated.begin() + SYNCHRONIZE_SEQUENCE_LENGTH, demodulated.end(), cuted.begin());

    string out = vector_of_ints_to_string(cuted);
    ofstream outfile(outpath);

    if (!outfile.is_open()){
        cerr << "Can't open output file" << endl;
        exit(EXIT_FAILURE);
    }

    outfile.write(out.c_str(), out.size());
    outfile.close();
}

/*
 * 
 */
int main(int argc, char** argv) {

    if (argc != 2){
        cerr << "Invalid input parameters." << endl;
        exit(EXIT_FAILURE);
    }

    SndfileHandle inputFile;
    int sampleRate;
    int *buffer;

    string path = argv[1];
    
    inputFile = SndfileHandle(path);

    auto frames = inputFile.frames();
    
    sampleRate = inputFile.samplerate();
    
    buffer = new int[sampleRate];

    inputFile.read(buffer, sampleRate);

    const vector<vector<int> > bauds = get_bauds(buffer, static_cast<int>(frames));

    const AmplitudeForBaud amplitudes = get_amplitudes(bauds);

    vector<int> demodulated_sequence = demodulate(bauds, amplitudes);

    write_to_file(demodulated_sequence, path);
    
    delete [] buffer;

    return EXIT_SUCCESS;
}

