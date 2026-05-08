#include "pService2.h"
#include "pService.h"
#include "pThrow.h"
#include "pKnife.h"
#include "pBigel.h"

  void pService2::pop_bBack(void* ptr) {
    Log::D(__func__);
    pService::getInstance().show();
  }

  void pService2::pop_bThrow(void* ptr) {
    Log::D(__func__);
    pThrow::getInstance().show();
  }
  void pService2::pop_bBig(void* ptr) {
    Log::D(__func__);
    pBigel::getInstance().show();
  }
  void pService2::pop_bKnife(void* ptr) {
    Log::D(__func__);
    pKnife::getInstance().show();
  }
