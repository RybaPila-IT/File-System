#ifndef _FILE_SYSTEM_ALLOCATOR_H
#define _FILE_SYSTEM_ALLOCATOR_H

#include "utility.h"

class Allocator {

private:

    uint16_t size;
    uint16_t first_free;
    vec_c    status;

    uint16_t find_first_free() {

        for (uint16_t i = 0; i < size; i++)
            if (status[i] == '1')
                return i;

        return size;
    }

    uint16_t free_blocks_amount() const {

        uint16_t amount = 0;

        for (uint16_t i = first_free; i < size; i++)
            if (status[i] == '1')
                amount++;

        return amount;
    }


public:
    explicit Allocator(std::ifstream& f) {
        size       = read_uint16_t(f);
        status     = read_string(f, size);
        first_free = find_first_free();
    }

    uint16_t get_size() const {
        return size;
    }

    uint16_t get_free_index() {

        while (first_free < size && status[first_free] != '1')
            first_free++;

        return first_free == size ? 0 : first_free;
    }

    void mark_as_used(uint16_t idx) {

        if (status[idx] == 0)
            throw std::runtime_error("Trying to corrupt used block");

        status[idx] = '0';
    }

    void dump_allocator_to_file(std::ofstream& f) {
        write_uint16_t(f, size);
        write_string(f, status, size);
    }

    void free(uint16_t idx) {

        if (idx == 0 || idx >= size)
            throw std::runtime_error("Trying to release unavailable block");

        if (status[idx] == '1')
            throw std::runtime_error("Trying to release free memory block");

        status[idx] = '1';
        first_free  = std::min(first_free, idx);
    }

    void info() const {
        std::cout << "Blocks in total: " << size << ". Free blocks: " << free_blocks_amount() << std::endl;
    }


};

#endif //_FILE_SYSTEM_ALLOCATOR_H
