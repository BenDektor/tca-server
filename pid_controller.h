#ifndef PID_H
#define PID_H

class PID {
public:
    PID(double Kp, double Ki, double Kd);

    double calculate(double setpoint, double measured_value);

private:
    double Kp, Ki, Kd;
    double prev_error;
    double integral;
};

#endif // PID_H
