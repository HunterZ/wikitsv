cmake_minimum_required(VERSION 3.13.4)

project(wiki2tsv
  VERSION 1.0
  DESCRIPTION "Wikipedia table <-> TSV converters"
  LANGUAGES C CXX
)

add_executable(wiki2tsv
  wiki2tsv.cpp
)

add_executable(tsv2wiki
  TableS.h
  TableS.cpp
  tsv2wiki.cpp
)

add_executable(tsvsort
  TableS.h
  TableS.cpp
  tsvsort.cpp
)
