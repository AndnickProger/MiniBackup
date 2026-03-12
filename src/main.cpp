#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include "engine.hpp"

int main(int argc, char *argv[])
{    
    if (argc < 4)
    {
        std::cerr << "Использование: " << argv[0]
                  << " <source_dir> <target_dir> <database_file> [threads]" << std::endl;
        return 1; 
    }

    try
    {        
        std::string sourcePath = argv[1]; 
        std::string targetPath = argv[2]; 
        std::string dbPath = argv[3];    
        
        size_t threads = std::thread::hardware_concurrency();
        if (argc >= 5)
        {
            threads = static_cast<size_t>(std::stoul(argv[4]));
        }
       
        backup::BackupEngine engine(dbPath, threads);

        std::cout << "--- Запуск резервного копирования ---" << std::endl;
        std::cout << "Источник: " << sourcePath << std::endl;
        std::cout << "Назначение: " << targetPath << std::endl;
        
        engine.run(sourcePath, targetPath);
        
        std::cout << "--- Бэкап успешно завершен! ---" << std::endl;
    }
    catch (const std::exception &e)
    {      
        std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {        
        std::cerr << "Неизвестная ошибка программы." << std::endl;
        return 1;
    }

    return 0; 
}
