#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>


class LiveFileWriter {
public:
    static void Run(
        const std::string& sourceFilename,
        const std::string& liveFilename,
        int delayMilliseconds,
        bool clearLiveFile,
        std::ostream& out
    );
};

#include "LiveFileWriter.tpp"