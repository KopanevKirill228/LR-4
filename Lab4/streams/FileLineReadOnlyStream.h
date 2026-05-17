#pragma once

#include "ReadOnlyStream.h"
#include "StreamExceptions.h"

#include <fstream>
#include <string>
#include <stdexcept>


class FileLineReadOnlyStream : public ReadOnlyStream<std::string> {
private:
    std::string filename_;
    std::ifstream file_;

    int position_;
    bool is_open_;
    bool end_reached_;

public:
    FileLineReadOnlyStream(const std::string& filename);

    FileLineReadOnlyStream(const FileLineReadOnlyStream& other) = delete;
    FileLineReadOnlyStream& operator=(const FileLineReadOnlyStream& other) = delete;

    ~FileLineReadOnlyStream();

    void Open() override;
    void Close() override;

    bool IsEndOfStream() const override;
    std::string Read() override;

    int GetPosition() const override;

    bool IsCanSeek() const override;
    int Seek(int index) override;

    bool IsCanGoBack() const override;
};