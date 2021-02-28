#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace
{
  enum ReadStateE
  {
    // find table start
    RS_TABLE,
    // find and read column headers until row end
    RS_HEADER,
    // find and read column data until row end
    RS_DATA,
    // found table or file end
    RS_DONE
  };

  struct WikitableS
  {
    typedef std::vector<std::string> RowT;
    typedef std::vector<RowT> DataT;
    RowT headerM;
    DataT dataM;
    void NormalizeColumns()
    {
      // scan header and data for maximum column count
      std::size_t numColumns(headerM.size());
      for (DataT::const_iterator dCiter(dataM.begin()); dCiter != dataM.end(); ++dCiter)
      {
        if (dCiter->size() > numColumns) numColumns = dCiter->size();
      }
      // now scan them again and adjust as needed
      const std::string emptyString;
      while (headerM.size() < numColumns)
      {
        headerM.push_back(emptyString);
      }
      for (DataT::iterator dIter(dataM.begin()); dIter != dataM.end(); ++dIter)
      {
        while (dIter->size() < numColumns) dIter->push_back(emptyString);
      }
    }
  };

  bool IsCaption(const std::string& s)
  {
    return (!s.find("|+"));
  }

  bool IsData(const std::string& s)
  {
    return (!s.find("|"));
  }

  bool IsHeader(const std::string& s)
  {
    return (!s.find("!"));
  }

  bool IsRowSep(const std::string& s)
  {
    return (!s.find("|-"));
  }

  bool IsTableEnd(const std::string& s)
  {
    return (!s.find("|}"));
  }

  bool IsTableStart(const std::string& s)
  {
    return (!s.find("{|"));
  }

  void PrintUsage(const std::string& argv0)
  {
    std::cerr << "USAGE: " << argv0 << " FILE\n";
    std::cerr << "Convert FILE to TSV and write to stdout\n";
  }

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

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << argv[0] << ": Incorrect number of input files specified\n\n";
    PrintUsage(argv[0]);
    return -1;
  }

  std::ifstream inFile;
  inFile.open(argv[1]);
  if (!inFile.is_open())
  {
    std::cerr << argv[0] << ": Failed to open input file '" << argv[1] << "' for read\n\n";
    PrintUsage(argv[0]);
    return -2;
  }

  ReadStateE readState = RS_TABLE; // file read state
  std::string line; // file read line buffer
  WikitableS::RowT row; // in-progress data row buffer
  WikitableS table; // our table
  while (readState != RS_DONE)
  {
    if (!std::getline(inFile, line)) readState = RS_DONE;
    switch (readState)
    {
      // looking for table start
      case RS_TABLE:
      {
        if (IsTableStart(line))
        {
          // start of table
          // advance to header read
          readState = RS_HEADER;
        }
        // else swallow unknown line
      }
      break;

      // looking for headers
      case RS_HEADER:
      {
        if (IsRowSep(line))
        {
          // end of header row
          if (!table.headerM.empty())
          {
            // advance to row read state
            readState = RS_DATA;
          }
          // else must be caption-header separator?
          // TODO: this doesn't work for headerless tables
        }
        else if (IsHeader(line))
        {
          // this line contains one or more column headers
          // tokenize everything after the first character by "!!" in case of multiple headers in this line
          const TokenListT& headerList(Split(line.substr(1), "!!"));
/*
          // process tokens
          for (TokenListT::const_iterator tlCiter(headerList.begin());
               tlCiter != headerList.end(); ++tlCiter)
          {
            const std::string& t(*tlCiter);
            // for each token, grab the part after any '|' separator
            //  find last pipe symbol
            std::size_t start(t.find_last_of('|'));
            // if npos, reset start to zero to grab whole string
            // else advance past pipe, or npos if pipe at end
            start = (
              start == std::string::npos ? 0 :
              start + 1 >= t.length() ? std::string::npos :
              start + 1
            );
            // push stripped header string
            table.headerM.push_back(Strip(t.substr(start)));
          }
*/
          // append token list to header list
          if (!headerList.empty()) table.headerM.insert(table.headerM.end(), headerList.begin(), headerList.end());
        }
        // else swallow unknown line
      }
      break;

      // looking for regular cell data within a row
      case RS_DATA:
      {
        // check data separator last, because it's a subset of everything else
        if (IsRowSep(line))
        {
          // end of row data
          // push scratch to table and clear it
          if (!row.empty())
          {
            table.dataM.push_back(row);
            row.clear();
          }
        }
        else if (IsTableEnd(line))
        {
          // end of table
          // push scratch to table and clear it
          if (!row.empty())
          {
            table.dataM.push_back(row);
            row.clear();
          }
          // advance to end state
          readState = RS_DONE;
        }
        else if (IsData(line))
        {
          // this line contains one or more row data cells
          // tokenize everything after the first character by "||" in case of multiple cells in this line
          const TokenListT& dataList(Split(line.substr(1), "||"));
          // append token list to row data buffer
          if (!dataList.empty()) row.insert(row.end(), dataList.begin(), dataList.end());
        }
        // else swallow unknown line
      }
      break;

      case RS_DONE:
      {
        // we're done
        // if we have a partially-constructed row, just push it
        if (!row.empty()) table.dataM.push_back(row);
      }
      break;
    }
  }
  // done building table
  // close input file
  inFile.close();

  // now print the table data to stdout as tab-separated values
  // first, normalize table's column count across header and row data
  table.NormalizeColumns();
  // now print the header row (if non-empty)
  if (!table.headerM.empty())
  {
    bool prependTab(false);
    for (WikitableS::RowT::const_iterator rowCiter(table.headerM.begin());
         rowCiter != table.headerM.end(); ++rowCiter)
    {
      if (prependTab) std::cout << "\t";
      else            prependTab = true;
      std::cout << *rowCiter;
    }
    std::cout << "\n";
  }
  // now print the data rows
  for (WikitableS::DataT::const_iterator dataCiter(table.dataM.begin());
       dataCiter != table.dataM.end(); ++dataCiter)
  {
    bool prependTab(false);
    for (WikitableS::RowT::const_iterator rowCiter(dataCiter->begin());
         rowCiter != dataCiter->end(); ++rowCiter)
    {
      if (prependTab) std::cout << "\t";
      else            prependTab = true;
      std::cout << *rowCiter;
    }
    std::cout << "\n";
  }

  std::cerr << "\nrows: " << table.dataM.size() << ", cols: " << table.dataM.begin()->size() << "\n";

  return 0;
}
