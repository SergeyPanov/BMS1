/*
 * File:   bms1A.cpp
 */

#include <cstdlib>
#include <cmath>

#include "sndfile.hh"
#include <vector>

#define SAMPLE_RATE 18000
#define CHANELS 1
#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
#define AMPLITUDE (1.0 * 0x7F000000)
#define FREQ (1000.0 / SAMPLE_RATE)
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
 *
 */
int main(int argc, char **argv) {

    SndfileHandle outputFile;
    int *buffer = new int[SAMPLE_RATE];

    vector<int> vec_buffer;

    vector<int> input_signal = {0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0};

    input_signal.insert(input_signal.begin(), SYNCHRONIZE_SEQUENCE.begin(), SYNCHRONIZE_SEQUENCE.end());


//    int input_signal[] = {0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0};

    int8_t symbol_int_representation = 0;  // Used for represent single symbol. 0 -> 00; 1 -> 01, 2 -> 10 -> 3 -> 11

    int samples_for_baud = SAMPLE_RATE / (60*input_signal.size() / 2); // Calculate sample rate used for modulation single baud

    int input_signal_index = 0; // Index of a particular bit in input_signal


    for (int i = 0; i < SAMPLE_RATE; i++) {
        double multiplier = 0;

        if (input_signal_index < input_signal.size()) {
            int bit0 = input_signal.at(input_signal_index);
            ++input_signal_index;
            int bit1 = input_signal.at(input_signal_index);
            ++input_signal_index;
            int coded_symbol = code_symbol(bit0, bit1);
            multiplier = get_multiplier(coded_symbol);
        } else {
            break;
        }

        for (int j = 0; j < samples_for_baud; ++j, ++i) {
            vec_buffer.push_back(multiplier * AMPLITUDE * sin(FREQ * 2 * i * M_PI));
//            buffer[i] = multiplier * AMPLITUDE * sin(FREQ * 2 * i * M_PI);

        }
    }

        outputFile = SndfileHandle("sine.wav", SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);


//        outputFile.write(buffer, SAMPLE_RATE);

        outputFile.write(vec_buffer.data(), vec_buffer.size());
        delete[] buffer;
        return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------------------------------------------------------------------
//
//
///*
// * File:   bms1A.cpp
// */
//
//#include <cstdlib>
//#include <math.h>
//
//#include "sndfile.hh"
//
//#define SAMPLE_RATE 18000
//#define CHANELS 1
//#define FORMAT (SF_FORMAT_WAV | SF_FORMAT_PCM_24)
//#define AMPLITUDE (1.0 * 0x7F000000)
//#define FREQ (1000.0 / SAMPLE_RATE)
//
///*
// *
// */
//int main(int argc, char** argv) {
//
//    SndfileHandle outputFile;
//    int *buffer = new int[SAMPLE_RATE];
//
//
//    for (int i = 0; i < SAMPLE_RATE; i++)
//        buffer [i] = AMPLITUDE * sin(FREQ * 2 * i * M_PI);
//
//
//    outputFile = SndfileHandle("sine.wav", SFM_WRITE, FORMAT, CHANELS, SAMPLE_RATE);
//
//
//    outputFile.write(buffer, SAMPLE_RATE);
//
//    delete [] buffer;
//    return EXIT_SUCCESS;
//}

