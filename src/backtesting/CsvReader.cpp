#include "backtesting/CsvReader.hpp"

// Gives std::ifstream for reading files from disk.
#include <fstream>
// Gives std::ostringstream for building clear error messages.
#include <sstream>
// Gives std::runtime_error for throwing exceptions when the CSV is invalid.
#include <stdexcept>
// Gives std::move for efficiently moving vectors into PriceTable.
#include <utility>
// Imports std::vector because we build vectors of strings and PriceRow objects.
#include <vector>

namespace {

// Remove simple leading/trailing spaces from a CSV field.
// yfinance does not add spaces, but trimming makes the reader safer for hand-edited files.
std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Split one CSV line on commas.
// This simple parser is enough for our generated price file because the fields
// are plain dates, tickers, and numbers without quoted commas.
std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }

    return fields;
}

// Convert one price field from text to double and provide a helpful error if it fails.
double parse_price(const std::string& text, std::size_t line_number, const std::string& asset) {
    try {
        std::size_t parsed_characters = 0;
        const double price = std::stod(text, &parsed_characters);

        // parsed_characters must cover the whole field. Otherwise a value like
        // "123abc" would partially parse as 123, which is not a clean price.
        if (parsed_characters != text.size()) {
            throw std::invalid_argument("extra characters after price");
        }

        return price;
    } catch (const std::exception&) {
        std::ostringstream message;
        message << "Invalid price for " << asset << " on CSV line " << line_number << ".";
        throw std::runtime_error(message.str());
    }
}

} // namespace

// Read a CSV file and convert it into the same PriceTable object used by BacktestEngine.
PriceTable CsvReader::read_price_table(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open CSV file: " + path);
    }

    std::string header_line;
    if (!std::getline(file, header_line)) {
        throw std::runtime_error("CSV file is empty: " + path);
    }

    const std::vector<std::string> header_fields = split_csv_line(header_line);
    if (header_fields.size() < 2) {
        throw std::runtime_error("CSV file must have a date column and at least one asset column.");
    }
    if (header_fields[0] != "date") {
        throw std::runtime_error("First CSV column must be named date.");
    }

    // The header after the date column becomes the asset list.
    std::vector<std::string> assets;
    assets.reserve(header_fields.size() - 1);
    for (std::size_t i = 1; i < header_fields.size(); ++i) {
        if (header_fields[i].empty()) {
            throw std::runtime_error("CSV header contains an empty asset name.");
        }
        assets.push_back(header_fields[i]);
    }

    std::vector<PriceRow> rows;
    std::string line;
    std::size_t line_number = 1;

    while (std::getline(file, line)) {
        ++line_number;

        // Skip blank lines so one accidental empty line at the end does not break the demo.
        if (trim(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = split_csv_line(line);
        if (fields.size() != header_fields.size()) {
            std::ostringstream message;
            message << "CSV line " << line_number << " has " << fields.size()
                    << " columns, expected " << header_fields.size() << ".";
            throw std::runtime_error(message.str());
        }
        if (fields[0].empty()) {
            std::ostringstream message;
            message << "CSV line " << line_number << " has an empty date.";
            throw std::runtime_error(message.str());
        }

        PriceRow row;
        row.date = fields[0];
        row.prices.reserve(assets.size());

        for (std::size_t i = 1; i < fields.size(); ++i) {
            row.prices.push_back(parse_price(fields[i], line_number, assets[i - 1]));
        }

        rows.push_back(std::move(row));
    }

    // PriceTable performs the final validation: at least two rows, positive prices,
    // and the same number of prices in each row.
    return PriceTable(std::move(assets), std::move(rows));
}
