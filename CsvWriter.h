#ifndef BALLS_BINS_CSV_WRITER_H
#define BALLS_BINS_CSV_WRITER_H

#include <fstream>
#include <string>
#include <vector>

namespace balls_bins {

class CsvWriter {
public:
    explicit CsvWriter(const std::string& path);

    void writeRow(const std::vector<std::string>& values);
    bool isOpen() const;

private:
    std::ofstream output_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_CSV_WRITER_H
