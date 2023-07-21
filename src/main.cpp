#include <mbed.h>

constexpr uint32_t can_id = 30;

BufferedSerial pc{USBTX, USBRX, 115200};

// CAN can{PA_11, PA_12, (int)1e6};
CAN can{PB_12, PB_13, (int)1e6};
CANMessage msg;

Timer timer;

struct FirstPenguin {
  static constexpr int max = INT16_MAX;
  uint32_t send_id;
  int16_t pwm[4] = {};
  struct {
    int32_t enc;
    uint32_t adc;
    void set(const uint8_t data[8]) {
      memcpy(this, data, sizeof(*this));
    }
  } receive[4] = {};
  void read(const CANMessage& msg) {
    if(msg.format == CANStandard && msg.type == CANData && msg.len == sizeof(receive[0])) {
      receive[msg.id - send_id - 1].set(msg.data);
    }
  }
  bool send() {
    return can.write(CANMessage{send_id, reinterpret_cast<const uint8_t*>(pwm), 8});
  }
};

FirstPenguin penguin{can_id};

int main() {
  printf("\nsetup\n");
  timer.start();
  auto pre = timer.elapsed_time();
  while(1) {
    auto now = timer.elapsed_time();

    if(can.read(msg)) {
      penguin.read(msg);
    }

    if(now - pre > 20ms) {
      for(auto e: penguin.receive) printf("% 5ld\t", e.enc);
      for(auto e: penguin.receive) printf("% 5ld\t", e.adc);
      printf("\n");
      for(auto& e: penguin.pwm) e = penguin.max / 4;
      penguin.send();
      pre = now;
    }
  }
}
