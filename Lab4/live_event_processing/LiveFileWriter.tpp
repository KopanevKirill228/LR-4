#pragma once


inline void LiveFileWriter::Run(
    const std::string& sourceFilename,
    const std::string& liveFilename,
    int delayMilliseconds,
    bool clearLiveFile,
    std::ostream& out
) {
    if (delayMilliseconds < 0) {
        throw std::invalid_argument("LiveFileWriter: delay must be non-negative");
    }

    std::ifstream source(sourceFilename);

    if (!source.is_open()) {
        throw std::runtime_error("LiveFileWriter: cannot open source file");
    }

    if (clearLiveFile) {
        std::ofstream clear(liveFilename, std::ios::trunc);

        if (!clear.is_open()) {
            throw std::runtime_error("LiveFileWriter: cannot create live file");
        }
    }

    std::ofstream live(liveFilename, std::ios::app);

    if (!live.is_open()) {
        throw std::runtime_error("LiveFileWriter: cannot open live file for writing");
    }

    std::string line;

    while (std::getline(source, line)) {
        live << line << "\n";
        live.flush();

        out << "  [writer] " << line << "\n";

        std::this_thread::sleep_for(
            std::chrono::milliseconds(delayMilliseconds)
        );
    }

    source.close();
    live.close();

    out << "  [writer] source file ended\n";
}