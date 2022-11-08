#ifndef _FileOperation_H_
#define _FileOperation_H_
#include <SPIFFS.h>
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

class FileOperation
{
public:
    static void initFlashFileSystem(fs::SPIFFSFS &fs)
    {
        if (!fs.begin(FORMAT_SPIFFS_IF_FAILED))
        {
            Serial.println(F("An Error has occurred while mounting SPIFFS"));
            while (1)
                ;
        }
    }

    static void listDir(fs::FS &fs, const char *dirname = "/", uint8_t levels = 0)
    {
        if (!Serial)
            return;
        Serial.printf("Listing directory: %s\r\n", dirname);

        File root = fs.open(dirname);
        if (!root)
        {
            Serial.println(F("Failed to open directory"));
            return;
        }
        if (!root.isDirectory())
        {
            Serial.println(F("Chosen path isn't a directory"));
            root.close();
            return;
        }

        File file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                Serial.print(F("  DIR : "));
                Serial.println(file.name());
                if (levels)
                {
                    listDir(fs, file.name(), levels - 1);
                }
            }
            else
            {
                Serial.print(F("  FILE: "));
                Serial.print(file.name());
                Serial.print(F("\tSIZE: "));
                Serial.println(file.size());
            }
            file.close();
            file = root.openNextFile();
        }
    }

    static String readFile(fs::FS &fs, const char *path)
    {
#ifdef DEBUGGING
        Serial.printf("Reading file: %s\r\n", path);
#endif
        File file = fs.open(path);
        if (!file || file.isDirectory())
        {
            Serial.println(F("Failed to open file for reading"));
            return String("");
        }
#ifdef DEBUGGING
        Serial.println(F("File contains:"));
#endif
        if (!file.available())
            return "";

        String acc = file.readString();
        file.close();
#ifdef DEBUGGING
        Serial.println(acc);
#endif
        return acc;
    }

    static void writeFile(fs::FS &fs, String path, String message)
    {
#ifdef DEBUGGING
        Serial.printf("Writing file: %s\r\n", path.c_str());
#endif
        File file = fs.open(path.c_str(), FILE_WRITE);
        if (!file)
        {
            Serial.println(F("Failed to open file for writing"));
            return;
        }
        if (file.print(message) || message.length() < 1)
            Serial.println(F("File written!"));
        else
            Serial.println(F("File write failed"));
        file.close();
    }

    static void appendFile(fs::FS &fs, String path, String message)
    {
#ifdef DEBUGGING
        Serial.printf("Appending to file: %s\r\n", path.c_str());
#endif
        File file = fs.open(path.c_str(), FILE_APPEND);
        if (!file)
        {
            Serial.println(F("Failed to open file for appending"));
            return;
        }
        if (file.print(message.c_str()))
            Serial.println(F("Update appended!"));
        else
            Serial.println(F("Appending failed"));
        file.close();
    }

    static void renameFile(fs::FS &fs, const char *path1, const char *path2)
    {
        Serial.printf("Renaming file %s to %s\r\n", path1, path2);
        if (fs.rename(path1, path2))
            Serial.println(F("File renamed!"));
        else
            Serial.println(F("Renaming failed"));
    }

    static void deleteFile(fs::FS &fs, const char *path)
    {
        Serial.printf("Deleting file: %s\r\n", path);
        if (fs.remove(path))
            Serial.println(F("File deleted!"));
        else
            Serial.println(F("Deletion failed"));
    }
};
#endif