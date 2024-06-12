#include "pid_controller.h"
#include <algorithm>

PID::PID(double Kp, double Ki, double Kd) 
    : Kp(Kp), Ki(Ki), Kd(Kd), prev_error(0), integral(0) {}

double PID::calculate(double setpoint, double measured_value) {
    double error = setpoint - measured_value;
    integral += error;
    double derivative = error - prev_error;
    prev_error = error;
    double output =  Kp * error + Ki * integral + Kd * derivative;
    output = std::max(-320.0, std::min(320.0, output));
    return output;
}

