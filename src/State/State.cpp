#include "State.h"

// Состояние для железа 
#include "State/Device/Boot.h"
#include "State/Device/Check.h"
#include "State/Device/Pressure.h"

// Системные состояния - для всех устройств 
#include "State/System/Idle.h"
#include "State/System/FINISH.h"
#include "State/System/NULLSTATE.h"
#include "State/System/FRAME.h"
#include "State/System/DONE.h"
#include "State/System/ACTION.h"
#include "State/System/Error.h"
#include "State/System/Service.h"

// Общие для разных устройств состояния 
#include "State/Main/TableUp.h"
#include "State/Main/DetectPaper.h"
#include "State/Main/DetectMark.h"
#include "State/Main/GuillotineForward.h"
#include "State/Main/PaperMove.h"

#include "State/Main/PaperPull.h"
//#include "State/Main/PaperOut.h"

// MODEL A - Состояния для простого нарезчик визиток
#include "State/A/Process.h"
#include "State/A/Calibration.h"
#include "State/A/Profilling.h"
#include "State/A/autoPROFILE.h"
#include "State/A/Slice.h"

void State::init(){
    App::state() = new Boot();
    App::state()->oneRun();
}

State* State::Factory(State::Type type){
    switch(type) {
        case State::Type::NULL_STATE:          return new NULLSTATE(); break;
        case State::Type::BOOT:                return new Boot(); break;
        case State::Type::IDLE:                return new Idle(); break;
        case State::Type::FINISH:                return new FINISH(); break;

        case State::Type::TABLE_UP:            return new TableUp(); break;

        case State::Type::DETECT_PAPER:       return new DetectPaper(); break;
        case State::Type::DETECT_MARK:        return new DetectMark(); break;
        case State::Type::PAPER_MOVE:          return new PaperMove(); break;
        case State::Type::PAPER_PULL:           return new PaperPull(); break;

        //case State::Type::PAPER_OUT:      return new PaperOut(); break;

        case State::Type::GUILLOTINE_FORWARD:      return new GuillotineForward(); break;

        case State::Type::ERROR:              return new Error(); break;

        case State::Type::FRAME:       return new FRAME(); break;
        case State::Type::PRESSURE:       return new Pressure(); break;
        case State::Type::DONE:         return new DONE(); break;
        case State::Type::ACTION:       return new ACTION(); break;

        case State::Type::CHECK:         switch(App::machine().type()) { case MachineType::A: return new A_CHECK(); break;} break;
        case State::Type::SERVICE:       switch(App::machine().type()) { case MachineType::A: return new A_SERVICE(); break;} break;
        case State::Type::PROCESS:       switch(App::machine().type()) { case MachineType::A: return new A_PROCESS(); break;} break;
        case State::Type::CALIBRATION:   switch(App::machine().type()) { case MachineType::A: return new Calibration(); break;} break;
        case State::Type::PROFILING:     switch(App::machine().type()) { case MachineType::A: return new Profilling(); break;} break;
        case State::Type::SLICE:         switch(App::machine().type()) { case MachineType::A: return new Slice(); break;} break;
        //case State::Type::PROFILING:     switch(App::machine().type()) { case MachineType::A: return new autoPROFILE(); break;} break;
    }
    return nullptr;
};
