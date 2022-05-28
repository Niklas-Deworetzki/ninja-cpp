
#pragma once

/**
 * The program loader of the NJVM.
 */

namespace NJVM {

    /**
     * Amount of bytes in ninja binary file's magic.
     */
    constexpr size_t NJBF_MAGIC_SIZE = 4;

    /**
     * Bytes of ninja binary file's magic.
     */
    constexpr char NJBF_MAGIC[NJBF_MAGIC_SIZE] = {'N', 'J', 'B', 'F'};

    /**
     * Load a ninja binary file for execution into this VM.
     *
     * The binary file must start with the NJBF magic number and
     * defines how many objects are reserved for static data, as
     * well as the program to be executed.
     *
     * @param filename C-style string of the path to the binary file to load.
     */
    void load(const char *filename);

}
