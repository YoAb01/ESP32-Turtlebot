idf_component_register(
  SRCS
    "main.cpp"
    "LED/LED_Blink.cpp"
    "timer/timer.cpp"
    "robot/robot.cpp"
    "robot/servo.cpp"
    "connection/wifi_ap.cpp"
    "connection/wifi_conn.cpp"
    "sensors/ultrasonic.cpp"
    "sensors/imu_gy521.cpp"
  INCLUDE_DIRS
    "."
    "LED"
    "robot"
    "timer"
    "connection"
    "sensors"
  )
