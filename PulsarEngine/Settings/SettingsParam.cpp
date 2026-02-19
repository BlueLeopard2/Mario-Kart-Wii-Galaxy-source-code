#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <Config.hpp>
#include <Settings/SettingsParam.hpp>

namespace Pulsar {
namespace Settings {

u8 Params::radioCount[Params::pageCount] ={
    3, 5, 2, 5, 2, //menu, race, host, OTT, KO
    2, 3, 4, //MKWG
};
u8 Params::scrollerCount[Params::pageCount] ={ 
    1, 1, 2, 0, 2, //menu, race, host, OTT, KO
    1, 2, 1, //MKWG
    };
u8 Params::buttonsPerPagePerRow[Params::pageCount][Params::maxRadioCount] = //first row is PulsarSettingsType, 2nd is rowIdx of radio
{
    { 2, 2, 3, 0, 0, 0}, //Menu
    { 2, 2, 2, 2, 3, 0}, //Race
    { 2, 2, 0, 0, 0, 0}, //Host
    { 3, 3, 2, 2, 2, 0}, //OTT
    { 2, 2, 0, 0, 0, 0}, //KO

    { 3, 4, 0, 0, 0, 0}, //Page1
    { 3, 2, 2, 0, 0, 0}, //Page2
    { 2, 2, 2, 4, 0, 0}, //Page3
    //{ 3, 0, 0, 0, 0, 0}, //Page4
    //{ 3, 0, 0, 0, 0, 0}, //Page5
};
u8 Params::optionsPerPagePerScroller[Params::pageCount][Params::maxScrollerCount] =
{
    { 5, 7, 0, 0, 0}, //Menu
    { 4, 0, 0, 0, 0}, //Race
    { 7, 7, 0, 0, 0}, //Host
    { 0, 0, 0, 0, 0}, //OTT
    { 4, 4, 0, 0, 0}, //KO

    { 6, 0, 0, 0, 0}, //Page1
    { 4, 6, 0, 0, 0}, //Page2
    { 6, 0, 0, 0, 0}, //Page3
    //{16,16,16,16, 0}, //Page4
    //{16,16,16,16, 0}, //Page5
};

}//namespace Settings
}//namespace Pulsar




