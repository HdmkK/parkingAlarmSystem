# 주차 경고 시스템

<br>

## 1. 프로젝트 설명
- 후진주차시 장애물과 가까워지면 경고를 해주는 시스템입니다.
- 후방 장애물과의 거리가 일정이하 가까워지면 LED가 GREEN->RED으로 바뀌고 경고가 시작됩니다.
- 후방 장애물과의 거리가 일정이상 멀어지면 LED가 RED->GREEN으로 바뀌고 경고가 종료됩니다.
- 후방 장애물과의 거리가 가까워질수록 경고음이 빨라지고, 멀어질수록 느려집니다.

<br>

## 2. 시스템 구조도
![Image](https://github.com/user-attachments/assets/40fd0a7e-a8a6-4072-8fb7-9dced35e02ca)

<br>

## 3. 디렉터리 구조
```
.
├── device_driver/
│   ├── button/
│   │   ├── Makefile
│   │   └── button_device_driver.c
│   ├── buzzer/
│   │   ├── Makefile
│   │   ├── buzzer_device_driver.c
│   │   ├── buzzer_ioctl.h
│   │   ├── buzzer_test
│   │   ├── buzzer_test.c
│   │   └── mknod.sh
│   ├── led/
│   │   ├── Makefile
│   │   ├── led_device_driver.c
│   │   ├── led_test.c
│   │   └── mknod.sh
│   └── ultra/
│       ├── Makefile
│       ├── mknod.sh
│       ├── ultra_device_driver.c
│       └── ultra_test.c
├── user_app/
│   ├── buzzer_ioctl.h
│   ├── control_ECU
│   ├── control_ECU.c
│   ├── sensing_ECU
│   └── sensing_ECU.c
├── README.md
└── 팀프로젝트(1인) 발표자료.pdf

```

<br>

## 4. 이슈사항
버튼 디바이스 드라이버를 구현하던 중 2가지 이슈가 발생
### 4-1. 채터링
#### 이슈
- 버튼 인터럽트 핸들러로 메시지를 출력할 때 메시지가 여러번 출력
#### 원인
- 채터링 현상(전자 회로 내의 스위치나 릴레이의 접점이 붙거나 떨어질 때 기계적인 진동에 의해 실제로는 매우 짧은 시간 안에 접점이 붙었다가 떨어지는 것을 반복하는 현상)
#### 해결방법
- 버튼을 눌렀을 때 핸들러를 실행한 후, 일정시간(약 200ms)동안 버튼 인터럽트를 비활성화하여 해결

#### 버튼 인터럽트 핸들러
```c
static irqreturn_t button_isr(int irq, void* dev_id){

  //작업 수행...

  //버튼 인터럽트 비활성화
  disable_irq(button_irq);

  //200ms후 버튼 인터럽트를 활성화하도록 타이머 설정
  button_timer.delay_jiffies = msecs_to_jiffies(200);
  button_timer.irq_num = button_irq;
  timer_setup(&button_timer.timer, button_timer_func, 0);
  button_timer.timer.expires = jiffies + button_timer.delay_jiffies;
  add_timer(&button_timer.timer);

  return IRQ_HANDLED;
}
```
#### 타이머 핸들러
```c
static void button_timer_func(struct timer_list* t){
  struct button_timer_info *info = from_timer(info, t, timer);

  //버튼 인터럽트 다시 활성화
  enable_irq(info->irq_num);
}
```
<br>

### 4-2. 커널 패닉

#### 이슈
- 채터링 문제를 해결한 후, 버튼을 누르니 커널패닉 발생
- 버튼 인터럽트 핸들러에서 disable_irq()부분에서 커널 패닉 발생
#### 원인
- disable_irq는 인터럽트 핸들러가 모두 종료될 때까지 Wait한 후, 인터럽트를 비활성화.

<br>

![Image](https://github.com/user-attachments/assets/73b728b3-a24f-419f-a8fe-9aa67742e8ce)

- 버튼 인터럽트가 실행 중인데, 그 내부에서 버튼 인터럽트를 disable_irq로 바활성화
- 그 결과 현재 핸들러가 수행중이므로 WAIT을 하게 되고, 인터럽트 핸들러에서 스케줄링을 유발하는 동작을 했으므로 커널패닉
- (인터럽트 핸들러는 다른 thread의 커널스택에 기생하여 실행되기 때문에 스케줄링을 유발하는 동작(ex. sleep등)을 수행해선 안됨.)
#### 해결
- disable_irq_nosync는 단순히 인터럽트를 비활성화
- disable_irq 대신 disable_irq_nosync를 사용하여 버튼 인터럽트를 비활성화
```c
static irqreturn_t button_isr(int irq, void* dev_id){

  //작업 수행...

  //버튼 인터럽트 비활성화
  //disable_irq(button_irq);
  disable_irq_nosync(button_irq);

  ...

  return IRQ_HANDLED;
}
```
