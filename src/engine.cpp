#include <iostream>
#include <boost/asio/post.hpp>

#include "engine.hpp"

namespace backup
{
    BackupEngine::BackupEngine(const std::string &dataBasePath, size_t threadCount) : storage(std::make_unique<StorageManager>(dataBasePath)),
                                                                                      pool(threadCount)
    {
        std::cout << "Engine: pool is initialized with " << threadCount << " threads\n";
    }

    void BackupEngine::run(const fs::path &source, const fs::path &target)
    {
        if (!fs::exists(source))
            throw std::runtime_error("Source path does not exist");
        if (!fs::exists(target))
            fs::create_directories(target);

        for (const auto &entry : fs::recursive_directory_iterator(source))
        {
            if (entry.is_regular_file())
            {
                auto filePath = entry.path();
                boost::asio::post(pool, [this, filePath, target]()
                                  {
                                      try
                                      {
                                          this->processFile(filePath, target);
                                      }
                                      catch (const std::exception &e)
                                      {
                                          std::cerr << "Error processing " << filePath << ": " << e.what() << std::endl;
                                      }
                                  });
            }
        }
    }

    void BackupEngine::processFile(const fs::path &filePath, const fs::path &targetDir)
    {
        std::string currentHash = calculateHash(filePath);

        if (storage->isFileChanged(filePath.string(), currentHash))
        {
            fs::path output = targetDir / (filePath.filename().string() + ".zst");
            compressor.compressFile(filePath, output);
            storage->updateFileInfo(filePath.string(), currentHash);
            std::cout << "BACKUP " << filePath.filename() << " -> " << output.filename() << std::endl;
        }
        else
        {
            std::cout << "SKIP " << filePath.filename() << " (hasn't changed)" << std::endl;
        }
    }

    std::string BackupEngine::calculateHash(const fs::path &path)
    {
        auto fsize = fs::file_size(path);
        auto lwt = fs::last_write_time(path);

        return std::to_string(fsize) + "_" + std::to_string(lwt);
    }

    void BackupEngine::wait()
    {
        pool.wait(); 
    }
}