#include "FileLineReadOnlyStream.h"


FileLineReadOnlyStream::FileLineReadOnlyStream(const std::string& filename)
    : filename_(filename),
    position_(0),
    is_open_(false),
    end_reached_(false)
{
    if (filename_.empty()) {
        throw std::invalid_argument("FileLineReadOnlyStream: filename is empty");
    }
}


FileLineReadOnlyStream::~FileLineReadOnlyStream() {
    if (file_.is_open()) {
        file_.close();
    }
}


void FileLineReadOnlyStream::Open() {
    if (file_.is_open()) {
        file_.close();
    }

    file_.clear();
    file_.open(filename_);

    if (!file_.is_open()) {
        throw std::runtime_error("FileLineReadOnlyStream: cannot open file");
    }

    position_ = 0;
    is_open_ = true;
    end_reached_ = false;
}


void FileLineReadOnlyStream::Close() {
    if (file_.is_open()) {
        file_.close();
    }

    is_open_ = false;
}


bool FileLineReadOnlyStream::IsEndOfStream() const {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    return end_reached_;
}


std::string FileLineReadOnlyStream::Read() {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (end_reached_) {
        throw EndOfStreamException();
    }

    std::string line;

    if (!std::getline(file_, line)) {
        end_reached_ = true;
        throw EndOfStreamException();
    }

    ++position_;

    return line;
}


int FileLineReadOnlyStream::GetPosition() const {
    return position_;
}


bool FileLineReadOnlyStream::IsCanSeek() const {
    return true;
}


int FileLineReadOnlyStream::Seek(int index) {
    if (!is_open_) {
        throw StreamIsClosedException();
    }

    if (index < 0) {
        throw StreamSeekException();
    }

    file_.clear();
    file_.seekg(0, std::ios::beg);

    if (!file_) {
        throw StreamSeekException();
    }

    position_ = 0;
    end_reached_ = false;

    std::string line;

    while (position_ < index) {
        if (!std::getline(file_, line)) {
            end_reached_ = true;
            throw StreamSeekException();
        }

        ++position_;
    }

    return position_;
}


bool FileLineReadOnlyStream::IsCanGoBack() const {
    return true;
}