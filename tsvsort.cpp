#include <iostream>
#include "TableS.h"

namespace
{
  void PrintUsage(const std::string& argv0)
  {
    std::cerr << "USAGE: " << argv0 << " FILE\n";
    std::cerr << "Write cleaned+sorted copy of TSV-formatted FILE to stdout\n";
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

  TableS table(argv[1], TableS::FT_TSV);
  table.WikiTitleClean();
  table.WikiTitleSort();
  table.PrintTSV();

  return 0;
}
