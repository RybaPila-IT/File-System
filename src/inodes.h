#ifndef _FILE_SYSTEM_INODES_H
#define _FILE_SYSTEM_INODES_H

#include "utility.h"

/**
 * Class managing inodes.
 *
 * This class is responsible for allocating new
 * inodes and erasing these, which files are being
 * deleted. It also gives necessary info which
 * is connected with the inode itself.
 */
class Inodes {

private:

    struct Inode;

    using inode_v = std::vector<Inode>;

    inode_v nodes;

    struct Inode {

        byte     is_dir;
        byte     number;
        uint16_t memory_block;

        explicit Inode(std::ifstream& f) {

            is_dir       = read_byte(f);
            number       = read_byte(f);
            memory_block = read_uint16_t(f);
        }

        void dump_inode(std::ofstream& f) const {

            write_byte(f, is_dir);
            write_byte(f, number);
            write_uint16_t(f, memory_block);
        }

    };

public:
    explicit Inodes(std::ifstream& f, uint16_t size): nodes() {

        for (uint16_t i = 0; i < size; i++)
            nodes.push_back(Inode(f));

    }

    uint16_t get_memory_block(uint16_t inode_number) {
        return nodes[inode_number].memory_block;
    }

    void dump_inodes_to_file(std::ofstream& f) {

        for (auto& inode : nodes)
            inode.dump_inode(f);
    }

    void create_new_inode(uint16_t inode_number, byte is_dir, uint16_t mem_block) {

        nodes[inode_number].is_dir       = is_dir;
        nodes[inode_number].number       = is_dir ? 0 : 1;
        nodes[inode_number].memory_block = mem_block;
    }

    uint16_t get_inode_mem_block(uint16_t n) const {
        return nodes[n].memory_block;
    }

    byte get_inode_pointers(uint16_t n) const {
        return nodes[n].number;
    }

    bool is_inode_directory(uint16_t n) const {
        return nodes[n].is_dir;
    }

    void add_pointer_to_inode(uint16_t n) {
        nodes[n].number++;
    }

    void remove_pointer_from_inode(uint16_t n) {
        nodes[n].number--;
    }

};


#endif //_FILE_SYSTEM_INODES_H
