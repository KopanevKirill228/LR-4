#pragma once

#include "ReadOnlyStream.h"

#include <fstream>
#include <string>


class FileLineReadOnlyStream : public ReadOnlyStream<std::string> {
private:
    std::string filename_;
    std::ifstream file_;

    std::string current_line_;
    bool has_current_line_;

    int position_;
    bool is_open_;

    void ReadNextLine();

public:
    explicit FileLineReadOnlyStream(const std::string& filename);
    ~FileLineReadOnlyStream() override;

    void Open() override;
    void Close() override;

    bool IsEndOfStream() const override;
    std::string Read() override;

    int GetPosition() const override;

    bool IsCanSeek() const override;
    int Seek(int index) override;

    bool IsCanGoBack() const override;
};