#ifndef _FILE_SYSTEM_FILE_SYSTEM_H
#define _FILE_SYSTEM_FILE_SYSTEM_H

#include <cstring>
#include "allocator.h"
#include "inodes.h"
#include "utility.h"
#include "memory_blocks.h"

/**
 * Top class managing file system.
 *
 * Class gathers all utilities provided by single
 * file system classes (such as Memory_BLocks/Inodes) and
 * binds them all together in order to manage file system stored on file.
 */
class File_System {

private:
    Allocator     inodes_allocator;
    Inodes        inodes;
    Allocator     memory_allocator;
    Memory_Blocks memory;

    // Function seeks for directory specified with path vector.
    // If during traversing file system is stops to find valid
    // directories it will continue to create new directories.
    Directory find_directory(const vec_s& path) {

        uint16_t inode = 0;
        uint16_t block = 0;

        auto dir_content = memory.full_file_content(block);

        Directory dir (inode, block, dir_content);

        for (auto const& s : path) {

            inode = dir.get_file_inode(s);

            if (!inode) {
                add_new_file_to_directory(dir, s, true);
                save_directory_to_memory(dir);
                inode = dir.get_file_inode(s);
            }

            if (!inodes.is_inode_directory(inode))
                throw std::runtime_error("Incorrect path (found file inside specified path)");

            block       = inodes.get_memory_block(inode);
            dir_content = memory.full_file_content(block);
            dir         = Directory(inode, block, dir_content);
        }

        return dir;
    }

    // Function adds new file or directory (specified with bool argument)
    // into existing directory. It informs inodes allocator and inodes
    // structures to mark specified fields as used and also asks memory
    // allocation system for 1 block of memory for created file.
    void add_new_file_to_directory(Directory& dir, const std::string& file_name, bool is_dir) {

        uint16_t file_inode     = inodes_allocator.get_free_index();
        uint16_t file_mem_block = memory_allocator.get_free_index();

        if (!file_inode || !file_mem_block)
            throw std::runtime_error("Unable to create new file; Missing free space");

        inodes_allocator.mark_as_used(file_inode);
        memory_allocator.mark_as_used(file_mem_block);

        inodes.create_new_inode(file_inode, is_dir, file_mem_block);
        dir.add_new_file(file_name, file_inode);
        inodes.add_pointer_to_inode(dir.inode_num);
    }

    // Function allocates needed blocks of memory for a
    // file or directory if it has run out of space.
    // Function will ask memory allocation system for as many
    // blocks as possible in order to fulfill the requirements
    // needed for saving the file into file system.
    void allocate_needed_memory(uint16_t mem_block, uint16_t dir_content_size, uint16_t dir_actual_size) {

        while (dir_content_size > dir_actual_size) {

            uint16_t next_block = memory_allocator.get_free_index();

            if (!next_block)
                throw std::runtime_error("Unable to extend directory; Out of memory");

            memory.append_to_block_list(mem_block, next_block);
            memory_allocator.mark_as_used(next_block);
            dir_actual_size += Memory_Blocks::get_memory_block_size();
        }
    }

    // Function will free memory blocks from a file
    // which are no longer crucial for it`s existence.
    // It will simply inform memory allocation system
    // to free as many blocks as possible until the file
    // fits into memory blocks list.
    void deallocate_excessive_memory(uint16_t mem_block, uint16_t dir_content_size, uint16_t dir_actual_size) {

        while (dir_content_size < dir_actual_size - Memory_Blocks::get_memory_block_size()) {

            uint16_t freed_block = memory.erase_from_block_list(mem_block);
            memory_allocator.free(freed_block);
            dir_actual_size -= Memory_Blocks::get_memory_block_size();
        }

    }

    // Function will add link to directory.
    // Src argument is the inode number of
    // original file present in file system. Dir
    // is the directory into which the link will be added.
    // Lastly, the link is the name of the link which
    // will be created.
    void add_link_to_directory(Directory& dir, uint16_t src, const std::string& link) {

        if (!src)
            throw std::runtime_error("File does not exist");

        if (inodes.is_inode_directory(src))
            throw std::runtime_error("Unable to link directory");

        dir.add_new_file(link, src);
        inodes.add_pointer_to_inode(dir.inode_num);
        inodes.add_pointer_to_inode(src);
    }

    // Self-explaining.
    void save_directory_to_memory(const Directory& dir) {

        auto dir_content = dir.get_directory_content();
        auto mem_block   = dir.mem_block;
        save_content_to_memory(mem_block, dir_content);
    }

    // Self-explaining.
    void save_file_to_memory(const File& file) {

        auto file_content = file.get_file_content();
        auto file_mem_b   = file.get_file_mem_block();
        save_content_to_memory(file_mem_b, file_content);
    }

