/*void ChargeJump(Kart::Movement& movement, const Kart::Collision& collision, Kart::Physics& physics, Input::State& state) {
    Item::Player& itemPlayer = Item::Manager::sInstance->players[collision.GetPlayerIdx()];
    const Kart::Pointers* pointers = itemPlayer.pointers;
    Kart::Status* status = pointers->kartStatus;
    static u16 height = 0; 
    static bool jumped = false;
    if (glider) {
        if (!hop && prevHop && accelerate && brake && status->stickX < 0.1 && status->stickX > -0.1 && movement.engineSpeed > 20) charging = true;
        if (charging) {
            counter +=1;
            effects.isAcceleratingAtStart = true;
            if (counter > 75 && movement.engineSpeed > 20 && state.motionControlFlick != 0) {
                jumped = true;
                status->trickableTimer = 0x4;
                counter = 0;
                height = 1;
                charging = false;
                wait = 50;
                effects.isAcceleratingAtStart = false;
            }
            if ((!accelerate || !brake || state.motionControlFlick != 0) && counter < 75) {
                counter = 0;
                charging = false;  
                effects.isAcceleratingAtStart = false;
            }  
        }
    }
    else counter = 0;
    wait -= 1;
    if (jumped && height >= 1) height += 1;
    if (height > 20) height = 0;
    if (height > 0) physics.speed0.y += 20 - height;
    if (jumped) movement.engineSpeed = 120;
    if (ground && wait < 0) {
        jumped = false;
        height = 0;
    }
    else {
        wait = 0;
        height = 0;
        jumped = false;
    }
    prevHop = hop;
}*/