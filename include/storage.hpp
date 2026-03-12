#pragma once

#include <string>
#include <sqlite3.h>

namespace backup
{
    class StorageManager
    {
    public:
        explicit StorageManager(const std::string &dateBasePath);

        ~StorageManager();

        StorageManager(const StorageManager &) = delete;

        StorageManager &operator=(const StorageManager &) = delete;

        bool isFileChanged(const std::string &path, const std::string &currentHash);

        void updateFileInfo(const std::string &path, const std::string &newHash);

    private:
        sqlite3 *dataBase = nullptr;

        void initDatabase();
    };
}