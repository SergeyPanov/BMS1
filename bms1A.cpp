/*
 * File:   bms1A.cpp
 */

#include <cstdlib>
#include <cmath>

#include <iostream>
#include <fstream>


#include "sndfile.hh"
#include <vector>
#include <algorithm>
#include <regex>


#define SAMPLE_RATE 18000
#define CHANELS 1
#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define AMPLITUDE (1.0 * 0x7F000000)
#define FREQ (1000.0 / SAMPLE_RATE)
#define EXTENSION ".wav"
#define EXTENSION_REMOVER "\\.\\w*$"
using namespace std;

const vector<int> SYNCHRONIZE_SEQUENCE = {0, 0, 1, 1, 0, 0, 1, 1};    // Sequence used for synchronization


/**
 * Based on fst and snd bits return int in a way:
 * fst = 0; snd = 0 -> 0
 * fst = 0; snd = 1 -> 1
 * fst = 1; snd = 0 -> 2
 * fst = 1; snd = 1 -> 3
 * @param fst symbol
 * @param snd symbol
 * @return coded
 */
int8_t code_symbol(int fst, int snd) {
    fst = fst << 1;
    int8_t coded_symbol = 0;
    coded_symbol |= (fst | snd);
    return coded_symbol;
}

/**
 * Return multiplier of amplitude for particular symbol.
 * 00 -> 0
 * 01 -> 1/3
 * 10 -> 2/3
 * 11 -> 3/3
 * @param coded_symbol symbol receuved from "code_symbol" function
 * @return
 */
double get_multiplier(int coded_symbol) {
    switch (coded_symbol) {
        case 0:
            return 0;
        case 1:
            return 1.0 / 3;
        case 2:
            return 2.0 / 3;
        case 3:
            return 3.0 / 3;
        default:
            return 0;
    }
}

/**
 * Read input.txt vector from file.
 * @param path path to file
 * @return
 */
vector<int> read_input_vector(const string &path) {
    vector<int> input_vector;
    ifstream inFile;
    string inp;
    inFile.open(path);
    if (!inFile) {
        cerr << "Unable open file" << endl;
        exit(EXIT_FAILURE);
    }
    inFile >> inp;
    inFile.close();

    for (auto ch : inp) {
        if (ch != '0' && ch != '1') {
            cerr << "Invalid input" << endl;
            exit(EXIT_FAILURE);
        }
        input_vector.push_back(ch - '0');
    }

    return input_vector;
}

/*
 *
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Invalid amount of parameters" << endl;
        exit(EXIT_FAILURE);
    }

    string path = argv[1];  // Get input file

    vector<int> vec_buffer;

    vector<int> input_signal = read_input_vector(path); // Read vector of ints form file.
    SndfileHandle outputFile;
    int *buffer = new int[SAMPLE_RATE];

    input_signal.insert(input_signal.begin(), SYNCHRONIZE_SEQUENCE.begin(), SYNCHRONIZE_SEQUENCE.end());    // Insert synchronization sequence.

    auto samples_for_baud = static_cast<int>((SAMPLE_RATE) / (input_signal.size())); // Calculate sample rate used for modulation single baud

    unsigned int input_signal_index = 0; // Index of a particular bit in input_signal

    for (int i = 0; i < SAMPLE_RATE; i++) {
        double multiplier = 0;

        if (input_signal_index < input_signal.size()) {
            int bit0 = input_signal.at(static_cast<unsigned long>(input_signal_index));
            ++input_signal_index;
            int bit1 = input_signal.at(static_cast<unsigned long>(input_signal_index));
            ++input_signal_index;
            int coded_symbol = code_symbol(bit0, bit1);
            multiplier = get_multiplier(coded_symbol);
        }else{
            break;
        }

        for (int j = 0; j < samples_for_baud; ++j, ++i) {
            vec_buffer.push_back(static_cast<int &&>(multiplier * AMPLITUDE * sin(FREQ * 2 * i * M_PI)));
        }
    }

    string output;

    output = regex_replace(path, regex(EXTENSION_REMOVER), "");

    output += EXTENSION;

    outputFile = SndfileHandle(output, SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);

    outputFile.write(vec_buffer.data(), static_cast<sf_count_t>(vec_buffer.size()));
    delete[] buffer;
    return EXIT_SUCCESS;
}

