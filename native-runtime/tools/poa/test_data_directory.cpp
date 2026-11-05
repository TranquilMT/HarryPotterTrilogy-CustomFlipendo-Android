#include <filesystem>
#include <fstream>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include "Package/DataDirectory.h"
int main(int argc,char**argv){
 assert(argc==2);namespace fs=std::filesystem;fs::path root=argv[1];fs::create_directories(root/"Maps");
 std::ofstream(root/"Maps"/"Intro.unr")<<"map";std::ofstream(root/"Maps"/"Menu.unr")<<"map";fs::create_directory(root/"Maps"/"Nested");
 int count=0;std::vector<std::string> maps;
 auto collect=[&](const auto&e){maps.push_back(e.path().filename().string());};
 assert(VisitDataFiles(root/"TestMaps",collect));assert(maps.empty());++count;
 assert(!VisitDataFiles(root/"Maps",collect));assert(maps.size()==2);++count;
 maps.clear();assert(VisitDataFiles(root/"Maps"/"Intro.unr",collect));assert(maps.empty());++count;
 fs::create_directory(root/"Empty");assert(!VisitDataFiles(root/"Empty",collect));assert(maps.empty());++count;
 fs::create_directory_symlink(root/"Missing",root/"Broken");assert(VisitDataFiles(root/"Broken",collect));++count;
 fs::remove_all(root/"Empty");assert(VisitDataFiles(root/"Empty",collect));++count;
 try{VisitDataFiles(root/"Maps",[](const auto&){throw std::runtime_error("parse failure");});assert(false);}catch(const std::runtime_error&){++count;}
 std::cout<<count<<" production data-directory cases passed\n";
}
