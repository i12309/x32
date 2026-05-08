#pragma once

#include <cmath>

#include "App/App.h"
#include "Catalog.h"
#include "Data.h"
#include "Remote/CanMachine.h"
#include "Service/Log.h"

class CanActions {
public:
    static bool CheckAll() {
        if (Remote::CanMachine::instance().checkAll()) return true;
        const String detail = String("CAN check failed: ") + Remote::CanMachine::instance().lastError();
        App::diag().addError(Catalog::ErrorCode::CHECK_FAILED, "CAN check failed", detail, false);
        return true;
    }

    static bool TableUp() {
        if (Remote::CanMachine::instance().tableUp(Catalog::SPEED::Normal)) return true;
        return fail(Catalog::ErrorCode::TABLE_NOT_UP, "TABLE_UP failed");
    }

    static bool TableDown() {
        if (Remote::CanMachine::instance().tableDown(Catalog::SPEED::Normal)) return true;
        return fail(Catalog::ErrorCode::TABLE_NOT_DOWN, "TABLE_DOWN failed");
    }

    static bool GuillotineHome() {
        if (Remote::CanMachine::instance().guillotineHome(Catalog::SPEED::Normal)) return true;
        return fail(Catalog::ErrorCode::GUILLOTINE_NOT_IN, "GUILLOTINE_HOME failed");
    }

    static bool GuillotineCut() {
        if (Remote::CanMachine::instance().guillotineCut()) return true;
        return fail(Catalog::ErrorCode::GUILLOTINE, "GUILLOTINE_CUT failed");
    }

    static bool PaperZeroPosition() {
        if (Remote::CanMachine::instance().paperZeroPosition()) return true;
        return fail(Catalog::ErrorCode::PAPER, "PAPER_ZERO_POSITION failed");
    }

    static bool DetectPaper() {
        Remote::ScenarioResult result;
        if (!Remote::CanMachine::instance().detectPaper(defaultDetectSteps(), result)) {
            return fail(Catalog::ErrorCode::PAPER_NOT_FOUND, "DETECT_PAPER failed");
        }

        Data::param.paperPosition = result.value;
        Log::D("DETECT_PAPER: position=%d", Data::param.paperPosition);
        return true;
    }

    static bool DetectMark() {
        Remote::ScenarioResult result;
        if (!Remote::CanMachine::instance().detectMark(defaultDetectSteps(), result)) {
            return fail(Catalog::ErrorCode::PAPER_FIND_IN_MARK, "DETECT_MARK failed");
        }

        Data::param.markPosition = result.value;
        Log::D("DETECT_MARK: position=%d", Data::param.markPosition);
        return true;
    }

    static bool PaperMoveFromPlan() {
        Catalog::WorkParam params = App::plan().getCurrentParams();
        const bool blocking = params.hasBlocking ? params.blocking : true;

        if (params.hasMm) {
            return paperMoveMm(params.mm, blocking);
        }
        if (params.hasSteps) {
            if (Remote::CanMachine::instance().paperMoveSteps(params.steps, blocking)) return true;
            return fail(Catalog::ErrorCode::PAPER, "PAPER_MOVE_STEPS failed");
        }

        return fail(Catalog::ErrorCode::CONFIG_ERROR, "PAPER_MOVE without steps/mm");
    }

    static bool ProductFeed() {
        const float mm = Data::work.task.PRODUCT_mm + (Data::work.task.OVER_mm * 2.0f);
        return paperMoveMm(mm, true);
    }

    static bool TailFeed() {
        return paperMoveMm(15.0f, true);
    }

    static bool EjectTail() {
        return paperMoveMm(60.0f, true);
    }

    static bool FeedWithThrowFromPlan() {
        Catalog::WorkParam params = App::plan().getCurrentParams();
        if (!params.hasMm) {
            return fail(Catalog::ErrorCode::CONFIG_ERROR, "PAPER_THROW group move needs mm");
        }
        if (Remote::CanMachine::instance().paperMoveWithThrowMm(params.mm, Data::work.profile.RATIO_mm)) {
            return true;
        }
        return fail(Catalog::ErrorCode::PAPER, "PAPER_THROW group move failed");
    }

    static bool paperMoveMm(float mm, bool blocking = true) {
        if (Remote::CanMachine::instance().paperMoveMm(mm, Data::work.profile.RATIO_mm, blocking)) {
            return true;
        }
        return fail(Catalog::ErrorCode::PAPER, "PAPER_MOVE_MM failed");
    }

private:
    static int32_t defaultDetectSteps() {
        const float ratio = Data::work.profile.RATIO_mm > 0.0f ? Data::work.profile.RATIO_mm : 100.0f;
        const float length = Data::work.profile.LENGHT_mm > 0.0f ? Data::work.profile.LENGHT_mm : 500.0f;
        return static_cast<int32_t>(lroundf((length + 150.0f) * ratio));
    }

    static bool fail(Catalog::ErrorCode code, const String& operation) {
        const String detail = operation + ": " + Remote::CanMachine::instance().lastError();
        App::diag().addFatal(code, operation, detail);
        return false;
    }
};
