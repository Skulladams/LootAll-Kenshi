// LootAll - GPL-3.0-only
#include "Config.h"
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <limits>
#include <cmath>
#include <algorithm>
namespace LootAll {
const char* const CategoryNames[CategoryCount] = {"Weapons","Armour","Shirts","Pants","Boots","Headgear","Backpacks","Food","Medicine","Ammo","BuildingMaterials","CraftingMaterials","Research","Blueprints","Books","Tools","Robotics","TradeGoods","Narcotics","SeveredLimbs","Miscellaneous"};
Config::Config():radius(20),minimumValue(0),minimumValuePerKg(0),preferBackpack(true),bestItemsFirst(true),lootFriendlies(false),allowStolen(true),debug(false),sort(ValuePerWeight) {
    for(int i=0;i<CategoryCount;++i) categories[i]=i!=SeveredLimbs;
    hotkeys[Nearby]=KeyBinding(0x2D);
}
std::string Trim(const std::string& s) {
    const size_t a=s.find_first_not_of(" \t\r\n");
    if(a==std::string::npos) return "";
    return s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
std::string Upper(std::string s) {
    for(size_t i=0;i<s.size();++i) s[i]=static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
    return s;
}
Ini ParseIni(const std::string& text) {
    Ini out; std::istringstream input(text); std::string line, section;
    while(std::getline(input,line)) {
        if(line.size()>=3 && static_cast<unsigned char>(line[0])==0xEF && static_cast<unsigned char>(line[1])==0xBB && static_cast<unsigned char>(line[2])==0xBF) line=line.substr(3);
        line=Trim(line); if(line.empty()||line[0]==';'||line[0]=='#') continue;
        if(line[0]=='[' && line[line.size()-1]==']') { section=Upper(Trim(line.substr(1,line.size()-2))); continue; }
        size_t eq=line.find('='); if(eq==std::string::npos) continue;
        out[section][Upper(Trim(line.substr(0,eq)))]=Trim(line.substr(eq+1));
    }
    return out;
}
static std::string Get(const Ini& ini,const char* section,const char* key) {
    Ini::const_iterator s=ini.find(Upper(section)); if(s==ini.end()) return "";
    std::map<std::string,std::string>::const_iterator k=s->second.find(Upper(key));
    return k==s->second.end()?"":k->second;
}
static bool Boolean(const Ini& ini,const char* section,const char* key,bool fallback,std::vector<std::string>& w) {
    std::string s=Upper(Get(ini,section,key)); if(s.empty()) return fallback;
    if(s=="1"||s=="TRUE"||s=="YES"||s=="ON") return true;
    if(s=="0"||s=="FALSE"||s=="NO"||s=="OFF") return false;
    w.push_back(std::string("Invalid boolean: ")+key); return fallback;
}
static double Number(const Ini& ini,const char* section,const char* key,double fallback,double low,double high,std::vector<std::string>& w) {
    std::string s=Get(ini,section,key); if(s.empty()) return fallback;
    char* end=0; double n=std::strtod(s.c_str(),&end);
    if(end==s.c_str()||*end||n!=n||n>std::numeric_limits<double>::max()||n< -std::numeric_limits<double>::max()) { w.push_back(std::string("Invalid number: ")+key); return fallback; }
    return std::max(low,std::min(high,n));
}
KeyBinding ParseKey(const std::string& text,bool& valid) {
    valid=true; std::string s=Upper(Trim(text)); KeyBinding result;
    if(s=="NONE") return result;
    size_t pos;
    while((pos=s.find('+'))!=std::string::npos) {
        std::string part=Trim(s.substr(0,pos)); s=Trim(s.substr(pos+1));
        if(part=="CTRL"||part=="CONTROL") result.modifiers|=1;
        else if(part=="ALT") result.modifiers|=2;
        else if(part=="SHIFT") result.modifiers|=4;
        else { valid=false; return KeyBinding(); }
    }
    if(s.size()==1 && ((s[0]>='A'&&s[0]<='Z')||(s[0]>='0'&&s[0]<='9'))) result.key=s[0];
    else if(s.size()>1 && s[0]=='F') {
        char* end=0; long n=std::strtol(s.c_str()+1,&end,10);
        if(!*end&&n>=1&&n<=24) result.key=0x70+static_cast<int>(n)-1;
    }
    if(!result.key) {
        const char* names[]={"INSERT","DELETE","HOME","END","PAGEUP","PAGEDOWN","SPACE","TAB","ENTER","ESCAPE","UP","DOWN","LEFT","RIGHT","PAUSE"};
        const int keys[]={0x2D,0x2E,0x24,0x23,0x21,0x22,0x20,9,13,27,0x26,0x28,0x25,0x27,0x13};
        for(int i=0;i<15;++i) if(s==names[i]) result.key=keys[i];
        if(s.size()==7 && s.substr(0,6)=="NUMPAD" && s[6]>='0'&&s[6]<='9') result.key=0x60+s[6]-'0';
    }
    if(!result.key) valid=false;
    return result;
}
Config ParseConfig(const Ini& ini,std::vector<std::string>& w) {
    Config c;
    c.radius=Number(ini,"General","LootRadius",20,1,100,w);
    c.minimumValue=Number(ini,"Value","MinimumValue",0,0,1e12,w);
    c.minimumValuePerKg=Number(ini,"Value","MinimumValuePerKg",0,0,1e12,w);
    c.preferBackpack=Boolean(ini,"General","PreferBackpack",true,w);
    c.bestItemsFirst=Boolean(ini,"General","BestItemsFirst",true,w);
    c.lootFriendlies=Boolean(ini,"General","LootFriendlies",false,w);
    c.allowStolen=Boolean(ini,"General","AllowStolenItems",true,w);
    c.debug=Boolean(ini,"General","DebugLogging",false,w);
    if(Boolean(ini,"General","LootNearbyContainers",false,w)) w.push_back("World-container looting is not supported; body looting remains enabled.");
    std::string sort=Upper(Get(ini,"General","SortMode"));
    if(sort=="VALUE") c.sort=Value;
    else if(sort=="ORIGINALORDER") c.sort=OriginalOrder;
    else if(!sort.empty()&&sort!="VALUEPERWEIGHT") w.push_back("Invalid SortMode; using ValuePerWeight.");
    for(int i=0;i<CategoryCount;++i) c.categories[i]=Boolean(ini,"Loot",CategoryNames[i],c.categories[i],w);
    const char* keys[]={"LootNearby","TakeEverything","ValuablePreset","DisarmPreset","SuppliesPreset"};
    for(int i=0;i<PresetCount;++i) {
        std::string s=Get(ini,"Hotkeys",keys[i]); if(s.empty()) continue;
        bool valid; KeyBinding k=ParseKey(s,valid);
        if(valid) c.hotkeys[i]=k; else w.push_back(std::string("Invalid hotkey: ")+keys[i]);
    }
    for(int i=0;i<PresetCount;++i) for(int j=0;j<i;++j)
        if(c.hotkeys[i].key && c.hotkeys[i].key==c.hotkeys[j].key && c.hotkeys[i].modifiers==c.hotkeys[j].modifiers) {
            c.hotkeys[i]=KeyBinding(); w.push_back(std::string("Duplicate binding disabled: ")+keys[i]);
        }
    const char* sections[]={"BLACKLIST","WHITELIST"};
    for(int i=0;i<2;++i) {
        Ini::const_iterator it=ini.find(sections[i]); if(it==ini.end()) continue;
        for(std::map<std::string,std::string>::const_iterator r=it->second.begin();r!=it->second.end();++r)
            if(!r->second.empty()) (i?c.whitelist:c.blacklist).push_back(r->second);
    }
    return c;
}
bool MatchRule(const std::vector<std::string>& rules,const std::string& id,const std::string& name) {
    for(size_t i=0;i<rules.size();++i) {
        std::string prefix=Upper(rules[i].substr(0,3));
        if(prefix=="ID:" ? rules[i].substr(3)==id : (Upper(rules[i].substr(0,5))=="NAME:" ? rules[i].substr(5)==name : (rules[i]==id||rules[i]==name))) return true;
    }
    return false;
}
double Efficiency(double value,double weight) { return weight>0 ? value/weight : (value>0 ? std::numeric_limits<double>::max() : 0); }
bool PassValues(double value,double weight,const Config& c) {
    return value==value && weight==weight && value>=0 && weight>=0
        && value<=std::numeric_limits<double>::max() && weight<=std::numeric_limits<double>::max()
        && value>=c.minimumValue && Efficiency(value,weight)>=c.minimumValuePerKg;
}
}
