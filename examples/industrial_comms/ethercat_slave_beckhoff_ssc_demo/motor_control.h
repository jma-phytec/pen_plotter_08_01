/*
 * Motor Control Header File
 */


#define MOTORX
//#define MYDEBUG

struct MotorModule {
    uint32_t step_base_addr;
    uint32_t step_pin;
    uint32_t step_dir;
    uint32_t dir_base_addr;
    uint32_t dir_pin;
    uint32_t dir_dir;
    float cur_pos;   // Current Position
    float next_pos;  // Target Position of current movement
    uint32_t positioning;   // Absolute or Incremental Mode
    uint32_t dir;       // Direction of movement
    float speed;        // Speed of current movement
    float pulse_width;  // Motor speed
    uint32_t unit;      // MM or Inch
    uint32_t moving;    // Indicate current operation
    Bool isActive;
};

typedef struct MotorModule MotorMod;

enum MotorCmd {
    UPDATE_POSITIONING = 1,
    UPDATE_DIR,
    UPDATE_UNIT,
    UPDATE_MOVING,
    UPDATE_HOME
};

void gpio_motor_control_ioctl(MotorMod *Motor, uint8_t cmd, uint32_t val);
void gpio_motor_move(MotorMod *Motor, Bool isNegative);
void gpio_motor_control_init(MotorMod *Motor, uint32_t core_id);
void gpio_motor_control_dir_main(MotorMod *Motor);
//void gpio_motor_control_step_main(void *args);
void gpio_motor_control_step_main(MotorMod *Motor, float LoopRequired);
void gpio_motor_control_setCurPos(MotorMod *Motor, float val);
void gpio_motor_control_setNextPos(MotorMod *Motor, float val, int isNegative);
void gpio_motor_control_setSpeed(MotorMod *Motor, float val);

