/*
 * File:   bms1A.cpp
 */

#include <cstdlib>
#include <cmath>

#include <iostream>
#include <fstream>


#include "sndfile.hh"
#include <vector>


#define SAMPLE_RATE 18000
#define CHANELS 1
#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define AMPLITUDE (1.0 * 0x7F000000)
#define FREQ (1000.0 / SAMPLE_RATE)
#define SIZE_MULTIPLIER 10
#define EXTENSION ".wav"
using namespace std;

const vector<int> SYNCHRONIZE_SEQUENCE = {0, 0, 1, 1, 0, 0, 1, 1};    // Sequence used for synchronization


/*
 * Code symbol in a way: 0 -> 00; 1 -> 01, 2 -> 10 -> 3 -> 11
 */
int8_t code_symbol(int fst, int snd) {
    fst = fst << 1;
    int8_t coded_symbol = 0;
    coded_symbol |= (fst | snd);
    return coded_symbol;
}

/*
 * Return multiplier of amplitude for particular symbol.
 * 00 -> 0
 * 01 -> 1/3
 * 10 -> 2/3
 * 11 -> 3/3
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

/*
 * Read input vector from file.
 */
vector<int> read_input_vector(const string &path) {
    vector<int> input_vector;
    ifstream inFile;
    string inp;
    inFile.open(path);
    if (!inFile) {
        cerr << "Unable open file" << endl;
        exit(1);
    }
    inFile >> inp;
    inFile.close();

    for (auto ch : inp) {
        if (ch != '0' && ch != '1') {
            cerr << "Invalid input" << endl;
            exit(1);
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
        exit(1);
    }

    SndfileHandle outputFile;
    int *buffer = new int[SAMPLE_RATE];

    vector<int> vec_buffer;

    vector<int> input_signal = read_input_vector(argv[1]);

    input_signal.insert(input_signal.begin(), SYNCHRONIZE_SEQUENCE.begin(), SYNCHRONIZE_SEQUENCE.end());

    int samples_for_baud = static_cast<int>(SAMPLE_RATE / (SIZE_MULTIPLIER *
                                                           input_signal.size())); // Calculate sample rate used for modulation single baud

    int input_signal_index = 0; // Index of a particular bit in input_signal


    for (int i = 0; i < SAMPLE_RATE; i++) {
        double multiplier = 0;

        if (input_signal_index < input_signal.size()) {
            int bit0 = input_signal.at(static_cast<unsigned long>(input_signal_index));
            ++input_signal_index;
            int bit1 = input_signal.at(static_cast<unsigned long>(input_signal_index));
            ++input_signal_index;
            int coded_symbol = code_symbol(bit0, bit1);
            multiplier = get_multiplier(coded_symbol);
        } else {
            break;
        }

        for (int j = 0; j < samples_for_baud; ++j, ++i) {
            vec_buffer.push_back(static_cast<int &&>(multiplier * AMPLITUDE * sin(FREQ * 2 * i * M_PI)));
        }
    }

    string output_file = argv[1];
    output_file += EXTENSION;

    outputFile = SndfileHandle(output_file, SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);

    outputFile.write(vec_buffer.data(), static_cast<sf_count_t>(vec_buffer.size()));
    delete[] buffer;
    return EXIT_SUCCESS;
}

