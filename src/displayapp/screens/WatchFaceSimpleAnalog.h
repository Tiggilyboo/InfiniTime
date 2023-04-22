#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include <displayapp/screens/BatteryIcon.h>
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }
  namespace Applications {
    namespace Screens {

      class WatchFaceSimpleAnalog : public Screen {
      public:
        WatchFaceSimpleAnalog(Controllers::DateTime& dateTimeController,
                              const Controllers::Battery& batteryController,
                              const Controllers::Ble& bleController,
                              Controllers::NotificationManager& notificationManager,
                              Controllers::Settings& settingsController,
                              Controllers::HeartRateController& heartRateController,
                              Controllers::MotionController& motionController);

        ~WatchFaceSimpleAnalog() override;

        void Refresh() override;

      private:
        uint8_t sHour, sMinute, sSecond;

        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {0};
        Utility::DirtyValue<bool> isCharging {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
        Utility::DirtyValue<bool> notificationState {false};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<bool> motionSensorOk {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};

        lv_obj_t* dot;
        lv_obj_t* hour_body;
        lv_obj_t* hour_body_trace;
        lv_obj_t* minute_body;
        lv_obj_t* minute_body_trace;
        lv_obj_t* second_body;

        lv_point_t hour_point[2];
        lv_point_t hour_trace_point[2];
        lv_point_t minute_point[2];
        lv_point_t minute_trace_point[2];
        lv_point_t second_point[2];

        lv_style_t hour_line_style;
        lv_style_t hour_trace_line_style;
        lv_style_t minute_line_style;
        lv_style_t minute_trace_line_style;
        lv_style_t second_line_style;

        lv_obj_t* label_date_day;
        lv_obj_t* plugIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* notificationIcon;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepsMeter;

        BatteryIcon batteryIcon;

        const Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        void UpdateClock();
        void SetBatteryIcon();

        lv_task_t* taskRefresh;
      };
    }
  }
}