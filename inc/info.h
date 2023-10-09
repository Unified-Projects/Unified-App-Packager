#pragma once
#include <string>
#include <vector>

#define TYPE_DIRECTORY 0x00
#define TYPE_FILE      0x01
#define TYPE_UNKOWN    0xFF

struct FileHeader{
    // Basic Info
    char header[8] = "UNIFIED";
    uint8_t Revision = 0;

    // Locational Info
    uint64_t DeleterOffset;
    uint64_t DeleterEnd;
    uint64_t DeleterSize;

    uint64_t DictionaryOffset;
    uint64_t DictionaryEnd;
    uint64_t DictionarySize;

    uint64_t FileContentOffset;
    uint64_t FileContentEnd;

    // Configuration
    uint8_t Compressed = 0x00; // 0x01 for RLE true (Maybe add typing in the future)
} __attribute__((__packed__));

struct DeletionInfoHeader{
    uint32_t size;
    std::string FilePath;
};

struct DictionaryInfoHeader{
    uint32_t size;
    uint32_t pathSize;
    uint8_t type;
    std::string FilePath;
    uint64_t DataOffset;
    uint64_t DataSize;
};

struct FileInfo
{
    std::string RootDirectory;
    std::string UpdaterDirectory;
    bool Updater = false;
    std::string Name;
    bool Compression;

    FileHeader Header;
    std::vector<DeletionInfoHeader> Update;
    std::vector<DictionaryInfoHeader> Dictionary;
    std::vector<uint8_t*>Datas;

    uint64_t UpdaterSize = 0;
};