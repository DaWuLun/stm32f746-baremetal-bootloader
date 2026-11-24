#ifndef _FAULTS_H_
#define _FAULTS_H_

void reset_handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void Default_Handler(void);

#endif
