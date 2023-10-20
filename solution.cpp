#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

std::string calculateSHA256(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::binary);
  if (file) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    char buffer[4096];
    while (!file.eof()) {
      file.read(buffer, sizeof(buffer));
      SHA256_Update(&sha256, buffer, file.gcount());
    }
    SHA256_Final(hash, &sha256);

    std::string result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      result += hash[i];
    }
    return result;
  }
  return "";
}

void compareDirectories(const std::string &dirPath1,
                        const std::string &dirPath2,
                        double similarityThreshold) {
  if (!fs::exists(dirPath1) || !fs::exists(dirPath2)) {
    std::cout << "Одна из указанных директорий не существует. Пожалуйста, "
                 "проверьте пути."
              << std::endl;
    return;
  }

  std::unordered_map<std::string, std::string> fileHashes1;
  std::unordered_map<std::string, std::string> fileHashes2;

  for (const auto &entry : fs::directory_iterator(dirPath1)) {
    if (fs::is_regular_file(entry.path())) {
      const std::string fileName = entry.path().filename();
      const std::string filePath = entry.path().string();
      fileHashes1[fileName] = calculateSHA256(filePath);
    }
  }

  for (const auto &entry : fs::directory_iterator(dirPath2)) {
    if (fs::is_regular_file(entry.path())) {
      const std::string fileName = entry.path().filename();
      const std::string filePath = entry.path().string();
      fileHashes2[fileName] = calculateSHA256(filePath);
    }
  }

  for (const auto &entry1 : fileHashes1) {
    const std::string &fileName1 = entry1.first;
    const std::string &hash1 = entry1.second;

    for (const auto &entry2 : fileHashes2) {
      const std::string &fileName2 = entry2.first;
      const std::string &hash2 = entry2.second;

      if (hash1 == hash2) {
        std::cout << "Идентичны: " << dirPath1 << "/" << fileName1 << " - "
                  << dirPath2 << "/" << fileName2 << std::endl;
      } else {
        std::ifstream file1(dirPath1 + "/" + fileName1,
                            std::ios::binary | std::ios::ate);
        std::ifstream file2(dirPath2 + "/" + fileName2,
                            std::ios::binary | std::ios::ate);

        if (file1 && file2) {
          std::streamsize fileSize1 = file1.tellg();
          std::streamsize fileSize2 = file2.tellg();

          if (fileSize1 > 0 && fileSize2 > 0) {
            size_t commonBytes = 0;
            file1.seekg(0);
            file2.seekg(0);

            char buffer1[4096];
            char buffer2[4096];

            while (file1 && file2) {
              file1.read(buffer1, sizeof(buffer1));
              file2.read(buffer2, sizeof(buffer2));

              size_t readSize1 = file1.gcount();
              size_t readSize2 = file2.gcount();
              size_t compareSize = std::min(readSize1, readSize2);

              for (size_t i = 0; i < compareSize; i++) {
                if (buffer1[i] == buffer2[i]) {
                  commonBytes++;
                }
              }
            }

            double similarity = static_cast<double>(commonBytes) /
                                std::max(fileSize1, fileSize2) * 100.0;

            if (similarity >= similarityThreshold) {
              std::cout << "Похожи: " << dirPath1 << "/" << fileName1 << " - "
                        << dirPath2 << "/" << fileName2 << " - " << similarity
                        << "%" << std::endl;
            }
          }
        }
      }
    }
  }

  for (const auto &entry1 : fileHashes1) {
    const std::string &fileName1 = entry1.first;
    if (fileHashes2.find(fileName1) == fileHashes2.end()) {
      std::cout << "Файл " << dirPath1 << "/" << fileName1 << " отсутствует в "
                << dirPath2 << std::endl;
    }
  }

  for (const auto &entry2 : fileHashes2) {
    const std::string &fileName2 = entry2.first;
    if (fileHashes1.find(fileName2) == fileHashes1.end()) {
      std::cout << "Файл " << dirPath2 << "/" << fileName2 << " отсутствует в "
                << dirPath1 << std::endl;
    }
  }
}

int main() {
  std::string directory1, directory2;
  double similarityThreshold;

  std::cout << "Введите путь к первой директории: ";
  std::cin >> directory1;
  std::cout << "Введите путь ко второй директории: ";
  std::cin >> directory2;
  std::cout << "Введите порог сходства (в процентах): ";
  std::cin >> similarityThreshold;

  compareDirectories(directory1, directory2, similarityThreshold);

  return 0;
}
