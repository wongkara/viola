/*
 * solenoid.h
 *
 *  Created on: Mar 31, 2026
 *      Author: srinimma
 */

#ifndef SRC_SOLENOID_H_
#define SRC_SOLENOID_H_


// GPIO PE15
void actuatePair1(void);

// GPIO PE14
void actuatePair2(void);

// GPIO PE12
void actuatePair3(void);

// GPIO PE10
void actuatePair4(void);

// initialize all solenoids to rest state
void Solenoid_init(void);


#endif /* SRC_SOLENOID_H_ */
