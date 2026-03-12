#include <boost/filesystem/fstream.hpp>
#include <vector>
#include <memory>
#include <stdexcept>
#include <zstd.h>

#include "compressor.hpp"

namespace backup
{
    void Compressor::compressFile(const fs::path &source, const fs::path &destination)
    {
        fs::ifstream inFile(source, std::ios::binary);
        fs::ofstream outFile(destination, std::ios::binary);

        if (!inFile || !outFile)
        {
            throw std::runtime_error("Compressor ERROR: Failed to open files for path: " + source.string());
        }

        std::unique_ptr<ZSTD_CCtx, decltype(&ZSTD_freeCCtx)> cctx(ZSTD_createCCtx(), &ZSTD_freeCCtx);
        if (!cctx)
        {
            throw std::runtime_error("Zstd ERROR: Failed to create CCtx");
        }

        std::vector<char> inBuffer(ZSTD_CStreamInSize());
        std::vector<char> outBuffer(ZSTD_CStreamOutSize());

        while (inFile.read(inBuffer.data(), inBuffer.size()) ||
               inFile.gcount() > 0)
        {
            ZSTD_inBuffer input = {inBuffer.data(), static_cast<size_t>(inFile.gcount()), 0};
            while (input.pos < input.size)
            {
                ZSTD_outBuffer output = {outBuffer.data(), outBuffer.size(), 0};
                const size_t ret = ZSTD_compressStream2(cctx.get(), &output, &input, ZSTD_e_continue);

                if (ZSTD_isError(ret))
                {
                    throw std::runtime_error(std::string("ZSTD ERROR: ") + ZSTD_getErrorName(ret));
                }
                outFile.write(outBuffer.data(), output.pos);
            }
        }

        ZSTD_outBuffer output = {outBuffer.data(), outBuffer.size(), 0};
        ZSTD_inBuffer lastInput = {nullptr, 0, 0};

        const size_t finished = ZSTD_compressStream2(cctx.get(), &output, &lastInput, ZSTD_e_end);
        if(ZSTD_isError(finished))
        {
            throw std::runtime_error("ZSTD ERROR: Failed to finish stream");
        }
        outFile.write(outBuffer.data(), output.pos);
    }
}