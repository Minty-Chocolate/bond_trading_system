#ifndef BOND_IOFILECONNECTOR_HPP
#define BOND_IOFILECONNECTOR_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../base/soa.hpp"

// ------------- Declaration: InputFileConnector -------------

template <typename K, typename V>
class InputFileConnector : public Connector<V> {
private:
  std::string filePath;

protected:
  Service<K, V> *connectedService;

public:
  InputFileConnector(const std::string &filePath, Service<K, V> *connectedService);
  void Publish(V &data) override;
  void read();
  virtual void parse(std::string line) = 0;
};

// ------------- Declaration: OutputFileConnector -------------

template <typename V>
class OutputFileConnector : public Connector<V> {
private:
  std::string filePath;

  void append(std::string line, bool newFile);

public:
  explicit OutputFileConnector(const std::string &filePath);
  void Publish(V &data) override;
  virtual std::string toString(V &data) = 0;
};

// ------------- Declaration: Utility Functions -------------

std::vector<std::string> splitString(std::string input, char delim);
double fractionalToDouble(std::string price);

// ------------- Definition: InputFileConnector -------------

template <typename K, typename V>
InputFileConnector<K, V>::InputFileConnector(const std::string &filePath, Service<K, V> *connectedService)
    : filePath(filePath), connectedService(connectedService) {}

template <typename K, typename V>
void InputFileConnector<K, V>::Publish(V &data) {
  // No-op for InputFileConnector
}

template <typename K, typename V>
void InputFileConnector<K, V>::read() {
  std::ifstream inFile(filePath);
  if (!inFile) {
    throw std::runtime_error("Unable to open file: " + filePath);
  }
  std::string line;
  while (std::getline(inFile, line)) {
    parse(line);
  }
}

// ------------- Definition: OutputFileConnector -------------

template <typename V>
OutputFileConnector<V>::OutputFileConnector(const std::string &filePath)
    : filePath(filePath) {
      append("", true);
    }

template <typename V>
void OutputFileConnector<V>::Publish(V &data) {
  append(toString(data), false);
}

template <typename V>
void OutputFileConnector<V>::append(std::string line, bool clearFile) {
  std::ofstream outFile;
  outFile.open(filePath, clearFile ? std::ios_base::out : std::ios_base::app);
  if (!outFile) {
    throw std::runtime_error("Unable to open file: " + filePath);
  }
  if (line != "") 
    outFile << line << std::endl;
  outFile.close();
}

// ------------- Definition: Utility Functions -------------

std::vector<std::string> splitString(std::string input, char delim) {
  std::stringstream ss(input);
  std::string word;
  std::vector<std::string> result;
  while (std::getline(ss, word, delim)) {
    result.push_back(word);
  }
  return result;
}

double fractionalToDouble(std::string price) {
  auto split = splitString(price, '-');
  if (split.size() != 2) {
    return 0.0;
  }
  return std::stod(split[0]) +
         std::stod(split[1].substr(0, 2)) / 32.0 +
         ((split[1][2] == '+') ? 4 : (split[1][2] - '0')) / 256.0;
}
#endif