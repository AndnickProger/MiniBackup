#include <stdexcept>
#include <iostream>
#include <memory>

#include "storage.hpp"

namespace backup
{
    using SqliteStmtPtr = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

    StorageManager::StorageManager(const std::string &dateBasePath)
    {
        if (sqlite3_open(dateBasePath.c_str(), &dataBase) != SQLITE_OK)
        {
            throw std::runtime_error("Can not open database: " + std::string(sqlite3_errmsg(dataBase)));
        }
        initDatabase();
    }

    StorageManager::~StorageManager()
    {
        if (dataBase)
            sqlite3_close(dataBase);
    }

    bool StorageManager::isFileChanged(const std::string &path, const std::string &currentHash)
    {
        const char *sql = "SELECT hash FROM files WHERE path = ?;";
        sqlite3_stmt *rawStmt{};

        if (sqlite3_prepare_v2(dataBase, sql, -1, &rawStmt, nullptr) != SQLITE_OK)
        {
            return true;
        }

        SqliteStmtPtr stmt(rawStmt, &sqlite3_finalize);

        if (sqlite3_bind_text(stmt.get(), 1, path.c_str(), -1, SQLITE_STATIC) != SQLITE_OK)
        {
            return true;
        }

        bool changed = true;

        if (sqlite3_step(stmt.get()) == SQLITE_ROW)
        {
            const char *db_hash_ptr = reinterpret_cast<const char *>(sqlite3_column_text(stmt.get(), 0));

            if (db_hash_ptr && currentHash == db_hash_ptr)
            {
                changed = false;
            }
        }

        return changed;
    }

    void StorageManager::updateFileInfo(const std::string &path, const std::string &newHash)
    {
        const char *sql = "INSERT OR REPLACE INTO files (path, hash, last_backup) VALUES (?, ?, CURRENT_TIMESTAMP);";
        sqlite3_stmt *rawStmt{};

        if (sqlite3_prepare_v2(dataBase, sql, -1, &rawStmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "DataBase ERROR: Prepare failed: " << sqlite3_errmsg(dataBase) << std::endl;
            return;
        }

        SqliteStmtPtr stmt(rawStmt, &sqlite3_finalize);

        sqlite3_bind_text(stmt.get(), 1, path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 2, newHash.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt.get()) != SQLITE_DONE)
        {
            std::cerr << "DataBase ERROR: Execution failed: " << sqlite3_errmsg(dataBase) << std::endl;
        }
    }

    void StorageManager::initDatabase()
    {
        const char *sql = "CREATE TABLE IF NOT EXISTS files ("
                          "path TEXT PRIMARY KEY, "
                          "hash TEXT NOT NULL, "
                          "last_backup DATETIME DEFAULT CURRENT_TIMESTAMP);";

        char *errorMessage{};
        if (sqlite3_exec(dataBase, sql, nullptr, nullptr, &errorMessage) != SQLITE_OK)
        {
            std::string error = errorMessage;
            sqlite3_free(errorMessage);
            throw std::runtime_error("SQL ERROR: " + error);
        }
    }
}
