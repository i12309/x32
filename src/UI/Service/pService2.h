#pragma once
#include <Nextion.h>
#include "Catalog.h"

#include "../Page.h"
#include "Controller/Registry.h"

class pService2: public Page {
    public:
    
      static pService2& getInstance() { static pService2 instance; return instance; }
      void loop() override { Page::loop(); nexLoop(nexT); };
      void show() override {
        Page::show();

        if (!(App::ctx().swThrow != nullptr)) {
          tThrow.setText("");
          tThrow.Set_background_color_bco(Catalog::Color::grey);
          //bThrow.Set_background_image_pic(-1);
          bThrow.detachPop();
        } else bThrow.attachPop(pop_bThrow, &bThrow);

        if (!((App::ctx().mBigelUp != nullptr) || (App::ctx().mBigelDown != nullptr))) {
          tBig.Set_background_color_bco(Catalog::Color::mediumGrey);
          bBig.setText("");
          //bBig.Set_background_image_pic(-1);
          bBig.detachPop();
        } else bBig.attachPop(pop_bBig, &bBig);

        if (!(App::ctx().mKnife1 != nullptr)) {
          tKnife.Set_background_color_bco(Catalog::Color::lightGrey);
          bKnife.setText("");
          //bKnife.Set_background_image_pic(-1);
          bKnife.detachPop();
        } else bKnife.attachPop(pop_bKnife, &bKnife);
      };
    
    private:
      NexTouch *nexT[5];
      // Button
      NexButton bThrow = NexButton(30, 8, "bThrow");
      NexText tThrow = NexText(30, 1, "tThrow");
      NexButton bBig   = NexButton(30, 9, "bBig");
      NexText tBig = NexText(30, 2, "tBig");
      NexButton bKnife = NexButton(30, 10, "bKnife");
      NexText tKnife = NexText(30, 3, "tKnife");
      NexButton bBack  = NexButton(30, 11, "bBack");

      //#####################################################################
      pService2() : Page(30, 0, "p5_Service2") {
          nexT[0] = &bBack;
          nexT[1] = &bThrow;
          nexT[2] = &bBig;
          nexT[3] = &bKnife;
          nexT[4] = NULL;

          bThrow.attachPop(pop_bThrow, &bThrow);
          bBig.attachPop(pop_bBig, &bBig);
          bKnife.attachPop(pop_bKnife, &bKnife);
          bBack.attachPop(pop_bBack, &bBack);
      }
    
      //#####################################################################
  
      static void pop_bThrow(void* ptr);
      static void pop_bBig(void* ptr);
      static void pop_bKnife(void* ptr);
      static void pop_bBack(void* ptr);
};
