#include <info.h>
#include <argparse.h>

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cstring>

void LoadDictionary(FileInfo& Info){
    // Load all directories from within the root path and create a dictionary on them
    try {
        if (std::filesystem::exists(Info.RootDirectory) && std::filesystem::is_directory(Info.RootDirectory)) {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(Info.RootDirectory)) {
                uint32_t PathSize = (entry.path().string().size() - Info.RootDirectory.size()) + 1; // PLUS 1 FOR "/"
                uint32_t Size = PathSize + 25;
                uint8_t Type = TYPE_UNKOWN;

                if(std::filesystem::is_directory(entry.path())){
                    Type = TYPE_DIRECTORY;
                }
                else if(std::filesystem::is_regular_file(entry.path())){
                    Type = TYPE_FILE;
                }
                else{
                    std::cout << "Warning: File type not supported for " << entry.path().string() << std::endl;
                }

                std::string Path = entry.path().string();
                std::replace(Path.begin(), Path.end(), '\\', '/');

                // Load path to info structure
                Info.Dictionary.push_back(DictionaryInfoHeader{Size, PathSize, Type, Path, 0, 0});

                // Update header to fit new data
                Info.Header.DictionaryEnd += Size;
                Info.Header.DictionarySize += Size;
            }
        } else {
            std::cout << "Given path is not a directory or does not exist." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Error encountered: " << e.what() << std::endl;
    }
}

void LoadUpdater(FileInfo& Info){
    // Load all directories from within the root path and create a dictionary on them
    try {
        if (std::filesystem::exists(Info.RootDirectory) && std::filesystem::is_directory(Info.RootDirectory)) {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(Info.RootDirectory)) {
                // Directories dont matter
                if(!std::filesystem::is_regular_file(entry.path())){
                    continue;
                }

                // Check if we need to count towards update
                std::string RemoveUpdaterDirectory = std::string(((entry.path().string().c_str()) + Info.UpdaterDirectory.size()));
                if(!std::filesystem::exists(Info.RootDirectory + RemoveUpdaterDirectory)){
                    continue;
                }

                std::cout << "Updating: " << entry.path().string() << std::endl;

                uint32_t PathSize = (entry.path().string().size() - Info.RootDirectory.size()) + 1; // PLUS 1 FOR "/"
                uint32_t Size = PathSize + 4;

                std::string Path = entry.path().string();
                std::replace(Path.begin(), Path.end(), '\\', '/');

                // Load path to info structure
                Info.Update.push_back(DeletionInfoHeader{Size, Path});

                // Update header to fit new data
                Info.Header.DeleterEnd += Size;
                Info.Header.DeleterSize += Size;
                Info.UpdaterSize += Size;
            }
        } else {
            std::cout << "Given path is not a directory or does not exist." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Error encountered: " << e.what() << std::endl;
    }
}

// COMPRESSION 
std::vector<uint8_t> compress(uint8_t* data, size_t size) {
    if(size && data){}
    return std::vector<uint8_t>{};
}

std::vector<uint8_t> uncompress(uint8_t* data, size_t size) {
    if(size && data){}
    return std::vector<uint8_t>{};
}
// END COMPRESSION

