#include "TableS.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <stdexcept>

namespace
{
  const std::string MARKUP_ITALIC("''");

  // return a copy of s with all leading and trailing whitespace removed
  std::string Strip(const std::string& s)
  {
      std::string::const_iterator sIter(s.begin());
      std::string::const_reverse_iterator sRiter(s.rbegin());
      while (std::isspace(*sIter))  ++sIter;
      // special case: return empty string if s is empty or all whitespace
      if (sIter == s.end()) return std::string();

      while (std::isspace(*sRiter)) ++sRiter;
      return std::string(sIter, sRiter.base());
  }

  typedef std::vector<std::string> TokenListT;
  // split string 's' into a vector of tokens, by delimiter string 'delim'
  // delimiter at string start will be interpreted as being preceded by an empty token
  // delimiter at string end will be interpreted as being followed by an empty token
  // all tokens will be stripped of leading/trailing whitepsace
  // empty string will result in a single empty token
  TokenListT Split(const std::string& s, const std::string& delim)
  {
    TokenListT tList;
    std::size_t delimStart(0);
    std::size_t tokenStart(0);
    while (delimStart != std::string::npos)
    {
      // find next delimiter start
      delimStart = s.find(delim, tokenStart);
      // if not found, snap off the rest of the string as the last token
      if (delimStart == std::string::npos)
      {
        tList.push_back(Strip(s.substr(tokenStart)));
        break;
      }
      // found a delimiter; push everything since the last one
      tList.push_back(Strip(s.substr(tokenStart, delimStart - tokenStart)));
      // advance token start marker past end of delimiter
      tokenStart = delimStart + delim.length();
      // if we go off the edge, the delimiter was at the end
      if (tokenStart >= s.length())
      {
        // push an empty token as the last one
        tList.push_back(std::string());
        break;
      }
    }
    return tList;
  }
}

TableS::TableS(const std::string& filename, const FileTypeE fileType)
{
  switch (fileType)
  {
    case FT_TSV:  LoadTSV(filename);  break;
    case FT_WIKI: LoadWiki(filename); break;
  }
}

void TableS::Clear()
{
  headerM.clear();
  dataM.clear();
}

void TableS::Normalize()
{
  // scan header and data for maximum column count
  std::size_t numCols(headerM.size());
  for (RowListT::const_iterator rlCiter(dataM.begin()); rlCiter != dataM.end(); ++rlCiter)
  {
    if (rlCiter->size() > numCols) numCols = rlCiter->size();
  }
  // now scan them again and adjust as needed
  const std::string emptyString;
  while (headerM.size() < numCols)
  {
    headerM.push_back(emptyString);
  }
  for (RowListT::iterator rlIter(dataM.begin()); rlIter != dataM.end(); ++rlIter)
  {
    while (rlIter->size() < numCols) rlIter->push_back(emptyString);
  }
}

void TableS::LoadTSV(const std::string& filename)
{
  std::ifstream inFile;
  inFile.open(filename);
  if (!inFile.is_open())
  {
    throw std::runtime_error(std::string("Failed to open input file: '") + filename + "' for read");
  }
  Clear();
  std::string line;
  bool readingHeader(true);
  while (std::getline(inFile, line))
  {
    const TokenListT& t(Split(line, "\t"));
    if (readingHeader)
    {
      headerM = t;
      readingHeader = false;
      continue;
    }
    dataM.push_back(t);
  }
  inFile.close();
  Normalize();
}

void TableS::LoadWiki(const std::string& filename)
{
  // TODO
  throw std::runtime_error("TODO");
  Normalize();
}

void TableS::PrintTSV() const
{
  // now print the header row (if non-empty)
  if (!headerM.empty())
  {
    bool prependTab(false);
    for (ColListT::const_iterator clCiter(headerM.begin());
         clCiter != headerM.end(); ++clCiter)
    {
      if (prependTab) std::cout << "\t";
      else            prependTab = true;
      std::cout << *clCiter;
    }
    std::cout << "\n";
  }
  // now print the data rows
  for (RowListT::const_iterator rlCiter(dataM.begin());
       rlCiter != dataM.end(); ++rlCiter)
  {
    bool prependTab(false);
    for (ColListT::const_iterator clCiter(rlCiter->begin());
         clCiter != rlCiter->end(); ++clCiter)
    {
      if (prependTab) std::cout << "\t";
      else            prependTab = true;
      std::cout << *clCiter;
    }
    std::cout << "\n";
  }
}

