#include <string>
#include <vector>
#include <cassert>
#include <iostream>
template<class T>using Array=std::vector<T>;
#include "Package/GameIniLayout.h"
int main(){
 int count=0;
 for(auto exe:{"hppoa","HPPOA"})for(int ancestry:{0,129,219,220,500}){GameLaunchInfo g;g.gameExecutableName=exe;g.ue1Version=ancestry;assert(!UsesIndexedPackagePaths(g));assert(!UsesCombinedUserIni(g));assert(LoadsSeparateUserIni(g));++count;}
 for(auto exe:{"Unreal","HP","Game"})for(int ancestry:{219,220,436}){GameLaunchInfo g;g.gameExecutableName=exe;g.ue1Version=ancestry;assert(UsesIndexedPackagePaths(g)==(ancestry<=219));assert(UsesCombinedUserIni(g)==(ancestry<=219));assert(LoadsSeparateUserIni(g)==(ancestry>219));++count;}
 GameLaunchInfo rune;rune.gameExecutableName="Rune";rune.ue1Version=219;assert(!UsesIndexedPackagePaths(rune));++count;
 GameLaunchInfo khg;khg.gameExecutableName="Khg";khg.ue1Version=219;assert(UsesIndexedPackagePaths(khg)&&UsesCombinedUserIni(khg));++count;
 GameLaunchInfo poa;poa.gameExecutableName="hppoa";assert(UsesDefaultPackagePaths(poa,false));assert(!UsesDefaultPackagePaths(poa,true));count+=2;
 GameLaunchInfo hp2;hp2.gameExecutableName="Game";hp2.ue1Version=436;assert(!UsesDefaultPackagePaths(hp2,false));++count;
 std::cout<<count<<" production INI layout cases passed\n";
}
