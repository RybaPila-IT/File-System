#ifndef _FILE_SYSTEM_MEMORY_BLOCKS_H
#define _FILE_SYSTEM_MEMORY_BLOCKS_H

/**
 *
 * Class storing memory of the file system.
 *
 */
class Memory_Blocks {

private:
    friend class File_System;

    static const ui content_size = 50;

    struct Memory_Block;
    using mem_v = std::vector<Memory_Block>;

    mem_v blocks;

    struct Memory_Block {

        uint16_t next_block;
        byte     occupied;
        vec_c    content;

        explicit Memory_Block(std::ifstream& f) {

            next_block = read_uint16_t(f);
            occupied   = read_byte(f);
            content    = read_string(f, content_size);
        }

        void dump_memory_block(std::ofstream& f) const {

                write_uint16_t(f, next_block);
                write_byte(f, occupied);
                write_string(f, content, content_size);
        }

        void clear_memory_block() {

            next_block = 0;
            occupied   = 0;

            for (ui i = 0; i < content_size; i++)
                content[i] = '\0';
        }

    };

    void fill_content_with_memory_chunk(uint16_t mem_start, vec_c& content) {

        do {

            uint16_t block_idx = 0;

            while (block_idx < blocks[mem_start].occupied)
                content.push_back(blocks[mem_start].content[block_idx++]);

            mem_start = blocks[mem_start].next_block;

        } while (mem_start);
    }

public:

    explicit Memory_Blocks(std::ifstream& f, uint16_t size) {

        for (uint16_t i = 0; i < size; i++)
            blocks.push_back(Memory_Block(f));
    }

    static ui get_memory_block_size() {
        return content_size;
    }

    // Function properly saves content inside content vec into memory system blocks.
    void save_file(uint16_t mem_block, const vec_c& content) {

        uint16_t con_idx     = 0;
        uint16_t mem_idx     = 0;

        while (con_idx < content.size()) {

                if (mem_idx == content_size) {

                    mem_idx = 0;
                    blocks[mem_block].occupied = (byte) content_size;
                    mem_block = blocks[mem_block].next_block;
                }

                blocks[mem_block].content[mem_idx++] = content[con_idx++];
        }

        blocks[mem_block].occupied = (byte) mem_idx;
    }


    void dump_memory_blocks_to_file(std::ofstream& f) {

        for (auto& block : blocks)
            block.dump_memory_block(f);
    }


    vec_16 free_memory(uint16_t n) {

        vec_16 freed_blocks;

        do {

            freed_blocks.push_back(n);
            uint16_t next_block  = blocks[n].next_block;
            blocks[n].clear_memory_block();
            n = next_block;
        } while (n);

        return freed_blocks;
    }

    uint16_t get_file_size(uint16_t start) {

        uint16_t size = 0;

        do {

            size += content_size;
            start = blocks[start].next_block;
        } while (start);

        return size;
    }

    void append_to_block_list(uint16_t start, uint16_t next) {

        while (blocks[start].next_block)
            start = blocks[start].next_block;

        blocks[start].next_block = next;
        blocks[next].clear_memory_block();
    }

    vec_c full_file_content(uint16_t mem_start) {

        vec_c content;
        fill_content_with_memory_chunk(mem_start, content);

        return content;
    }

    uint16_t erase_from_block_list(uint16_t start) {

        uint16_t n_start = blocks[start].next_block;

        if (!n_start)
            throw std::runtime_error("CRITICAL ERROR. Trying to shrink directory into 0 blocks but directory still exists");

        while (blocks[n_start].next_block) {

            start   = n_start;
            n_start = blocks[n_start].next_block;
        }

        blocks[start].next_block = 0;
        blocks[n_start].clear_memory_block();

        return n_start;
    }

};

#endif //_FILE_SYSTEM_MEMORY_BLOCKS_H
