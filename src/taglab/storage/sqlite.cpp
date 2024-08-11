#include <SQLiteCpp/Statement.h>
#include <cstdint>
#include <taglab/storage/sqlite.h>

#include <SQLiteCpp/Transaction.h>

#include <format>
#include <numeric>
#include <ranges>
#include <tuple>

namespace vws = std::ranges::views;
using namespace taglab;
using namespace taglab::storage;

SQLiteStorage::SQLiteStorage(std::string_view path)
    : db_{path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
{
    db_.exec("CREATE TABLE Entry (id INTEGER PRIMARY KEY AUTOINCREMENT, path VARCHAR(255) NOT "
             "NULL);");
}

void SQLiteStorage::addEntries(std::vector<Entry> const &entries)
{
    auto const values = entries | vws::transform([](auto const &entry) {
                            return std::format("('{}')", entry.path.string());
                        });
    auto const valuesString = std::accumulate(
            std::next(std::cbegin(values)), std::cend(values), *std::cbegin(values),
            [](auto const &first, auto const &second) { return format("{},{}", first, second); });
    auto const command = format("INSERT INTO Entry (path) VALUES {};", valuesString);
    db_.exec(command);
}

std::vector<Entry> SQLiteStorage::entries() const
{
    auto query = SQLite::Statement{db_, "SELECT * FROM Entry"};
    auto entries = std::vector<Entry>{};
    while (query.executeStep()) {
        entries.emplace_back(query.getColumns<Entry, 2>());
    }
    return entries;
}


SQLiteStorage SQLiteStorage::inMemory()
{
    return SQLiteStorage(":memory:");
}
