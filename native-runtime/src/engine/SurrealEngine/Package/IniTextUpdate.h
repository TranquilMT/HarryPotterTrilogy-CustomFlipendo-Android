#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>

namespace IniTextUpdate
{
struct Key { std::string Name; std::vector<std::string> Values; };
struct Section { std::string Name; std::vector<Key> Keys; };
inline std::string Trim(std::string s)
{
    const auto a=s.find_first_not_of(" \t\r\n");
    if(a==std::string::npos) return {};
    return s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
inline std::string Fold(std::string s)
{
    for(char& c:s) c=char(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
// Rewrite each known key once with its complete value list. Old excess rows
// disappear, new rows are inserted, and comments/unknown lines survive.
inline std::string Update(const std::vector<std::string>& lines, const std::vector<Section>& sections)
{
    std::map<std::string,const Section*> models;
    for(const auto& section:sections) models[Fold(section.Name)]=&section;
    std::set<std::pair<std::string,std::string>> emitted;
    std::set<std::string> seenSections;
    std::string result,current;
    auto write=[&](const std::string& section,const Key& key)
    {
        if(!emitted.emplace(section,Fold(key.Name)).second) return;
        for(const auto& value:key.Values) result+=key.Name+"="+value+"\n";
    };
    auto flush=[&]()
    {
        auto it=models.find(current);
        if(it!=models.end()) for(const auto& key:it->second->Keys) write(current,key);
    };
    seenSections.insert(current);
    for(auto line:lines)
    {
        while(!line.empty()&&(line.back()=='\r'||line.back()=='\n')) line.pop_back();
        const auto trimmed=Trim(line);
        if(trimmed.size()>=2&&trimmed.front()=='['&&trimmed.back()==']')
        {
            flush(); current=Fold(trimmed.substr(1,trimmed.size()-2));seenSections.insert(current);
            result+=line+'\n';continue;
        }
        const auto equal=trimmed.find('=');bool handled=false;
        auto model=models.find(current);
        if(!trimmed.empty()&&trimmed[0]!=';'&&trimmed.rfind("//",0)!=0&&equal!=std::string::npos&&model!=models.end())
        {
            auto name=Fold(Trim(trimmed.substr(0,equal)));
            for(const auto& key:model->second->Keys) if(Fold(key.Name)==name)
            {write(current,key);handled=true;break;}
        }
        if(!handled) result+=line+'\n';
    }
    flush();
    for(const auto& section:sections)
    {
        const auto name=Fold(section.Name);
        if(seenSections.count(name)) continue;
        if(!section.Name.empty()) result+='\n'+std::string("[")+section.Name+"]\n";
        for(const auto& key:section.Keys) write(name,key);
    }
    return result;
}
}
