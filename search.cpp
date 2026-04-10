#include "include/search.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
/**
 * Cleans the provided `token` by returning a new string with all leading and
 * trailing punctuation removed. If the `token` contains no letters, returns
 * the empty string to indicate that it should be discarded.
 *
 * Assumes that `token` contains no spaces.
 */
string cleanToken(const string& token) {
  string cleaned;
  if (token.empty()) {
    return "";
  }
  int indexStart = 0;
  while (indexStart < token.size()) {
    if (ispunct(token[indexStart])) {
      indexStart++;
    } else {
      break;
    }
  }

  int indexEnd = token.size() - 1;
  while (indexEnd >= 0) {
    if (ispunct(token[indexEnd])) {
      indexEnd--;

    } else {
      break;
    }
  }

  bool alphaSeen = false;
  for (int i = 0; i < token.size(); i++) {
    if (isalpha(token[i])) {
      alphaSeen = true;
      break;
    }
  }
  if (!alphaSeen) return "";

  for (int i = indexStart; i <= indexEnd; i++) {
    if (isalpha(token[i])) {
      cleaned += tolower(token[i]);
    } else {
      cleaned += token[i];
    }
  }

  return cleaned;
}

set<string> gatherTokens(const string& text) {
  set<string> finalSet;

  string inLine;
  istringstream iss(text);

  while (getline(iss, inLine, ' ')) {
    if (inLine.empty()) {
      continue;
    }

    string cleaned = cleanToken(inLine);

    if (!cleaned.empty()) {
      finalSet.insert(cleaned);
    }
  }
  return finalSet;
}

int buildIndex(const string& filename, map<string, set<string>>& index) {
  ifstream inputFile(filename);
  if (!inputFile.is_open()) {
    return 0;
  }

  int page = 0;
  string url;
  string key;

  while (getline(inputFile, url)) {
    getline(inputFile, key);

    set<string> cleanedKey = gatherTokens(key);
    for (string keyToken : cleanedKey) {
      index[keyToken].insert(url);
    }

    page++;
  }

  return page;
}

set<string> findQueryMatches(const map<string, set<string>>& index,
                             const string& sentence) {
  stringstream iss(sentence);
  string word;
  set<string> finalSet;

  int countTerms = 0;
  while (iss >> word) {
    char flag = '\0';
    string uncleaned;

    if (!word.empty() && (word[0] == '-' || word[0] == '+')) {
      flag = word[0];
      for (char ch : word) {
        if (ch == '-' || ch == '+') {
          continue;
        }
        uncleaned += ch;
      }
    } else {
      uncleaned = word;
    }

    string cleaned = cleanToken(uncleaned);
    if (cleaned.empty()) {
      continue;
    }

    set<string> foundSet;

    auto it = index.find(cleaned);
    if (!(it == index.end())) {
      foundSet = it->second;
    } else {
      foundSet = {};
    }

    if (countTerms == 0 && flag == '\0') {
      countTerms++;
      finalSet = foundSet;
      continue;
    }

    if (countTerms > 0) {
      set<string> afterOperation;
      if (flag == '+') {
        set_intersection(finalSet.begin(), finalSet.end(), foundSet.begin(),
                         foundSet.end(),
                         inserter(afterOperation, afterOperation.begin()));
        finalSet = afterOperation;
      } else if (flag == '-') {
        set_difference(finalSet.begin(), finalSet.end(), foundSet.begin(),
                       foundSet.end(),
                       inserter(afterOperation, afterOperation.begin()));
        finalSet = afterOperation;
      } else {
        set_union(finalSet.begin(), finalSet.end(), foundSet.begin(),
                  foundSet.end(),
                  inserter(afterOperation, afterOperation.begin()));
        finalSet = afterOperation;
      }
      countTerms++;
    }
  }
  return finalSet;
}

void searchEngine(const string& filename) {
  map<string, set<string>> index;

  ifstream infile(filename);
  if (!infile.is_open()) {
    cout << "Invalid filename.\n";
  }

  cout << "Stand by while building index..." << endl;
  int pageNum = buildIndex(filename, index);

  cout << "Indexed " << pageNum << " pages containing " << index.size()
       << " unique terms" << endl;
  bool exit = true;
  while (exit) {
    cout << "Enter query sentence (press enter to quit): ";

    string sentence;
    getline(cin, sentence);

    if (sentence.empty()) {
      cout << "Thank you for searching!" << endl;
      exit = false;
      continue;
    }

    set<string> matches = findQueryMatches(index, sentence);
    cout << "Found " << matches.size() << " matching pages" << endl;

    for (const string& url : matches) {
      cout << url << endl;
    }
  }
}
