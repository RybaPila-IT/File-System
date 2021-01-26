#include <iostream>
#include <fstream>
#include <vector>

#include "file_system.h"

int main(int argc, char** argv){

    std::ifstream input;
    std::ofstream output;

    input = std::ifstream(argv[1]);

    if (!input) {

        int size;
        std::cout << "Specify the file system size in bytes: ";
        std::cin >> size;

        if (size > 0) {

            output = std::ofstream(argv[1]);
            File_System_Manager::make_empty_file_system(output, size);
            input = std::ifstream(argv[1]);
            output.close();
        }
    }

    if (input) {

        File_System system(input);
        input.close();

        File_System_Manager::manage_file_system(system);

        output = std::ofstream(argv[1]);

        system.dump_file_system_to_file(output);
        output.close();
    }

    return 0;
}
