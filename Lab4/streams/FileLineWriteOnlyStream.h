#pragma once

#include "WriteOnlyStream.h"
#include "StreamExceptions.h"

#include <fstream>
#include <string>
#include <stdexcept>


class FileLineWriteOnlyStream : public WriteOnlyStream<std::string> {
private:
    std::string filename_;
    std::ofstream file_;

    int position_;
    bool is_open_;
    bool append_mode_;

public:
    FileLineWriteOnlyStream(const std::string& filename, bool appendMode = false);

    FileLineWriteOnlyStream(const FileLineWriteOnlyStream& other) = delete;
    FileLineWriteOnlyStream& operator=(const FileLineWriteOnlyStream& other) = delete;

    ~FileLineWriteOnlyStream();

    void Open() override;
    void Close() override;

    int GetPosition() const override;
    int Write(const std::string& item) override;
};