#include "../Source/Config.h"
#include <iostream>
#include <limits>
#include <cstdlib>
using namespace LootAll;
static int checks=0;
static void Check(bool condition,const char* message) {
    ++checks;
    if(!condition) { std::cerr<<"FAILED: "<<message<<std::endl; std::exit(1); }
}
static Config Read(const std::string& s) { std::vector<std::string> w; return ParseConfig(ParseIni(s),w); }
int main() {
    Config c=Read("");
    Check(c.radius==20&&c.hotkeys[Nearby].key==0x2D,"empty config defaults");
    Check(c.categories[Weapons]&&!c.categories[SeveredLimbs],"safe category defaults");
    Check(c.preferBackpack&&c.allowStolen&&!c.lootFriendlies,"safe general defaults");
    Check(Read("[General]\nLootRadius=-4").radius==1,"lower radius clamp");
    Check(Read("[General]\nLootRadius=500").radius==100,"upper radius clamp");
    Check(Read("[General]\nLootRadius=nan").radius==20,"NaN rejected");
    Check(Read("[General]\nLootRadius=1e999").radius==20,"infinity rejected");
    Check(Read("[General]\nLootRadius=27cats").radius==20,"trailing garbage rejected");
    Check(Read("[General]\nLootRadius=22.5").radius==22.5,"fractional radius supported");
    Check(Read("[General]\nPreferBackpack=wrong").preferBackpack,"invalid boolean fallback");
    Check(!Read("[General]\nPreferBackpack=off").preferBackpack,"boolean word form");
    Check(Read("[General]\nSortMode=OriginalOrder").sort==OriginalOrder,"original sort");
    Check(Read("[General]\nSortMode=wrong").sort==ValuePerWeight,"invalid sort fallback");
    c=Read("[Hotkeys]\nLootNearby=INSERT\nTakeEverything=INSERT\nDisarmPreset=CTRL+INSERT");
    Check(!c.hotkeys[Everything].key,"duplicate disabled");
    Check(c.hotkeys[Disarm].key==0x2D&&c.hotkeys[Disarm].modifiers==1,"modifier binding distinct");
    bool valid=false; KeyBinding key=ParseKey("shift+alt+F24",valid);
    Check(valid&&key.key==0x87&&key.modifiers==6,"F24 and modifiers");
    ParseKey("F25",valid); Check(!valid,"out-of-range F key");
    ParseKey("CTRL+",valid); Check(!valid,"missing base key");
    key=ParseKey("NONE",valid); Check(valid&&!key.key,"disabled binding");
    c=Read("[Value]\nMinimumValue=100\nMinimumValuePerKg=50");
    Check(PassValues(100,2,c),"inclusive thresholds");
    Check(!PassValues(99,1,c),"minimum value");
    Check(!PassValues(100,3,c),"minimum efficiency");
    Check(PassValues(100,0,c),"positive zero-weight item");
    Check(!PassValues(0,0,c),"zero value and weight");
    Check(!PassValues(100,-1,c),"negative weight rejected");
    Check(!PassValues(100,std::numeric_limits<double>::quiet_NaN(),c),"NaN weight rejected");
    Check(!PassValues(100,std::numeric_limits<double>::infinity(),c),"infinite weight rejected");
    Check(!PassValues(std::numeric_limits<double>::infinity(),1,c),"infinite value rejected");
    c=Read("[Blacklist]\nItem1=id:123-mod\nItem2=name:Iron Stick\n[Whitelist]\nItem1=Ancient Science Book");
    Check(MatchRule(c.blacklist,"123-mod","Other"),"stable ID rule");
    Check(!MatchRule(c.blacklist,"other","123-mod"),"ID prefix does not match name");
    Check(MatchRule(c.blacklist,"other","Iron Stick"),"exact name rule");
    Check(!MatchRule(c.blacklist,"other","Iron Stick Improved"),"no substring rule");
    Check(MatchRule(c.whitelist,"other","Ancient Science Book"),"bare legacy name");
    c=Read("\xEF\xBB\xBF[general]\r\n lootradius = 31\r\n# ignored\n[Loot]\nFood=no");
    Check(c.radius==31&&!c.categories[Food],"UTF-8 BOM, CRLF, case and whitespace");
    c=Read("[Value]\nMinimumValue=-10\nMinimumValuePerKg=-10");
    Check(c.minimumValue==0&&c.minimumValuePerKg==0,"negative thresholds clamped");
    std::cout<<checks<<" configuration/filter boundary checks passed"<<std::endl;
    return 0;
}
