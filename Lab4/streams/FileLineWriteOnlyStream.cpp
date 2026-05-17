#include "FileLineWriteOnlyStream.h"


FileLineWriteOnlyStream::FileLineWriteOnlyStream(const std::string& filename, bool appendMode)
    : filename_(filename),
    position_(0),
    is_open_(false),
    append_mode_(appendMode)
{
    if (filename_.empty()) {
        throw std::invalid_argument("FileLineWriteOnlyStream: filename is empty");
    }
}


FileLineWriteOnlyStream::~FileLineWriteOnlyStream() {
    if (file_.is_open()) {
        file_.close();
    }
}


void FileLineWriteOnlyStream::Open() {
    if (file_.is_open()) {
        file_.close();
    }

    file_.clear();

    if (append_mode_) {
        file_.open(filename_, std::ios::out | std::ios::app);
    }
    else {
        file_.open(filename_, std::ios::out | std::ios::trunc);
    }

    if (!file_.is_open()) {
        throw std::runtime_error("FileLineWriteOnlyStream: cannot open file");
    }

    position_ = 0;
    is_open_ = true;
}


void FileLineWriteOnlyStream::Close() {
    if (file_.is_open()) {
        file_.close();
    }

    is_open_ = false;
}


int FileLineWriteOnlyStream::GetPosition() const {
    return position_;
}


int FileLineWriteOnlyStream::Write(const std::string& item) {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    file_ << item << '\n';

    if (!file_) {
        throw std::runtime_error("FileLineWriteOnlyStream: write error");
    }

    ++position_;

    return position_;
}