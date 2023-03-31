#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor, Controllers::HeartRateController& controller)
  : heartRateSensor {heartRateSensor}, controller {controller} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);

  if (pdPASS != xTaskCreate(HeartRateTask::Process, "Heartrate", 500, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  lastBpm = 0;

  while (true) {
    auto delay = CurrentTaskDelay();
    Messages msg;

    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          if (state == States::Running) {
            state = States::Idle;
          } else if (state == States::Measuring) {
            state = States::BackgroundWaiting;
            backgroundMeasurementWaitingStart = xTaskGetTickCount();
            StopMeasurement();
          }
          break;
        case Messages::WakeUp:
          if (state == States::Idle) {
            state = States::Running;
          } else if (state == States::BackgroundMeasuring) {
            state = States::Measuring;
          } else if (state == States::BackgroundWaiting) {
            state = States::Measuring;
            StartMeasurement();
          }
          break;
        case Messages::StartMeasurement:
          if (state == States::Measuring || state == States::BackgroundMeasuring) {
            break;
          }
          state = States::Measuring;
          lastBpm = 0;
          StartMeasurement();
          break;
        case Messages::StopMeasurement:
          if (state == States::Running || state == States::Idle) {
            break;
          }
          if (state == States::Measuring) {
            state = States::Running;
          } else if (state == States::BackgroundMeasuring) {
            state = States::Idle;
          }
          StopMeasurement();
          break;
      }
    }

    if (state == States::BackgroundWaiting) {
      HandleBackgroundWaiting();
    } else if (state == States::BackgroundMeasuring || state == States::Measuring) {
      HandleSensorData();
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

void HeartRateTask::StartMeasurement() {
  heartRateSensor.Enable();
  vTaskDelay(100);
  ppg.SetOffset(heartRateSensor.ReadHrs());
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  vTaskDelay(100);
}

void HeartRateTask::HandleBackgroundWaiting() {
  if (xTaskGetTickCount() - backgroundMeasurementWaitingStart >= DURATION_BETWEEN_BACKGROUND_MEASUREMENTS) {
    state = States::BackgroundMeasuring;
    StartMeasurement();
  }
}

void HeartRateTask::HandleSensorData() {
  ppg.Preprocess(static_cast<float>(heartRateSensor.ReadHrs()));
  auto bpm = ppg.HeartRate();

  if (lastBpm == 0 && bpm == 0) {
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, 0);
  }

  if (bpm != 0) {
    lastBpm = bpm;
    controller.Update(Controllers::HeartRateController::States::Running, lastBpm);
    if (state == States::BackgroundMeasuring) {
      StopMeasurement();
      state = States::BackgroundWaiting;
      backgroundMeasurementWaitingStart = xTaskGetTickCount();
    }
  }
}

int HeartRateTask::CurrentTaskDelay() {
    switch (state) {
      case States::Measuring:
      case States::BackgroundMeasuring:
        return 50;
      case States::Running:
        return 100;
      case States::BackgroundWaiting:
        return 500;
      default:
        return portMAX_DELAY;
    }
}