#include "CsvWriter.h"

#include <stdexcept>

namespace balls_bins {

namespace {

std::string escapeCsvValue(const std::string& value) {
    bool needs_quotes = false;
    for (const char character : value) {
        if (character == ',' || character == '"' || character == '\n' ||
            character == '\r') {
            needs_quotes = true;
            break;
        }
    }

    if (!needs_quotes) {
        return value;
    }

    std::string escaped = "\"";
    for (const char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += "\"";

    return escaped;
}

}  // namespace

CsvWriter::CsvWriter(const std::string& path) : output_(path) {
    if (!output_) {
        throw std::runtime_error("Failed to open CSV output path: " + path);
    }
}

void CsvWriter::writeRow(const std::vector<std::string>& values) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            output_ << ',';
        }

        output_ << escapeCsvValue(values[i]);
    }

    output_ << '\n';
}

bool CsvWriter::isOpen() const {
    return output_.is_open();
}

}  // namespace balls_bins