void LoadData(FileInfo& Info){
    // For each entry in the dictionary load data
    if(Info.Compression){
        // Current Offset into data
        uint64_t Offset = 0;

        for(DictionaryInfoHeader& DInfoH : Info.Dictionary){
            // If type is directory
            if(DInfoH.type != TYPE_FILE){
                // Load different info
                DInfoH.DataOffset = 0x00;
                DInfoH.DataSize = 0x00;
                continue;
            }

            std::cout << "Compressing: " << DInfoH.FilePath << std::endl;

            // Load data and get file size
            std::ifstream file(DInfoH.FilePath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Failed to open the file: " << DInfoH.FilePath << std::endl;
                continue;
            }

            // Get file size
            uint64_t FileSize = static_cast<std::size_t>(file.tellg());
            file.seekg(0, std::ios::beg);

            if(FileSize == 0){
                DInfoH.DataOffset = 0x00;
                DInfoH.DataSize = 0x00;
                continue;
            }

            // Create data buffer for filesize
            char* data = new char[FileSize];

            // Read file data
            if (!file.read(data, FileSize)) {
                std::cerr << "Failed to read the file: " << DInfoH.FilePath << std::endl;
            }

            file.close();

            // Compression
            std::vector<uint8_t> CompressedData = compress((uint8_t*)data, FileSize);
            uint64_t NewSize = CompressedData.size();

            uint8_t* NewData = new uint8_t[NewSize];
            memcpy(NewData, CompressedData.data(), NewSize);
            delete data;

            // Now update the dictionary and load the data
            DInfoH.DataOffset = Offset;
            DInfoH.DataSize = NewSize;
            Info.Datas.push_back(NewData);

            Offset+=NewSize;
        }

        // Update Info
        Info.Header.FileContentOffset = Info.Header.DictionaryEnd;
        Info.Header.FileContentEnd = Info.Header.FileContentOffset + Offset;
    }
    else{
        // Current Offset into data
        uint64_t Offset = 0;

        for(DictionaryInfoHeader& DInfoH : Info.Dictionary){
            // If type is directory
            if(DInfoH.type != TYPE_FILE){
                // Load different info
                DInfoH.DataOffset = 0x00;
                DInfoH.DataSize = 0x00;
                continue;
            }

            // Load data and get file size
            std::ifstream file(DInfoH.FilePath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Failed to open the file: " << DInfoH.FilePath << std::endl;
                continue;
            }

            // Get file size
            uint64_t FileSize = static_cast<std::size_t>(file.tellg());
            file.seekg(0, std::ios::beg);

            if(FileSize == 0){
                DInfoH.DataOffset = 0x00;
                DInfoH.DataSize = 0x00;
                continue;
            }

            // Create data buffer for filesize
            char* data = new char[FileSize];

            // Read file data
            if (!file.read(data, FileSize)) {
                std::cerr << "Failed to read the file: " << DInfoH.FilePath << std::endl;
            }

            file.close();

            // Now update the dictionary and load the data
            DInfoH.DataOffset = Offset;
            DInfoH.DataSize = FileSize;
            Info.Datas.push_back((uint8_t*)data);

            Offset += FileSize;
        }

        // Update Info
        Info.Header.FileContentOffset = Info.Header.DictionaryEnd;
        Info.Header.FileContentEnd = Info.Header.FileContentOffset + Offset;
    }
}

bool writeFile(const std::string &filePath, uint8_t* data, uint64_t size) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open or create the file: " << filePath << std::endl;
        return false;
    }

    file.write((char*)data, size);
    if (!file) {
        std::cerr << "Failed to write to the file: " << filePath << std::endl;
        return false;
    }

    file.close();
    return true;
}

void ExportData(FileInfo& Info){
    // First create data to export

    // Calculate the size
    uint64_t TotalSize = 0;
    
    // File Header
    TotalSize += sizeof(FileHeader);

    // Updater
    for(DeletionInfoHeader& DInfoH : Info.Update){
        TotalSize += DInfoH.size;
    }

    // Dictionary + Data
    for(DictionaryInfoHeader& DInfoH : Info.Dictionary){
        TotalSize += DInfoH.size + DInfoH.DataSize;
    }

    // Allocate Data for this
    // TODO: BAD ON MEMORY EFFICIENCY!
    uint8_t* OutputData = new uint8_t[TotalSize];
    uint64_t Offset = 0;

    // Copy Header
    memcpy(OutputData + Offset, &(Info.Header), sizeof(FileHeader));
    Offset += sizeof(FileHeader);

    // Now copy Updater if used
    if(Info.Updater){
        for(DeletionInfoHeader& DInfoH : Info.Update){
            // Individual values
            ((uint32_t*)(OutputData + Offset))[0] = DInfoH.size;
            Offset+=4;

            memcpy(OutputData + Offset, ("/" + (std::string(DInfoH.FilePath.c_str() + Info.RootDirectory.size()))).c_str(), DInfoH.size - 4);
            Offset+=DInfoH.size - 4;
        }
    }

    // Now copy dictionary
    for(DictionaryInfoHeader& DInfoH : Info.Dictionary){
        // Individual values
        ((uint32_t*)(OutputData + Offset))[0] = DInfoH.size;
        Offset+=4;

        ((uint32_t*)(OutputData + Offset))[0] = DInfoH.pathSize;
        Offset+=4;

        (OutputData + Offset)[0] = DInfoH.type;
        Offset+=1;

        memcpy(OutputData + Offset, ("/" + (std::string(DInfoH.FilePath.c_str() + Info.RootDirectory.size()))).c_str(), DInfoH.pathSize);
        Offset+=DInfoH.pathSize;

        ((uint64_t*)(OutputData + Offset))[0] = DInfoH.DataOffset;
        Offset+=8;

        ((uint64_t*)(OutputData + Offset))[0] = DInfoH.DataSize;
        Offset+=8;
    }

    // Now copy data
    int Index = 0;
    for(DictionaryInfoHeader& DInfoH : Info.Dictionary){
        if(DInfoH.type != TYPE_FILE){
            continue; // Dont try load non-file data
        }

        // Get the size
        uint64_t size = DInfoH.DataSize;

        if(size == 0){
            continue; // Not included in data
        }

        std::cout << "Current Size: " << Offset << " Size needed: " << size << " Size Left: " << TotalSize - Offset << std::endl;

        // Now copy the data in
        uint8_t* data = Info.Datas[Index];
        memcpy(OutputData + Offset, data, size);
        Offset += size;

        Index++;
    }

    // Now write data buffer
    writeFile("./" + (Info.Name + ".inst"), OutputData, TotalSize);

    // Free all used memory
    delete OutputData;
    for(uint8_t* Data : Info.Datas){
        delete Data;
    }

    return;
}