    // Function will save the vector named content
    // containing the content of the file or the
    // directory into the memory list pointed by
    // mem_block. Mem_block points to the head
    // of the memory blocks list.
    // IMPORTANT: at this point there is an assumption
    // that all needed memory is already allocated at the list
    // and the content will safely fit into this list.
    void save_content_to_memory(uint16_t mem_block, const vec_c& content) {

        uint16_t content_size = content.size();
        uint16_t actual_size  = memory.get_file_size(mem_block);

        if (content_size > actual_size)
            allocate_needed_memory(mem_block, content_size, actual_size);
        else
            deallocate_excessive_memory(mem_block, content_size, actual_size);

        memory.save_file(mem_block, content);
    }

    // Function checks whether the directory does not
    // have any files/dirs/links inside.
    // Generally speaking it checks if the dir is empty.
    bool can_erase_directory_from_directory(Directory& dir, const std::string& s) {

        uint16_t inode = dir.get_file_inode(s);

        return !inodes.get_inode_pointers(inode);
    }

    // Function erases the file/directory/link from directory
    // specified as the dir argument.
    // Later it informs the whole file system about necessary
    // tasks needed to maintain the unity of the system:
    // freeing inode and memory block, erasing memory list
    // and decreasing number of files inside the directory.
    void erase_from_directory(Directory& dir, const std::string& s) {

        uint16_t file_node = dir.get_file_inode(s);

        if (!file_node)
            throw std::runtime_error("File not found");

        if (inodes.is_inode_directory(file_node) && !can_erase_directory_from_directory(dir, s))
            throw std::runtime_error("Unable to erase directory. Directory is not empty");
        else if (!inodes.is_inode_directory(file_node))
            inodes.remove_pointer_from_inode(file_node);

        if (!inodes.get_inode_pointers(file_node)) {
            uint16_t mem_block = inodes.get_inode_mem_block(file_node);
            inodes_allocator.free(file_node);
            auto freed_blocks = memory.free_memory(mem_block);
            for (auto block : freed_blocks)
                memory_allocator.free(block);

        }

        dir.erase_file(s);
        inodes.remove_pointer_from_inode(dir.inode_num);
    }

    // Function gets File object representing a file
    // contained by directory dir.
    File get_file(const Directory& dir, const std::string& file_name) {

        uint16_t file_inode = dir.get_file_inode(file_name);
        uint16_t file_mem   = inodes.get_inode_mem_block(file_inode);

        if (!file_inode)
            throw std::runtime_error("File not found; Unable to write into file");

        if (inodes.is_inode_directory(file_inode))
            throw std::runtime_error("Attempt to open directory as file");

        auto content = memory.full_file_content(file_mem);

        return File(file_mem, content);
    }

    // Self-explaining.
    ui get_dir_size(uint16_t inode) {

        auto mem_b = inodes.get_inode_mem_block(inode);
        auto conte = memory.full_file_content(mem_b);

        Directory dir(inode, mem_b, conte);
        ui size = dir.get_directory_content().size();

        for (unsigned short i : dir.inodes) {

            if (inodes.is_inode_directory(i))
                size += get_dir_size(i);
            else {
                mem_b = inodes.get_inode_mem_block(i);
                conte = memory.full_file_content(mem_b);
                size  += conte.size();
            }

        }

        return size;
    }

    // Function gives an information about specified directory.
    void info_directory(const Directory& dir) {

        std::cout << "Dir size: " << dir.get_directory_content().size() << " bytes" << std::endl;

        for (ui i = 0; i < dir.inodes.size(); i++) {

            if (!i)
                std::cout << "Inner files and directories info:" << std::endl;

            if (inodes.is_inode_directory(dir.inodes[i]))
                std::cout << dir.names[i] << " ---> " << get_dir_size(dir.inodes[i]) << " bytes" << std::endl;
            else {
                auto mem_b = inodes.get_inode_mem_block(dir.inodes[i]);
                auto conte = memory.full_file_content(mem_b);
                std::cout << dir.names[i] << " ---> " << conte.size() << " bytes" << std::endl;
            }
        }
    }

    static void print_content_of_directory(const Directory & dir) {
        dir.print_content();
    }

    static void print_file_content(const File& file) {
        file.print_content();
    }

    static void add_to_file(File& f, const std::string& m) {
        f.add_to_file(m);
    }

    static void cut_from_file(File& f, ui to_cut) {
        f.cut_from_file(to_cut);
    }

    static void info_file(const File& file) {
        std::cout << "File size: " << file.content.size() << " bytes" << std::endl;
    }

public:
    explicit File_System(std::ifstream& f):
            inodes_allocator(f), inodes(f, inodes_allocator.get_size()),
            memory_allocator(f), memory(f, memory_allocator.get_size()) {}


    void add_file(const vec_s& path, const std::string& file_name) {

        Directory dir = find_directory(path);
        add_new_file_to_directory(dir, file_name, false);
        save_directory_to_memory(dir);
    }

