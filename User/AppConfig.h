#ifndef APP_CONFIG_H
#define APP_CONFIG_H


typedef enum {
  FC_TASK_PRIORITY_NONE          = 0,          ///< No priority (not initialized).
  FC_TASK_PRIORITY_IDLE          = 1,          ///< Reserved for Idle thread.
  FC_TASK_PRIORITY_LOW           = 4,          ///< Priority: low
  FC_TASK_PRIORITY_LOW1          = 4+1,        ///< Priority: low + 1
  FC_TASK_PRIORITY_LOW2          = 4+2,        ///< Priority: low + 2
  FC_TASK_PRIORITY_LOW3          = 4+3,        ///< Priority: low + 3
  FC_TASK_PRIORITY_BELOWNORMAL   = 8,          ///< Priority: below normal
  FC_TASK_PRIORITY_BELOWNORMAL1  = 8+1,        ///< Priority: below normal + 1
  FC_TASK_PRIORITY_BELOWNORMAL2  = 8+2,        ///< Priority: below normal + 2
  FC_TASK_PRIORITY_BELOWNORMAL3  = 8+3,        ///< Priority: below normal + 3
  FC_TASK_PRIORITY_NORMAL        = 12,         ///< Priority: normal
  FC_TASK_PRIORITY_NORMAL1       = 12+1,       ///< Priority: normal + 1
  FC_TASK_PRIORITY_NORMAL2       = 12+2,       ///< Priority: normal + 2
  FC_TASK_PRIORITY_NORMAL3       = 12+3,       ///< Priority: normal + 3
  FC_TASK_PRIORITY_ABOVENORMAL   = 16,         ///< Priority: above normal
  FC_TASK_PRIORITY_ABOVENORMAL1  = 16+1,       ///< Priority: above normal + 1
  FC_TASK_PRIORITY_ABOVENORMAL2  = 16+2,       ///< Priority: above normal + 2
  FC_TASK_PRIORITY_ABOVENORMAL3  = 16+3,       ///< Priority: above normal + 3
  FC_TASK_PRIORITY_HIGH          = 20,         ///< Priority: high
  FC_TASK_PRIORITY_HIGH1         = 20+1,       ///< Priority: high + 1
  FC_TASK_PRIORITY_HIGH2         = 20+2,       ///< Priority: high + 2
  FC_TASK_PRIORITY_HIGH3         = 20+3,       ///< Priority: high + 3
  FC_TASK_PRIORITY_REALTIME      = 24,         ///< Priority: realtime
  FC_TASK_PRIORITY_REALTIME1     = 24+1,       ///< Priority: realtime + 1
  FC_TASK_PRIORITY_REALTIME2     = 24+2,       ///< Priority: realtime + 2
  FC_TASK_PRIORITY_REALTIME3     = 24+3,       ///< Priority: realtime + 3
  FC_TASK_PRIORITY_ISR           = 31,         ///< Reserved for ISR deferred thread.
  FC_TASK_PRIORITY_ERROR         = -1,         ///< System cannot determine priority or illegal priority.
  FC_TASK_PRIORITY_RESERVED      = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} fd_task_priority_t;


#define RS232 usart1
#define RS485 usart2


#define DEBUG
#define MODEBUS


#ifdef DEBUG
	#define LOGD_OUT
#endif

#ifdef MODEBUS
	#define RS485_MODEBUS
	#define RS232_MODEBUS
#endif


#define HOLD_REG_SIZE 		10
#define INPUT_REG_SIZE 		10
#define COIL_REG_SIZE 		10
#define COIL_IN_REG_SIZE 	10

extern unsigned short holdReg[HOLD_REG_SIZE];
extern unsigned short inputReg[INPUT_REG_SIZE];
extern unsigned short coilReg[COIL_REG_SIZE];
extern unsigned short coilInReg[COIL_IN_REG_SIZE];


#endif