int main(int argc, char** argv){
    // Start main parser
    Unified::InputParser Parser(argc, argv);

    // First see if help is requested
    if(Parser.cmdOptionExists("-h") || Parser.cmdOptionExists("--help")){
        // Print help info
        std::cout << "Hello" << std::endl;

        return 0; // Don't continue
    }

    // ARGS:
    //  - "-h" or "--help" Calls for help
    //  - "-r {DIR}" or "--root {DIR}" Root directory to be loaded 
    //  - "-o {NAME}" or "--output-name {NAME}" Name for exported file
    //  - "-u {DIR}" or "--update {DIR}" The original files to be removed
    //  - "-c" Calls for RLE compression to be active
    //  - 
    //  - 
    //  - 
    //  - 

    // Now to pull actual arguments required!
    std::string Directory = "";
    std::string UpdaterDirectory = "";
    bool Updater = false;
    std::string OutputFile = "";
    bool Compression = false; // Not required to be specified due to default value

    // Directory
    if(!Parser.getCmdOption("-r").empty()){
        Directory = Parser.getCmdOption("-r");
    }
    else if(!Parser.getCmdOption("--root").empty()){
        Directory = Parser.getCmdOption("--root");
    }
    else{
        std::cout << "No Root Directory Specified." << std::endl;
        return 0;
    }

    std::cout << "Using root directory: " << Directory << std::endl;

    // Get compression
    if(Parser.cmdOptionExists("-c")){
        Compression = true;
    }

    // Output file name
    if(!Parser.getCmdOption("-o").empty()){
        OutputFile = Parser.getCmdOption("-o");
    }
    else if(!Parser.getCmdOption("--output-name").empty()){
        OutputFile = Parser.getCmdOption("--output-name");
    }
    else{
        std::cout << "No Output Name Specified." << std::endl;
        return 0;
    }

    std::cout << "Using name: " << OutputFile << std::endl;

    // Update root name
    if(!Parser.getCmdOption("-u").empty()){
        UpdaterDirectory = Parser.getCmdOption("-u");
        Updater = true;
    }
    else if(!Parser.getCmdOption("--update").empty()){
        UpdaterDirectory = Parser.getCmdOption("--update");
        Updater = true;
    }

    // TODO: Maybe allow custom path to be used.

    FileInfo info = {Directory, UpdaterDirectory, Updater, OutputFile, Compression, {}, {}, {}, {}, 0x00};

    // If we plan on this being a updater use this
    if(info.Updater){
        LoadUpdater(info);
        std::cout << "Loaded Update Dictionary" << std::endl;
    
        info.Header.DeleterOffset = sizeof(FileHeader);
        info.Header.DeleterEnd += sizeof(FileHeader); 
    }

    // Load all the data
    LoadDictionary(info);
    std::cout << "Loaded Dictionary" << std::endl;
    LoadData(info);
    std::cout << "Loaded Data" << std::endl;

    // Finish the Header
    info.Header.DictionaryOffset = sizeof(FileHeader) + info.UpdaterSize;
    info.Header.DictionaryEnd += sizeof(FileHeader) + info.UpdaterSize;
    info.Header.Compressed = (Compression) ? 0x01 : 0x00;

    // Now export new installer
    ExportData(info);
    std::cout << "Exported to " << "./" << info.Name << ".inst" << std::endl;

    return 0;
}