    void write_to_file(const vec_s& path, const std::string& file_name, const std::string& m) {

        Directory dir  = find_directory(path);
        File      file = get_file(dir, file_name);

        add_to_file(file, m);
        save_file_to_memory(file);
    }

    void cut(const vec_s& path, const std::string& file_name, ui to_cut) {

        Directory dir  = find_directory(path);
        File      file = get_file(dir, file_name);

        cut_from_file(file, to_cut);
        save_file_to_memory(file);
    }


    void erase(const vec_s& path, const std::string& file_name) {

        Directory dir = find_directory(path);
        erase_from_directory(dir, file_name);
        save_directory_to_memory(dir);
    }

    void cat(const vec_s& path, const std::string& name) {

        auto dir        = find_directory(path);
        auto file_inode = dir.get_file_inode(name);
        auto file_mem_b = inodes.get_inode_mem_block(file_inode);
        auto content    = memory.full_file_content(file_mem_b);

        if (!file_inode && name != "/")
            throw std::runtime_error("File does not exist. Unable to perform cat operation");

        if (inodes.is_inode_directory(file_inode)) {
            dir = Directory(file_inode, file_mem_b, content);
            print_content_of_directory(dir);
        } else {
            File file = File(file_mem_b, content);
            print_file_content(file);
        }

    }

    void mkdir(const vec_s& path, const std::string& dir_name) {

        Directory dir = find_directory(path);
        add_new_file_to_directory(dir, dir_name, true);
        save_directory_to_memory(dir);
    }

    void link(const vec_s& f_path, const std::string& file, const vec_s& l_path, const std::string& link) {

        auto dir     = find_directory(f_path);
        auto f_inode = dir.get_file_inode(file);

        dir = find_directory(l_path);
        add_link_to_directory(dir, f_inode, link);
        save_directory_to_memory(dir);
    }

    void info(const vec_s& path, const std::string& name) {

        auto dir   = find_directory(path);
        auto inode = dir.get_file_inode(name);
        auto mem_b = inodes.get_inode_mem_block(inode);
        auto conte = memory.full_file_content(mem_b);

        if (!inode && name != "/")
            throw std::runtime_error("File or directory does not exist.");

        if (inodes.is_inode_directory(inode)) {
            dir = Directory(inode, mem_b, conte);
            info_directory(dir);
        }
        else {
            File file(mem_b, conte);
            info_file(file);
        }

    }

    vec_c get_file_content(const vec_s& path, const std::string& name) {

        auto dir        = find_directory(path);
        auto file_inode = dir.get_file_inode(name);
        auto mem_block  = inodes.get_inode_mem_block(file_inode);
        auto content    = memory.full_file_content(mem_block);

        if (inodes.is_inode_directory(file_inode))
            throw std::runtime_error("Attempt to get directory");

        return memory.full_file_content(mem_block);
    }

    void memory_info() const {
        memory_allocator.info();
    }

    void inodes_info() const {
        inodes_allocator.info();
    }

    void dump_file_system_to_file(std::ofstream& f) {

        inodes_allocator.dump_allocator_to_file(f);
        inodes.dump_inodes_to_file(f);
        memory_allocator.dump_allocator_to_file(f);
        memory.dump_memory_blocks_to_file(f);
    }

};

/**
 * Class handling user input.
 *
 * File_System_Manager class is a translator
 * between user and the file system. It gathers
 * user input and based upon it orders File_System
 * class to perform required tasks.
 * This class is also responsible for creating
 * clean file system if this action is going to
 * be performed.
 */
class File_System_Manager {

private:

    static const char* end;
    static const char* cat;
    static const char* copy;
    static const char* erase;
    static const char* mkdir;
    static const char* echo;
    static const char* touch;
    static const char* link;
    static const char* cut;
    static const char* info;
    static const char* memory;
    static const char* inodes;
    static const char* get;

    static void write_manager(std::ofstream& out, uint16_t size) {

        byte arr[2];

        memcpy(arr, &size, sizeof(size));

        out << arr[0];
        out << arr[1];

        for (uint16_t i = 0; i < size; i++)
            out << (i == 0 ? '0' : '1');

    }

    static void write_inodes(std::ofstream& out, uint16_t size) {

        for (uint16_t i = 0; i < size; i++) {
            if (i == 0)
                out << (byte) 1 << (byte) 0 << (byte) 0 << (byte) 0;
            else
                out << (byte) 0 << (byte) 0 << (byte) 0 << (byte) 0;
        }
    }

    static void write_memory_blocks(std::ostream & out, uint16_t size) {

        const uint16_t block_content = 50;
        char  empty_block[block_content];

        for (char & i : empty_block)
            i = 0;

        for (uint16_t i = 0; i < size; i++) {
            out << (byte) 0 << (byte) 0 << (byte) 0;
            out.write(empty_block, block_content);
        }

    }

