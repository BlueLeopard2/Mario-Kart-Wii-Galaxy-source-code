#include <kamek.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Kart/KartDamage.hpp>
#include <MKWG.hpp>

//MKWG Dev Note: Code by Retro Rewind and WTP Teams

namespace MKWG {
namespace Race {
    static void ChangeBlueProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        bool itemModeBlastBlitz = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODERANDOM) ? Pulsar::MKWGSETTING_ITEMMODE_RANDOM : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
            itemModeBlastBlitz = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODEBLASTBLITZ) ? Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlastBlitz == Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ){
            dest->limit = 25;
        }
        if(itemModeRandom == Pulsar::MKWGSETTING_ITEMMODE_RANDOM){
            dest->limit = 5;
        }
    }
    kmCall(0x80790b74, ChangeBlueProp);

    static void ChangeBulletProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        bool itemModeBlastBlitz = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODERANDOM) ? Pulsar::MKWGSETTING_ITEMMODE_RANDOM : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
            itemModeBlastBlitz = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODEBLASTBLITZ) ? Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlastBlitz == Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ){
            dest->limit = 5;
        }
        if(itemModeRandom == Pulsar::MKWGSETTING_ITEMMODE_RANDOM){
            dest->limit = 25;
        }
    }
    kmCall(0x80790bf4, ChangeBulletProp);

    static void ChangeBombProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        bool itemModeBlastBlitz = Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODERANDOM) ? Pulsar::MKWGSETTING_ITEMMODE_RANDOM : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
            itemModeBlastBlitz = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODEBLASTBLITZ) ? Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlastBlitz == Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ){
            dest->limit = 25;
        }
        if(itemModeRandom == Pulsar::MKWGSETTING_ITEMMODE_RANDOM){
            dest->limit = 20;
        }
    }
    kmCall(0x80790bb4, ChangeBombProp);

} // namespace Race   
} // namespace MKWG

