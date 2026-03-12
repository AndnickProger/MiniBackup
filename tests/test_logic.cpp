#define BOOST_TEST_MODULE EngineTests
#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#include "engine.hpp"

namespace fs = boost::filesystem;

struct BackupFixture {
    fs::path test_dir = "./test_env";
    fs::path src = test_dir / "src";
    fs::path dst = test_dir / "dst";
    fs::path db = test_dir / "test.db";

    BackupFixture() {
        fs::create_directories(src);
        fs::create_directories(dst);
    }

    ~BackupFixture() {
        fs::remove_all(test_dir);
    }

    void create_dummy_file(const std::string& name, const std::string& content) {
        std::ofstream ofs(src / name);
        ofs << content;
        ofs.close();
    }
};

BOOST_FIXTURE_TEST_SUITE(EngineTestSuite, BackupFixture)

BOOST_AUTO_TEST_CASE(test_full_backup_cycle)
{   
    create_dummy_file("data.txt", "Hello RuBackup!");    
   
    {
        backup::BackupEngine engine(db.string(), 2);
        engine.run(src, dst);
        engine.wait();
    } 
   
    BOOST_CHECK(fs::exists(dst / "data.txt.zst"));
    BOOST_CHECK(fs::file_size(dst / "data.txt.zst") > 0);
}

BOOST_AUTO_TEST_CASE(test_incremental_skip)
{
    create_dummy_file("stable.txt", "Constant content");
   
    {
        backup::BackupEngine engine(db.string(), 1);
        engine.run(src, dst);
        engine.wait();
    }

    auto first_backup_time = fs::last_write_time(dst / "stable.txt.zst");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        backup::BackupEngine engine(db.string(), 1);
        engine.run(src, dst);
        engine.wait();
    }
    
    auto second_backup_time = fs::last_write_time(dst / "stable.txt.zst");
    BOOST_CHECK(first_backup_time == second_backup_time);
}

BOOST_AUTO_TEST_SUITE_END()
