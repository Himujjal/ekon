#ifndef TEST_H
#define TEST_H
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static int SuccCount = 0;
static int FailCount = 0;

// Check which parts of the program failed
static void CheckRet(const char *func, unsigned int line, bool ret) {
  if (ret == false) {
    // print in red color
    std::cout << "\033[31m" << func << "(" << line << "):fail"
              << "\033[37m" << std::endl;
    ++FailCount;
  } else {
    ++SuccCount;
  }
}

// Print How many tests passed and how many failed?
static void PrintResult() {
  std::cout << "\033[32m"
            << "SuccCount:" << SuccCount << "\033[37m" << std::endl;
  if (FailCount == 0)
    std::cout << "\033[32m"
              << "FailCount:" << FailCount << "\033[37m" << std::endl;
  else
    std::cout << "\033[31m"
              << "FailCount:" << FailCount << "\033[37m" << std::endl;
}

// Get the current time as milliseconds
static uint64_t Now() {
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}

// Read the file from path and then return the contents as a string
static std::string Read(const std::string &path) {
  std::ifstream file_stream{path};

  if (file_stream.fail())
    std::cout << "failed to read: " << path << std::endl;

  std::ostringstream str_stream{};
  file_stream >> str_stream.rdbuf();

  if (file_stream.fail() && !file_stream.eof())
    std::cout << "failed to read" << std::endl;

  return str_stream.str();
}

// Print the string
static void printStr(const std::string &s, int len) {
  printf("%s", s.c_str());
  for (int i = s.size(); i < len; ++i) {
    printf(" ");
  }
  printf("\t");
}

// Print the String for long long values
static void printStr(unsigned long long n, int len) {
  std::stringstream ss;
  ss << n;
  std::string s;
  ss >> s;
  printStr(s, len);
}

class Test {
public:
  // initialize constructor in case of a single file is mentioned
  Test(const std::string &name, const std::string &path) {
    mName = name;
    mPath = path;

    std::string ekon = Read(path);
    mEkons.push_back(ekon);

    mParseTime = 0;
    mStringifyTime = 0;
    mAllTime = 0;

    mParseStatus = true;
    mStringifyStatus = true;
    mAllStatus = true;
  }
  // initialize constructor in case of multiple EKON files
  Test(const std::string &name, const std::vector<std::string> &ekons) {
    mName = name;
    mPath = "nofile";

    mEkons = ekons;

    mParseTime = 0;
    mStringifyTime = 0;
    mAllTime = 0;

    mParseStatus = true;
    mStringifyStatus = true;
    mAllStatus = true;
  }
  // parse a cstring. return false if error
  virtual bool Parse(const char *ekon, unsigned long long *ms) {
    mParseStatus = false;
    return false;
  }
  // parse and then stringify an ekon. return false if error
  virtual bool Stringify(const char *ekon, unsigned long long *ms) {
    mStringifyStatus = false;
    return false;
  }
  // Do parse and then stringification. return false if error
  virtual bool All(const char *ekon, unsigned long long *ms) {
    mAllStatus = false;
    return false;
  }
  // Test all Parsing
  bool ParseTest() {
    unsigned long long ms = 0;
    for (int i = 0; i < mEkons.size(); ++i) {
      if (Parse(mEkons[i].c_str(), &ms) == false) {
        mParseStatus = false;
      }
      mParseTime += ms;
    }

    return true;
  }
  // Test all Stringification
  bool StringifyTest() {
    unsigned long long ms = 0;
    for (int i = 0; i < mEkons.size(); ++i) {
      if (Stringify(mEkons[i].c_str(), &ms) == false) {
        mStringifyStatus = false;
      }
      mStringifyTime += ms;
    }

    return true;
  }
  // Test all Tests
  bool AllTest() {
    unsigned long long ms = 0;
    for (int i = 0; i < mEkons.size(); ++i) {
      if (All(mEkons[i].c_str(), &ms) == false) {
        mAllStatus = false;
      }
      mAllTime += ms;
    }

    return true;
  }
  // print the name of the file
  void printName(const std::string &path) {
    std::string name;
    for (int i = 0; i < path.size(); ++i) {
      if (path[i] == '/') {
        name = "";
      } else {
        name += path[i];
      }
    }
    printStr(name, 32);
  }
  void PrintParse() {
    printStr(mName, 32);
    printStr("Parse", 32);
    printName(mPath);
    if (mParseStatus)
      printStr(mParseTime, 32);
    else
      printStr("Fail", 32);
    printf("\n");
  }
  void printStringify() {
    printStr(mName, 32);
    printStr("Stringify", 32);
    printName(mPath);
    if (mStringifyStatus)
      printStr(mStringifyTime, 32);
    else
      printStr("Fail", 32);
    printf("\n");
  }
  void PrintAll() {
    printStr(mName, 32);
    printStr("All", 32);
    printName(mPath);
    if (mAllStatus)
      printStr(mAllTime, 32);
    else
      printStr("Fail", 32);
    printf("\n");
  }
  std::string Name() { return mName; }

private:
  std::string mName;
  std::vector<std::string> mEkons;
  std::string mPath;
  unsigned long long mParseTime;
  unsigned long long mStringifyTime;
  unsigned long long mAllTime;
  bool mParseStatus;
  bool mStringifyStatus;
  bool mAllStatus;
};
#endif
