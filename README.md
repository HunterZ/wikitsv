# wikitsv
Tools for transcoding between Wikimedia tables and TSV files

## Description
This repository contains a number of small utilities that I've written in C++ to help maintain a large table on Wikipedia.

## Building
Run CMake to generate a project file, then build via `cmake --build` or via IDE/make/ninja/whatever.

There are no special library dependencies, as this simply makes heavy use of the C++ standard library (things like Boost would be handy, but totally overkill!).

The code should also be platform-agnostic.

## Running
Each tool takes a single filename as a command line parameter, and writes its output to stdout.

## Tool Descriptions
### tsv2wiki
`tsv2wiki` converts a text file containing a tab-separated-values (TSV) formatted table into an equivalent Wikimedia markup table. This allows me to feed the output of a spreadsheet application or `tsvsort` back into a Wikipedia article.

### tsvsort
`tsvsort` applies a case-insensitive title sort/alphabetization to the first column (minus header row) of a TSV table. It also tries to extract the sort key from various Wikimedia link formats, and ignores some forms of italicization.

### wiki2tsv
`wiki2tsv` converts a Wikimedia markup table to a tab-separated-values (TSV) formatted table, for import into a spreadsheet application such as LibreOffice Calc or Microsoft Excel.

## DISCLAIMER
These tools are suited to my own purposes, and probably won't support your use cases and/or meet your needs. Feel free to use them as a starting point though!