    static ui get_file_content_size(std::ifstream& input) {

        input.seekg (0, std::ifstream::end);
        ui size = input.tellg();
        input.seekg (0, std::ifstream::beg);

        return size;
    }

    static std::string convert_to_string(char* buffer, ui length) {

        std::string m;

        for (ui i = 0; i < length; i++)
            m.append(1, buffer[i]);
            
        delete [] buffer;

        return m;
    }

    static std::string get_file_content(const std::string& name) {

        std::ifstream input(name);

        if (!input)
            throw std::runtime_error("Unable to open file to copy from");

        ui length = get_file_content_size(input);
        char* buffer = new char[length];
        input.read(buffer, length);

        return convert_to_string(buffer, length);
    }

    static void echo_command(File_System& system, const std::string& file, const vec_s& file_path) {

        std::string message;
        std::getline(std::cin, message);
        message.erase(0, 1);

        system.write_to_file(file_path, file, message);
    }

    static void cat_command(File_System& system, const std::string& file, const vec_s& file_path) {
        system.cat(file_path, file);
    }

    static void touch_command(File_System& system, const std::string& file, const vec_s& file_path) {
        system.add_file(file_path, file);
    }

    static void erase_command(File_System& system, const std::string& file, const vec_s& file_path) {
        system.erase(file_path, file);
    }

    static void mkdir_command(File_System& system, const std::string& dir, const vec_s& dir_path) {
        system.mkdir(dir_path, dir);
    }

    static void copy_command(File_System& system, const std::string& file, const vec_s& file_path) {

        std::string src_file, content;
        std::cin >> src_file;
        content = get_file_content(src_file);

        system.write_to_file(file_path, file, content);
    }

    static void link_command(File_System& system, const std::string& file, const vec_s& file_path) {

        std::string l;
        std::cin >> l;
        auto link_path = path(l);

        system.link(file_path, file, link_path, l);
    }

    static void cut_command(File_System& system, const std::string& file, const vec_s& file_path) {

        ui to_cut;
        std::cin >> to_cut;

        system.cut(file_path, file, to_cut);
    }

    static void info_command(File_System& system, const std::string& file, const vec_s& file_path) {

        if (file == memory)
            system.memory_info();
        else if (file == inodes)
            system.inodes_info();
        else
            system.info(file_path, file);

    }

    static void get_command(File_System& system, const std::string& file, const vec_s& file_path) {

        std::string output_path;
        std::cin >> output_path;

        std::ofstream output(output_path);

        if (!output)
            throw std::runtime_error("File does not exist");

        auto content = system.get_file_content(file_path, file);

        write_string(output, content, content.size());
        output.close();
    }

public:
    static void make_empty_file_system(std::ofstream& out, uint16_t bytes) {

        uint16_t size = bytes / 4;

        write_manager(out, size);       // Inodes manager.
        write_inodes(out, size);        // Inodes.
        write_manager(out, size);       // Memory manager.
        write_memory_blocks(out, size); // Memory blocks.
    }

    static void manage_file_system(File_System& system) {

        std::string command, file, message;
        std::cin >> command;

        while (command != end) {

            std::cin >> file;
            vec_s file_path = path(file);

            try {

                if (command == echo)
                    echo_command(system, file, file_path);
                else if (command == touch)
                    touch_command(system, file, file_path);
                else if (command == cat)
                    cat_command(system, file, file_path);
                else if (command == erase)
                    erase_command(system, file, file_path);
                else if (command == mkdir)
                    mkdir_command(system, file, file_path);
                else if (command == copy)
                    copy_command(system, file, file_path);
                else if (command == link)
                    link_command(system, file, file_path);
                else if (command == cut)
                    cut_command(system, file, file_path);
                else if (command == info)
                    info_command(system, file, file_path);
                else if (command == get)
                    get_command(system, file, file_path);
                else
                    std::cout << "Unrecognised command\n";

            } catch (const std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
            }
            std::cin >> command;
        }

    }

};

const char* File_System_Manager::cat    = "cat";
const char* File_System_Manager::end    = "quit";
const char* File_System_Manager::copy   = "copy";
const char* File_System_Manager::erase  = "erase";
const char* File_System_Manager::mkdir  = "mkdir";
const char* File_System_Manager::echo   = "echo";
const char* File_System_Manager::touch  = "touch";
const char* File_System_Manager::link   = "link";
const char* File_System_Manager::cut    = "cut";
const char* File_System_Manager::info   = "info";
const char* File_System_Manager::memory = "memory";
const char* File_System_Manager::inodes = "inodes";
const char* File_System_Manager::get    = "get";

#endif //_FILE_SYSTEM_FILE_SYSTEM_H
