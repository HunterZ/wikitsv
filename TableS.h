#ifndef TABLES_H
#define TABLES_H

#include <string>
#include <vector>

// utility class for modeling and managing a data table
struct TableS
{
  typedef std::vector<std::string> ColListT;
  typedef std::vector<ColListT>    RowListT;

  // list of column headers
  ColListT headerM;
  // list of table data rows, each containing a list of column data values for that row
  RowListT dataM;

  enum FileTypeE
  {
    // TSV file
    FT_TSV,
    FT_WIKI
  };

  // construct a table instance from a file of the specified type
  // throws std::runtime_error if file cannot be opened
  // automatically calls Normalize()
  TableS(const std::string& filename, const FileTypeE fileType = FT_TSV);

  // clear header and data contents
  void Clear();

  // normalize header and data rows to the same column count, by adding empty column values as needed
  void Normalize();

  // clear table and populate with data from TSV file
  // throws std::runtime_error if file cannot be opened
  // automatically calls Normalize()
  void LoadTSV(const std::string& filename);

  // clear table and populate with data from wiki-formatted file
  // throws std::runtime_error if file cannot be opened
  // automatically calls Normalize()
  void LoadWiki(const std::string& filename);

  // print table to stdout in TSV format
  void PrintTSV() const;

  // print table to stdout in wiki format
  void PrintWiki() const;

  // perform various cleanups for wiki export:
  // - strip leading/trailing whitespace from all cells
  // - ensure all title column values are italicized
  // - lowercase all checkbox column markup
  void WikiTitleClean();

  // extract wiki display text from first column of each data row, then perform title sort on that data
  // calls WikiTitleClean() to enforce italicizing
  void WikiTitleSort();
};

#endif
