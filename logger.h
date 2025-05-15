
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>

namespace Logger{

    void log(std::string info){
        std::ofstream save_file("engine-logs\\logsv3.txt", std::ios::app);
        save_file << info;
        save_file.close();
    }

} // namespace Logger


#endif