void TableS::PrintWiki() const
{
  // print table start, caption, separator
  std::cout << "{| class=\"wikitable sortable\"\n";
  std::cout << "|+ Games for IBM PC compatibles with MT-32 support\n";
  std::cout << "|-\n";
  // print the header values
  for (ColListT::const_iterator clCiter(headerM.begin());
       clCiter != headerM.end(); ++clCiter)
  {
    // std::cout << "! scope=\"col\" | " << *clCiter << "\n";
    std::cout << "! " << *clCiter << "\n";
  }
  // print the row data
  //  loop over rows
  for (RowListT::const_iterator rlCiter(dataM.begin());
       rlCiter != dataM.end(); ++rlCiter)
  {
    // write a row separator line
    std::cout << "|-\n";
    // loop over columns within row
    std::size_t colNum(0);
    for (ColListT::const_iterator clCiter(rlCiter->begin());
         clCiter != rlCiter->end(); ++clCiter, ++colNum)
    {
      // write title column on its own row, followed by start of next row
      switch (colNum)
      {
        case 0: // title column
        {
          // write pipe, data, newline
          std::cout << "|" << *clCiter << std::endl;
        }
        break;

        case 1: // first column of remaining data
        {
          // prepend with single pipe
          std::cout << "|";
          // write data, or space if empty
          if (clCiter->empty()) std::cout << " ";
          else                   std::cout << *clCiter;
          // no newline
        }
        break;

        default: // remaining columns
        {
          // prepend with double pipe
          std::cout << "||";
          // write data, or space if empty
          if (clCiter->empty()) std::cout << " ";
          else                   std::cout << *clCiter;
        }
      }
    }
    std::cout << "\n"; // end of row
  }
  // print table end
  std::cout << "|}\n";
}

void TableS::WikiTitleClean()
{
  // loop over rows
  for (RowListT::iterator rlIter(dataM.begin());
       rlIter != dataM.end(); ++rlIter)
  {
    // loop over columns within row
    bool firstCol(true);
    for (ColListT::iterator rowIter(rlIter->begin());
         rowIter != rlIter->end(); ++rowIter)
    {
      std::string& cell(*rowIter);
      // strip cell
      cell = Strip(cell);
      // perform context-specific tasks
      if (firstCol)
      {
        // title column; italicize if needed
        // do nothing if empty
        if (cell.empty()) continue;
        // prepend with italic markup if needed
        if (cell.substr(0, MARKUP_ITALIC.length()) != MARKUP_ITALIC)
          cell.insert(0, MARKUP_ITALIC);
        // append with italic markup if needed
        if (cell.substr(cell.length() - MARKUP_ITALIC.length(), MARKUP_ITALIC.length()) != MARKUP_ITALIC)
          cell.append(MARKUP_ITALIC);
        firstCol = false;
        continue;
      }

      // non-title column
      // check for mis-capitalized check markup
      if (cell == "{{Ya}}" || cell == "{{yA}}" || cell == "{{YA}}")
      {
        cell = "{{ya}}";
      }
    }
  }
}

void TableS::WikiTitleSort()
{
  // clean titles so that we can make assumptions
  WikiTitleClean();
  // derive sort key from title column, then insert row into map by key
  std::map<std::string, ColListT> sortMap;
  for (auto rlIter(dataM.begin()); rlIter != dataM.end(); ++rlIter)
  {
    const std::string& title(rlIter->at(0));
    std::string key;
    // 5 cases are currently supported:
    // ''[[...|KEY]]''
    // ''[[KEY]]''
    // ''{{ill|...|lt=KEY|...}}''
    // ''{{ill|KEY|...}}''
    // falls back on entire string if none of these apply
    std::size_t keyStart(0);
    std::size_t keyEnd(std::string::npos);
    if (title.substr(0, 4) == "''[[")
    {
      // type 1 or 2; look for pipe
      keyStart = title.find_last_of('|');
      if (keyStart == std::string::npos)
      {
        // no pipe; this is type 2
        keyStart = 4;
      }
      else
      {
        // this is type 1; key starts after pipe
        ++keyStart;
      }
      keyEnd = title.length() - 4;
    }
    else if (title.substr(0, 8) == "''{{ill|")
    {
      // type 3 or 4; look for link text sequence
      keyStart = title.find("|lt=", 7);
      if (keyStart == std::string::npos)
      {
        // no link text sequence; this is type 4
        keyStart = 8;
      }
      else
      {
        // this is type 3; key starts after link text sequence
        keyStart += 4;
      }
      // key ends just before next pipe
      // TODO: this isn't very robust
      keyEnd = title.find("|", keyStart) - 1;
    }
    else
    {
      // unknown type; grab the whole thing
      keyStart = 0;
      keyEnd = title.length() - 1;
    }
    // increase keyStart if it's pointing at quotes
    if (title.substr(keyStart, 2) == "''") keyStart += 2;
    // decrease keyEnd if it's pointing at quotes
    if (title.substr(keyEnd - 1, 2) == "''") keyEnd -= 2;
    // extract the key
    key = title.substr(keyStart, keyEnd - keyStart);
    // now move any leading articles to the end
    if (key.substr(0, 2) == "A ")
    {
      key = key.substr(2, key.length() - 2).append(", A");
    }
    else if (key.substr(0, 3) == "An ")
    {
      key = key.substr(3, key.length() - 3).append(", An");
    }
    else if (key.substr(0, 4) == "The ")
    {
      key = key.substr(4, key.length() - 4).append(", The");
    }
    // convert key to uppercase
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    // we now have a fully-formed key
    // swap row into map by key
    sortMap[key].swap(*rlIter);
  }
  // we've built a sorted map; swap entries back into the table
  auto rlIter(dataM.begin());
  auto smIter(sortMap.begin());
  for (; smIter != sortMap.end(); ++smIter, ++rlIter)
  {
    rlIter->swap(smIter->second);
  }
}
