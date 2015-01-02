/*******************************************************************************
 * Projekt:         Projekt č.1: Kodér a dekodér pro chybové zabezpečení dat
 * Předmět:         Bezdrátové a mobilní sítě
 * Soubor:          decoder.cpp
 * Datum:           Říjen 2013
 * Jméno:           Radim
 * Příjmení:        Loskot
 * Login autora:    xlosko01
 * E-mail:          xlosko01(at)stud.fit.vutbr.cz
 * Popis:           Hlavní modul, implementuje dekodér Reed-Solomonon kódů
 *
 ******************************************************************************/

/**
 * @file decoder.cpp
 *
 * @brief Main (executable) module, implements decoder using Reed-Solomon codes.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <climits>

extern "C" {
	#include "ecc.h"
}

using namespace std;

const int INTERLEAVING_STACKSIZE = INT_MAX /*TODO: Uncomment for stack encoding support: 255*/;         /**< Defines how many blocks will interleaved */
const int INTERLEAVING_BLOCKSIZE = MSG_MAXSIZE; /**< Defines maximum size of message block */
const int PARITY_NUMBER = NPAR;                 /**< Defines number of parity bits */

/**
 * Alias to vector of vector.
 */
template <class ValueType>
using Vec2D = vector<vector<ValueType> >;

/**
 * Iterator which appends values into 2D vector.
 * Iterates over first dimension and on assign operator appends values into current vector
 */
template <class ValueType>
class Vec2D_back_insert_iterator : public iterator<output_iterator_tag,void,void,void,void> {

    protected:
        Vec2D<ValueType>* container;
        typename Vec2D<ValueType>::iterator iter;

    public:
        typedef ValueType vylue_type;
        typedef Vec2D<ValueType> container_type;

        explicit Vec2D_back_insert_iterator (Vec2D<ValueType>& x) : container(&x), iter(x.begin()) {}

        Vec2D_back_insert_iterator<ValueType>& operator= (const ValueType& value)
            { iter->push_back(value); return *this; }
        Vec2D_back_insert_iterator<ValueType>& operator= (ValueType&& value)
            { iter->push_back(std::move(value)); return *this; }
        Vec2D_back_insert_iterator<ValueType>& operator* ()
            { return *this; }
        Vec2D_back_insert_iterator<ValueType>& operator++ ()
            { iter++; return *this; }
        Vec2D_back_insert_iterator<ValueType> operator++ (int)
            { iter++; return *this; }
};


int main(int argc, char *argv[]) {

    if (argc < 2) {
        cerr << "Missing argument that specifies path to the file which should be decoded!" << endl;
        return EXIT_FAILURE;
    } else if (argc > 2) {
        cerr << "Too many program arguments! Expected only one argument!" << endl;
    }

    /* Open input stream which should be decoded */
    ifstream is;
    is.open(argv[1], ios::in | ifstream::binary );
    if( !is ) {
        cerr << "Failed to open file!" << endl;
        return EXIT_FAILURE;
    }

    /* Open output stream where to put decoded data */
    ofstream os;
    os.open(string(argv[1]) + ".ok", ios::out | ofstream::binary );

    bool isEOF = false;

    initialize_ecc ();
    while(is.good() && !isEOF) {
        int currPos = is.tellg();
        is.seekg(0, is.end);
        int remPos = static_cast<int>(is.tellg()) - currPos; // Number of remaining bytes to the end of file
        is.seekg(currPos);

        bool failed = false;
        isEOF = remPos == 0;

        /* Determine how many blocks should be read and the number of bytes in the last block */
        int stackSize = (remPos / INTERLEAVING_BLOCKSIZE) + ((remPos % INTERLEAVING_BLOCKSIZE != 0)? 1 : 0);
        int lastBlockSize = (remPos != 0 && remPos % INTERLEAVING_BLOCKSIZE == 0)? INTERLEAVING_BLOCKSIZE : remPos % INTERLEAVING_BLOCKSIZE;
        lastBlockSize = (stackSize > INTERLEAVING_STACKSIZE)? INTERLEAVING_BLOCKSIZE : lastBlockSize;
        stackSize = (stackSize > INTERLEAVING_STACKSIZE)? INTERLEAVING_STACKSIZE : stackSize;

        /* Read interleaved messages into vector of messages */
        Vec2D<unsigned char> inputBuffs(stackSize);
        for (int i = 0; ((stackSize > 1) && (i < INTERLEAVING_BLOCKSIZE)) || ((stackSize == 1) && (i < lastBlockSize)); i++) {
            istreambuf_iterator<char> is_iter(is);
            int seekLen = (i < lastBlockSize)? stackSize : stackSize - 1; // After reading all bytes of the final block omit this block from the interleaving

            // Append one character to each message
            copy_n (is_iter, seekLen, Vec2D_back_insert_iterator<unsigned char>(inputBuffs));
            is_iter++;
        }

        // Decode input and print into destination file
        if (!inputBuffs.empty()) {
			
            int pos = 0;
            for_each(inputBuffs.begin(), inputBuffs.end(), [&failed, &pos, &currPos] (vector<unsigned char> &block) {
                long int block_number = (currPos / ((long int)INTERLEAVING_STACKSIZE * INTERLEAVING_BLOCKSIZE )) * INTERLEAVING_STACKSIZE + pos;
                decode_data(&block[0], block.size());
                /* check if syndrome is all zeros */
                if (check_syndrome () != 0) {
                    if (!correct_errors_erasures (&block[0], block.size(), 0, NULL) && !failed) {
                        cerr << "Unable to correct errors in block #" << block_number << "!" << endl;
                        //failed = true;
                    }
                }
                int len = block.size() - NPAR;
                if (len < 0 && !failed) {
                    cerr << "Block #" << block_number << "has bad size!" << endl;
                    failed = true;
                } else if (!failed) {
                    block.resize(block.size() - NPAR);
                }
                pos++;
            });

            if (failed) {
                break;
            }

            // Print to destination file
            for_each(inputBuffs.begin(), inputBuffs.end(), [&os] (vector<unsigned char> &block) {
                copy(block.begin(), block.end(), ostream_iterator<char>(os));
            });

            inputBuffs.clear();
            stackSize = 0;
        }
    }

    is.close();
    os.close();

    return EXIT_SUCCESS;
}
