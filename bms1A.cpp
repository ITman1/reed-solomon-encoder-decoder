/*******************************************************************************
 * Projekt:         Projekt č.1: Kodér a dekodér pro chybové zabezpečení dat
 * Předmět:         Bezdrátové a mobilní sítě
 * Soubor:          encoder.cpp
 * Datum:           Říjen 2013
 * Jméno:           Radim
 * Příjmení:        Loskot
 * Login autora:    xlosko01
 * E-mail:          xlosko01(at)stud.fit.vutbr.cz
 * Popis:           Hlavní modul, implementuje kodér Reed-Solomonon kódů
 *
 ******************************************************************************/

/**
 * @file encoder.cpp
 *
 * @brief Main (executable) module, implements encoder using Reed-Solomon codes.
 * @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
 */

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

extern "C" {
	#include "ecc.h"
}

using namespace std;

const int INTERLEAVING_STACKSIZE = 255;         /**< Defines how many blocks will interleaved */
const int INTERLEAVING_BLOCKSIZE = MSG_MAXSIZE; /**< Defines maximum size of message block */
const int PARITY_NUMBER = NPAR;                 /**< Defines number of parity bits */

unsigned char buffer[MSG_MAXSIZE];

/**
 * @brief Interleaves messages to the output file.
 * @param os Output where to put interleaved messages.
 * @param data Data to be interleaved.
 */
void printData(ofstream &os, vector<vector<unsigned char> > &data) {
    vector<pair<vector<unsigned char> *, vector<unsigned char>::iterator >> data_iters;

    /* Initialize vector of all messages and corresponding iterators */
    for_each(data.begin(), data.end(), [&data_iters] (vector<unsigned char> &block) {
        data_iters.push_back(pair<vector<unsigned char> *, vector<unsigned char>::iterator>(&block, block.begin()));
    });

    /* Iterates over all messages until all their iterators are empty */
    while (!data_iters.empty()) {
        vector<pair<vector<unsigned char> *, vector<unsigned char>::iterator >>::iterator block_iter = data_iters.begin();

        while (block_iter != data_iters.end()) {

            /* Remove message from the vector if its iterator reached the end */
            if (block_iter->second ==  block_iter->first->end()) {
                block_iter = data_iters.erase(block_iter);
            } else {
                os << (char)*block_iter->second;
                ++block_iter->second;
                ++block_iter;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        cerr << "Missing argument that specifies path to the file which should be encoded!" << endl;
        return EXIT_FAILURE;
    } else if (argc > 2) {
        cerr << "Too many program arguments! Expected only one argument!" << endl;
    }

    /* Open input stream which should be encoded */
    ifstream is;
    is.open(argv[1], ios::in | ifstream::binary );
    if( !is ) {
        cerr << "Failed to open file!" << endl;
        return EXIT_FAILURE;
    }

    /* Open output stream where to put encoded data */
    ofstream os;
    os.open(string(argv[1]) + ".out", ios::out | ofstream::binary );

    vector<vector<unsigned char> > inputBuffs;
    istreambuf_iterator<char> is_iter(is);
    typedef istreambuf_iterator<char>::traits_type is_traits;
    bool isEOF = false;
    int stackSize = 0;

    initialize_ecc ();
    while(!isEOF && is.good()) {
        int currPos = is.tellg();
        is.seekg(0, is.end);
        int remPos = static_cast<int>(is.tellg()) - currPos; // Number of remaining bytes to the end of file
        is.seekg(currPos);
        isEOF = remPos == 0;

        if (!isEOF) {
            /* Insert new buffer into vector of buffers (stack) */
            stackSize++;
            inputBuffs.push_back(vector<unsigned char>());
            vector <unsigned char> &currInputBuff = inputBuffs.back();

            /* Read n bytes from file into current buffer */
            int read_bytes = INTERLEAVING_BLOCKSIZE - PARITY_NUMBER;
            read_bytes = (remPos >= read_bytes)? read_bytes : remPos;
            copy_n (is_iter, read_bytes, back_inserter(currInputBuff));
            is_iter++;
        }

        // Encode input and print into destination file
        if (/* TODO: Uncomment for stack encoding support stackSize == INTERLEAVING_STACKSIZE || */ isEOF) {
            for_each(inputBuffs.begin(), inputBuffs.end(), [] (vector<unsigned char> &block) {
                int block_size = block.size();
                encode_data(&block[0], block_size, const_cast<unsigned char *>(buffer));
                block.resize(block_size + PARITY_NUMBER);
                copy (const_cast<unsigned char *> (buffer), const_cast<unsigned char *>(&buffer[block_size + PARITY_NUMBER]), block.begin());
            });
            printData(os, inputBuffs);
            inputBuffs.clear();
            stackSize = 0;
        }
    }

    is.close();
    os.close();

    return EXIT_SUCCESS;
}
