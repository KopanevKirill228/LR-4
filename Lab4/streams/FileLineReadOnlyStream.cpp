#include "FileLineReadOnlyStream.h"

#include <stdexcept>


FileLineReadOnlyStream::FileLineReadOnlyStream(const std::string& filename)
    : filename_(filename),
    file_(),
    current_line_(""),
    has_current_line_(false),
    position_(0),
    is_open_(false) {
}


FileLineReadOnlyStream::~FileLineReadOnlyStream() {
    if (is_open_) {
        Close();
    }
}


void FileLineReadOnlyStream::ReadNextLine() {
    if (!std::getline(file_, current_line_)) {
        current_line_ = "";
        has_current_line_ = false;
    }
    else {
        has_current_line_ = true;
    }
}


void FileLineReadOnlyStream::Open() {
    if (is_open_) {
        Close();
    }

    file_.open(filename_);

    if (!file_.is_open()) {
        throw std::runtime_error("FileLineReadOnlyStream: cannot open file");
    }

    position_ = 0;
    is_open_ = true;

    ReadNextLine();
}


void FileLineReadOnlyStream::Close() {
    if (file_.is_open()) {
        file_.close();
    }

    current_line_ = "";
    has_current_line_ = false;
    position_ = 0;
    is_open_ = false;
}


bool FileLineReadOnlyStream::IsEndOfStream() const {
    if (!is_open_) {
        return true;
    }

    return !has_current_line_;
}


std::string FileLineReadOnlyStream::Read() {
    if (!is_open_) {
        throw std::logic_error("FileLineReadOnlyStream: stream is closed");
    }

    if (IsEndOfStream()) {
        throw std::out_of_range("FileLineReadOnlyStream: end of stream");
    }

    std::string result = current_line_;

    ++position_;

    ReadNextLine();

    return result;
}


int FileLineReadOnlyStream::GetPosition() const {
    return position_;
}


bool FileLineReadOnlyStream::IsCanSeek() const {
    return false;
}


int FileLineReadOnlyStream::Seek(int) {
    throw std::logic_error("FileLineReadOnlyStream: seek is not supported");
}


bool FileLineReadOnlyStream::IsCanGoBack() const {
    return false;
}