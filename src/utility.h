#ifndef _FILE_SYSTEM_UTILITY_H
#define _FILE_SYSTEM_UTILITY_H

using ui     = unsigned int;
using byte   = unsigned char;
using vec_s  = std::vector<std::string>;
using vec_16 = std::vector<uint16_t>;
using vec_c  = std::vector<char>;


uint16_t read_uint16_t(std::ifstream& f);
uint16_t read_uint16_t(const vec_c& buffer, uint16_t pos);

void     write_uint16_t(std::ostream& f, uint16_t val);
byte     read_byte(std::ifstream& f);

struct Directory {

    uint16_t inode_num;     // Inode number of the directory.
    uint16_t mem_block;     // First block of the directory.
    vec_s    names;         // Name of the file
    vec_16   inodes;        // Directly mapped onto the inode number.


    explicit Directory(uint16_t inode_nr, uint16_t mem_block, const vec_c& dir_content):
                      inode_num(inode_nr), mem_block(mem_block) {

        uint16_t read = 0;

        while (read < dir_content.size()) {

            std::string s;

            while (dir_content[read] != '\0')
                s.append(1, dir_content[read++]);

            names.push_back(s);
            inodes.push_back(read_uint16_t(dir_content, ++read));

            read += 2;
        }

    }

    void add_new_file(const std::string& s, uint16_t inode) {

        for (auto const& name : names)
            if (name == s)
                throw std::runtime_error("File already exists");

        names.push_back(s);
        inodes.push_back(inode);
    }

    uint16_t get_file_inode(const std::string& s) const {

        ui idx;

        for (idx = 0; idx < names.size() && names[idx] != s; idx++) {}

        return idx == names.size() ? 0 : inodes[idx];
    }

    void print_content() const {
        for (auto const & s : names)
            std::cout << s << std::endl;
    }

    void erase_file(const std::string& s) {
        vec_s::iterator  i;
        vec_16::iterator i_1;

        for (i = names.begin(), i_1 = inodes.begin(); i != names.end() && i_1 != inodes.end(); i++, i_1++)
            if (*i == s)
                break;

        if (i == names.end())
            throw std::runtime_error("File not found");

        names.erase(i);
        inodes.erase(i_1);
    }

    vec_c get_directory_content() const {

        vec_c content;

        for (ui i = 0; i < names.size(); i++) {

            for (auto c : names[i])
                content.push_back(c);

            content.push_back('\0');
            content.push_back((char) inodes[i]);
            content.push_back((char) inodes[i] >> 8);
        }

        return content;
    }

};

struct File {

    uint16_t mem_block;
    vec_c    content;

    explicit File(uint16_t m_b, vec_c& c): mem_block(m_b), content(c) {}

    void print_content() const {

        for (auto& c : content)
            std::cout << c;

        std::cout << std::endl;
    }

    void add_to_file(const std::string& m) {

        for (auto& c : m)
            content.push_back(c);
    }

    void cut_from_file(ui to_cut) {

        if (to_cut >= content.size())
            content.clear();
        else
            content.erase(content.end() - to_cut, content.end());
    }


    vec_c get_file_content() const {
        return content;
    }

    uint16_t get_file_mem_block() const {
        return mem_block;
    }

};


uint16_t read_uint16_t(std::ifstream& f) {

    char buffer[2];
    f.read(buffer, 2);

    return ((uint16_t) buffer[1] << 8) | (byte) buffer[0];
}

uint16_t read_uint16_t(const vec_c& buffer, uint16_t pos) {
    return ((uint16_t) buffer[pos + 1] << 8) | (byte) buffer[pos];
}

vec_c convert_to_vector(char* buffer, ui size) {

    vec_c content_vec;

    for (ui i = 0; i < size; i++)
        content_vec.push_back(buffer[i]);
	
    delete [] buffer;

    return content_vec;
}

vec_c read_string(std::ifstream& f, uint16_t size) {

    char* content = new char[size];
    
    f.read(content, size);

    return convert_to_vector(content, size);
}

void write_uint16_t(std::ostream& f, uint16_t val) {

    byte my_arr[2];
    memcpy(my_arr, &val, sizeof(val));

    f.write( (const char*) my_arr, 2);
}

void write_byte(std::ofstream& f, byte val) {
    f.put(val);
}

void write_string(std::ofstream&f, const vec_c& content, uint16_t size) {

    char* buffer = new char[size];

    for (uint16_t i = 0; i < size; i++)
        buffer[i] = content[i];

    f.write(buffer, size);
    
    delete[] buffer;
}


byte read_byte(std::ifstream& f) {

    char t;
    f.read(&t, 1);

    return (byte) t;
}

vec_s path(std::string& s) {

    const std::string delimiter = "/";

    vec_s path;
    std::size_t pos;

    if (s != "/") {
        while ((pos = s.find(delimiter)) != std::string::npos) {
            path.push_back(s.substr(0, pos));
            s.erase(0, pos + delimiter.length());
        }
    }

    return path;
}

#endif //_FILE_SYSTEM_UTILITY_H
